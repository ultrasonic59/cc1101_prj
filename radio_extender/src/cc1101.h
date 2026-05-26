/********************************************************************************* 
* cc1101.h * CC1101 драйвер для AT32F415RCT7 (SPI2 + ручные команды) 
* Частота: 433 MHz, 10 kbps, 2FSK 
* ******************************************************************************/
#ifndef CC1101_H
#define CC1101_H
#include <stdint.h>
/* CC1101 команды */
#define CC1101_CMD_SRES      0x30
#define CC1101_CMD_SRX       0x34
#define CC1101_CMD_STX       0x35
#define CC1101_CMD_SIDLE     0x36
#define CC1101_CMD_SCAL      0x33
#define CC1101_CMD_SFRX      0x3A
#define CC1101_CMD_SFTX      0x3B
#define CC1101_CMD_SNOP      0x3D
/* CC1101 регистры */
#define CC1101_REG_IOCFG2    0x00
#define CC1101_REG_IOCFG1    0x01
#define CC1101_REG_IOCFG0    0x02
#define CC1101_REG_FIFOTHR   0x03
#define CC1101_REG_SYNC1     0x04
#define CC1101_REG_SYNC0     0x05
#define CC1101_REG_PKTLEN    0x06
#define CC1101_REG_PKTCTRL1  0x07
#define CC1101_REG_PKTCTRL0  0x08
#define CC1101_REG_ADDR      0x09
#define CC1101_REG_CHANNR    0x0A
#define CC1101_REG_FSCTRL1   0x0B
#define CC1101_REG_FSCTRL0   0x0C
#define CC1101_REG_FREQ2     0x0D
#define CC1101_REG_FREQ1     0x0E
#define CC1101_REG_FREQ0     0x0F
#define CC1101_REG_MDMCFG4   0x10
#define CC1101_REG_MDMCFG3   0x11
#define CC1101_REG_MDMCFG2   0x12
#define CC1101_REG_MDMCFG1   0x13
#define CC1101_REG_MDMCFG0   0x14
#define CC1101_REG_DEVIATN   0x15
#define CC1101_REG_MCSM2     0x16
#define CC1101_REG_MCSM1     0x17
#define CC1101_REG_MCSM0     0x18
#define CC1101_REG_FOCCFG    0x19
#define CC1101_REG_BSCFG     0x1A
#define CC1101_REG_AGCCTRL2  0x1B
#define CC1101_REG_AGCCTRL1  0x1C
#define CC1101_REG_AGCCTRL0  0x1D
#define CC1101_REG_WOREVT1   0x1E
#define CC1101_REG_WOREVT0   0x1F
#define CC1101_REG_WORCTRL   0x20
#define CC1101_REG_FREND1    0x21
#define CC1101_REG_FREND0    0x22
#define CC1101_REG_FSCAL3    0x23
#define CC1101_REG_FSCAL2    0x24
#define CC1101_REG_FSCAL1    0x25
#define CC1101_REG_FSCAL0    0x26
#define CC1101_REG_RCCTRL1   0x27
#define CC1101_REG_RCCTRL0   0x28
#define CC1101_REG_FSTEST    0x29
#define CC1101_REG_PTEST     0x2A
#define CC1101_REG_AGCTEST   0x2B
#define CC1101_REG_TEST2     0x2C
#define CC1101_REG_TEST1     0x2D
#define CC1101_REG_TEST0     0x2E
#define CC1101_REG_PARTNUM   0x30
#define CC1101_REG_VERSION   0x31
#define CC1101_REG_FREQEST   0x32
#define CC1101_REG_LQI       0x33
#define CC1101_REG_RSSI      0x34
#define CC1101_REG_MARCSTATE 0x35
#define CC1101_MARCSTATE_TX  0x13
#define CC1101_MARCSTATE_IDLE 0x01
#define CC1101_MARCSTATE_RX  0x0D
#define CC1101_MARCSTATE_RX_END 0x0E
#define CC1101_MARCSTATE_TXRX_SWITCH 0x10
#define CC1101_MARCSTATE_RXFIFO_OVERFLOW 0x11
#define CC1101_RXBYTES_OVERFLOW 0x80U
#define CC1101_LINK_PKTLEN   5U
#define CC1101_MAX_PKTLEN    64U
#define CC1101_PKTCTRL0_VAR  0x01U
#define CC1101_REG_WORTIME1  0x36
#define CC1101_REG_WORTIME0  0x37
#define CC1101_REG_PKTSTATUS 0x38
#define CC1101_REG_VCO_VC_DAC 0x39
#define CC1101_REG_TXBYTES   0x3A
#define CC1101_REG_RXBYTES   0x3B
#define CC1101_REG_RCCTRL1_STATUS 0x3C
#define CC1101_REG_RCCTRL0_STATUS 0x3D
#define CC1101_REG_PATABLE   0x3E
#define CC1101_REG_FIFO      0x3F
/* Функции */
extern void CC1101_Init(void);
extern void CC1101_WriteReg(uint8_t addr, uint8_t value);
uint8_t CC1101_ReadReg(uint8_t addr);
void CC1101_WriteBurst(uint8_t addr, const uint8_t *data, uint8_t len);
void CC1101_ReadBurst(uint8_t addr, uint8_t *data, uint8_t len);
void CC1101_SendCmd(uint8_t cmd);
void CC1101_SetFrequency(uint32_t freq_hz);
void CC1101_SetPower(uint8_t power);
/* Добавить к списку функций */
uint8_t CC1101_CheckConnection(void);
uint8_t CC1101_VerifySpi(void);
uint8_t CC1101_ProbeSpi(void);
uint8_t CC1101_ReapplyRadio(void);
void CC1101_ApplyVariablePacketMode(void);
void CC1101_ApplyLinkPacketMode(void);
void CC1101_FlushRxFifo(void);
uint8_t CC1101_EnterRx(void);
void CC1101_StrobeRx(void);
void CC1101_ClearRxFifo(void);
void CC1101_ListenAfterTx(void);

#endif /* CC1101_H */