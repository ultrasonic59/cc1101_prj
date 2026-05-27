/*********************************************************************************
* cc1101.c * CC1101 драйвер для AT32F415RCT7 (SPI2 + ручные команды)
* Частота: 433 MHz, 10 kbps, 2FSK
* ******************************************************************************/
#include "at32f415.h"
#include "cc1101.h"
#include "bat_brd.h"
#include "printk.h"

void CC1101_WriteReg(uint8_t addr, uint8_t value);
uint8_t CC1101_ReadReg(uint8_t addr);
void CC1101_SendCmd(uint8_t cmd);

static uint8_t spi_rf_sel_msb = 0;
static uint8_t spi_rf_sel_cpha2 = 1;
static uint8_t spi_rf_sel_div = SPI_MCLK_DIV_64;

static uint8_t SPI_RF_Transfer(uint8_t data)
{
    uint16_t rez;

    while (spi_i2s_flag_get(SPI2, SPI_I2S_TDBE_FLAG) == RESET) {
    }
    spi_i2s_data_transmit(SPI2, data);
    while (spi_i2s_flag_get(SPI2, SPI_I2S_RDBF_FLAG) == RESET) {
    }
    rez = spi_i2s_data_receive(SPI2);
    return (uint8_t)rez;
}

static void CC1101_WaitReady(void)
{
    uint32_t t;

    for (t = 0; t < 5000u; t++) {
        if (gpio_input_data_bit_read(MISO_RF_GPIO, MISO_RF_PIN) == RESET) {
            return;
        }
        delay_us(10);
    }
}

typedef struct {
    uint8_t msb;
    uint8_t cpha2;
    const char *tag;
} spi_rf_mode_t;

static const spi_rf_mode_t spi_rf_modes[] = {
    { 0, 1, "LSB/CPHA1" },
    { 1, 0, "MSB/CPHA0" },
    { 1, 1, "MSB/CPHA1" },
    { 0, 0, "LSB/CPHA0" },
};

static const uint8_t spi_rf_divs[] = {
    SPI_MCLK_DIV_64,
    SPI_MCLK_DIV_128,
    SPI_MCLK_DIV_32,
    SPI_MCLK_DIV_256,
};

static const char *spi_rf_div_tags[] = { "/64", "/128", "/32", "/256" };

static int cc1101_spi_score(uint8_t *partnum, uint8_t *version, uint8_t *rb)
{
    int score = 0;

    CC1101_SendCmd(CC1101_CMD_SRES);
    CC1101_WaitReady();
    delay_us(150);

    *partnum = CC1101_ReadReg(CC1101_REG_PARTNUM);
    *version = CC1101_ReadReg(CC1101_REG_VERSION);

    CC1101_WriteReg(CC1101_REG_CHANNR, 0xA5U);
    *rb = CC1101_ReadReg(CC1101_REG_CHANNR);
    CC1101_WriteReg(CC1101_REG_CHANNR, 0x00U);

    if (*partnum == 0x00U) {
        score += 10;
    }
    if (*version != 0x00U && *version != 0xFFU) {
        score += 5;
    }
    if (*rb == 0xA5U) {
        score += 20;
    }
    return score;
}

uint8_t CC1101_ProbeSpi(void)
{
    uint8_t partnum;
    uint8_t version;
    uint8_t rb;
    int score;

    /* Быстрый путь: MSB/CPHA0/128 — проверено на плате */
    spi_rf_configure(1, 0, SPI_MCLK_DIV_128);
    score = cc1101_spi_score(&partnum, &version, &rb);
    printk("\n\r CC1101 probe MSB/CPHA0/128: PARTNUM=0x%02X VERSION=0x%02X rb=0x%02X score=%d",
           partnum, version, rb, score);

    if (score >= 20) {
        spi_rf_sel_msb = 1;
        spi_rf_sel_cpha2 = 0;
        spi_rf_sel_div = SPI_MCLK_DIV_128;
        printk("\n\r CC1101 SPI selected: MSB/CPHA0/128 (score=%d)", score);
        return 1U;
    }

    {
        int best_score = score;
        uint8_t best_i = 1;
        uint8_t best_d = 1;
        uint8_t i;
        uint8_t d;

        for (d = 0; d < (uint8_t)(sizeof(spi_rf_divs) / sizeof(spi_rf_divs[0])); d++) {
            for (i = 0; i < (uint8_t)(sizeof(spi_rf_modes) / sizeof(spi_rf_modes[0])); i++) {
                if (spi_rf_modes[i].msb == 1 && spi_rf_modes[i].cpha2 == 0 &&
                    spi_rf_divs[d] == SPI_MCLK_DIV_128) {
                    continue;
                }
                spi_rf_configure(spi_rf_modes[i].msb, spi_rf_modes[i].cpha2, spi_rf_divs[d]);
                score = cc1101_spi_score(&partnum, &version, &rb);
                printk("\n\r CC1101 probe %s%s: PARTNUM=0x%02X VERSION=0x%02X rb=0x%02X score=%d",
                       spi_rf_modes[i].tag, spi_rf_div_tags[d],
                       partnum, version, rb, score);

                if (score > best_score) {
                    best_score = score;
                    best_i = i;
                    best_d = d;
                }
            }
        }

        spi_rf_sel_msb = spi_rf_modes[best_i].msb;
        spi_rf_sel_cpha2 = spi_rf_modes[best_i].cpha2;
        spi_rf_sel_div = spi_rf_divs[best_d];
        spi_rf_configure(spi_rf_sel_msb, spi_rf_sel_cpha2, spi_rf_sel_div);

        CC1101_SendCmd(CC1101_CMD_SRES);
        CC1101_WaitReady();
        delay_us(150);

        printk("\n\r CC1101 SPI selected: %s%s (score=%d)",
               spi_rf_modes[best_i].tag, spi_rf_div_tags[best_d], best_score);

        return (best_score >= 20) ? 1U : 0U;
    }
}

void CC1101_SendCmd(uint8_t cmd);

static void cc1101_write_radio_block(void)
{
    uint8_t freqs[3] = { 0x10, 0xA7, 0x62 };

    CC1101_SendCmd(CC1101_CMD_SIDLE);
    CC1101_WaitReady();
    delay_us(100);

    CC1101_WriteReg(CC1101_REG_SYNC1, 0xD3);
    CC1101_WriteReg(CC1101_REG_SYNC0, 0x91);
    CC1101_WriteBurst(CC1101_REG_FREQ2, freqs, 3);
    CC1101_WriteReg(CC1101_REG_MDMCFG4, 0xC8);
    CC1101_WriteReg(CC1101_REG_MDMCFG3, 0x93);
    CC1101_WriteReg(CC1101_REG_MDMCFG2, 0x13);
    CC1101_WriteReg(CC1101_REG_MDMCFG1, 0x22);
    CC1101_WriteReg(CC1101_REG_MDMCFG0, 0xF8);
    CC1101_WriteReg(CC1101_REG_DEVIATN, 0x15);
    CC1101_WriteReg(CC1101_REG_FREND1, 0x56);
    CC1101_WriteReg(CC1101_REG_FREND0, 0x10);
    CC1101_WriteReg(CC1101_REG_PATABLE, 0xC0);

    CC1101_SendCmd(CC1101_CMD_SCAL);
    CC1101_WaitReady();
    delay_us(500);
}

uint8_t CC1101_ReapplyRadio(void)
{
    uint8_t f2;
    uint8_t f1;
    uint8_t f0;
    uint8_t s1;

    cc1101_write_radio_block();

    f2 = CC1101_ReadReg(CC1101_REG_FREQ2);
    f1 = CC1101_ReadReg(CC1101_REG_FREQ1);
    f0 = CC1101_ReadReg(CC1101_REG_FREQ0);
    s1 = CC1101_ReadReg(CC1101_REG_SYNC1);

    printk("\n\r CC1101 radio FREQ2/1/0=%02X %02X %02X SYNC1=%02X",
           f2, f1, f0, s1);

    CC1101_ApplyVariablePacketMode();
    CC1101_SendCmd(CC1101_CMD_SFRX);
    CC1101_StrobeRx();

    return (f2 == 0x10U && f1 == 0xA7U && f0 == 0x62U && s1 == 0xD3U) ? 1U : 0U;
}

void CC1101_ApplyVariablePacketMode(void)
{
    /* Переменная длина: 1-й байт FIFO = размер кадра протокола; PKTLEN = максимум */
    CC1101_WriteReg(CC1101_REG_PKTCTRL1, 0x00);
    CC1101_WriteReg(CC1101_REG_PKTCTRL0, CC1101_PKTCTRL0_VAR);
    CC1101_WriteReg(CC1101_REG_PKTLEN, CC1101_MAX_PKTLEN);
}

void CC1101_ApplyLinkPacketMode(void)
{
    /* PING/PONG: 5 байт в FIFO без байта длины CC1101 ([LEN][ID][TYPE][CRC][CRC]) */
    CC1101_WriteReg(CC1101_REG_PKTCTRL1, 0x00);
    CC1101_WriteReg(CC1101_REG_PKTCTRL0, 0x00);
    CC1101_WriteReg(CC1101_REG_PKTLEN, CC1101_LINK_PKTLEN);
}

void CC1101_ClearRxFifo(void)
{
    uint8_t rxb;

    rxb = CC1101_ReadReg(CC1101_REG_RXBYTES);
    if (rxb & CC1101_RXBYTES_OVERFLOW) {
        CC1101_SendCmd(CC1101_CMD_SIDLE);
        CC1101_WaitReady();
        delay_us(100);
        CC1101_SendCmd(CC1101_CMD_SFRX);
    } else if ((rxb & 0x7Fu) > 0) {
        CC1101_SendCmd(CC1101_CMD_SFRX);
    }
}

void CC1101_ListenAfterTx(void)
{
    /* Без SIDLE после TX — как в старом _tasks.c: SFRX + SRX */
    CC1101_SendCmd(CC1101_CMD_SFRX);
    CC1101_SendCmd(CC1101_CMD_SRX);
    delay_us(500);
}

void CC1101_StrobeRx(void)
{
    CC1101_SendCmd(CC1101_CMD_SRX);
    delay_us(3000);
}

uint8_t CC1101_EnterRx(void)
{
    CC1101_SendCmd(CC1101_CMD_SIDLE);
    CC1101_WaitReady();
    delay_us(200);
    CC1101_SendCmd(CC1101_CMD_SFRX);
    CC1101_ApplyVariablePacketMode();
    CC1101_StrobeRx();
    return 1U;
}

void CC1101_FlushRxFifo(void)
{
    CC1101_SendCmd(CC1101_CMD_SIDLE);
    CC1101_WaitReady();
    delay_us(100);
    CC1101_SendCmd(CC1101_CMD_SFRX);
    CC1101_ApplyVariablePacketMode();
}

void CC1101_Init(void)
{
    CC1101_SendCmd(CC1101_CMD_SRES);
    CC1101_WaitReady();
    delay_us(150);

    CC1101_WriteReg(CC1101_REG_IOCFG0, 0x06);
    CC1101_WriteReg(CC1101_REG_FIFOTHR, 0x47);
    CC1101_ApplyVariablePacketMode();
    CC1101_WriteReg(CC1101_REG_MCSM1, 0x0C);
    CC1101_WriteReg(CC1101_REG_MCSM2, 0x00);
/* RX timeout off — постоянно в RX */
    CC1101_WriteReg(CC1101_REG_FSCTRL1, 0x06);
    CC1101_WriteReg(CC1101_REG_FSCTRL0, 0x00);

    cc1101_write_radio_block();

    CC1101_WriteReg(CC1101_REG_MCSM0, 0x18);
    CC1101_WriteReg(CC1101_REG_FOCCFG, 0x16);
    CC1101_WriteReg(CC1101_REG_AGCCTRL2, 0x43);
    CC1101_WriteReg(CC1101_REG_AGCCTRL1, 0x40);
    CC1101_WriteReg(CC1101_REG_AGCCTRL0, 0x91);
    CC1101_WriteReg(CC1101_REG_FSCAL3, 0xE9);
    CC1101_WriteReg(CC1101_REG_FSCAL2, 0x2A);
    CC1101_WriteReg(CC1101_REG_FSCAL1, 0x00);
    CC1101_WriteReg(CC1101_REG_FSCAL0, 0x1F);
    CC1101_WriteReg(CC1101_REG_TEST2, 0x81);
    CC1101_WriteReg(CC1101_REG_TEST1, 0x35);
    CC1101_WriteReg(CC1101_REG_TEST0, 0x09);

    CC1101_SendCmd(CC1101_CMD_SFRX);
    CC1101_StrobeRx();
}

uint8_t CC1101_VerifySpi(void)
{
    uint8_t partnum = CC1101_ReadReg(CC1101_REG_PARTNUM);
    uint8_t version = CC1101_ReadReg(CC1101_REG_VERSION);
    uint8_t rb;

    CC1101_WriteReg(CC1101_REG_CHANNR, 0xA5U);
    rb = CC1101_ReadReg(CC1101_REG_CHANNR);
    CC1101_WriteReg(CC1101_REG_CHANNR, 0x00U);

    printk("\n\r CC1101 verify PARTNUM=0x%02X VERSION=0x%02X CHANNR rb=0x%02X",
           partnum, version, rb);

    if (rb != 0xA5U) {
        return 0;
    }
    return 1;
}

void CC1101_WriteReg(uint8_t addr, uint8_t value){
gpio_bits_reset(NCS_RF_GPIO, NCS_RF_PIN);
  SPI_RF_Transfer(addr);
  SPI_RF_Transfer(value);
gpio_bits_set(NCS_RF_GPIO, NCS_RF_PIN);
}
uint8_t CC1101_ReadReg(uint8_t addr){
  uint8_t value;
gpio_bits_reset(NCS_RF_GPIO, NCS_RF_PIN);
  SPI_RF_Transfer(addr | 0x80);
  value = SPI_RF_Transfer(0xFF);
gpio_bits_set(NCS_RF_GPIO, NCS_RF_PIN);
  return value;
}
void CC1101_WriteBurst(uint8_t addr, const uint8_t *data, uint8_t len){
gpio_bits_reset(NCS_RF_GPIO, NCS_RF_PIN);
  SPI_RF_Transfer(addr | 0x40);
  for (uint8_t i = 0; i < len; i++)
    SPI_RF_Transfer(data[i]);
gpio_bits_set(NCS_RF_GPIO, NCS_RF_PIN);

}
void CC1101_ReadBurst(uint8_t addr, uint8_t *data, uint8_t len){
gpio_bits_reset(NCS_RF_GPIO, NCS_RF_PIN);
  SPI_RF_Transfer(addr | 0xC0);
  for (uint8_t i = 0; i < len; i++)
    data[i] = SPI_RF_Transfer(0xFF);
gpio_bits_set(NCS_RF_GPIO, NCS_RF_PIN);

}
void CC1101_SendCmd(uint8_t cmd){
gpio_bits_reset(NCS_RF_GPIO, NCS_RF_PIN);
  SPI_RF_Transfer(cmd);
gpio_bits_set(NCS_RF_GPIO, NCS_RF_PIN);

}
void CC1101_SetFrequency(uint32_t freq_hz){
  (void)freq_hz;
}
void CC1101_SetPower(uint8_t power){
  CC1101_WriteReg(CC1101_REG_PATABLE, power);
}

uint8_t CC1101_CheckConnection(void)
{
    return CC1101_VerifySpi();
}
