/********************************************************************************* 
* uart.c * UART1 драйвер с кольцевыми буферами для AT32F415RCT7 
* 115200 8N1 * ******************************************************************************/

#include "at32f415.h"
#include "bat_brd.h"
#include "uart.h"

static uint8_t rx_buffer[UART_RX_BUF_SIZE];
static uint8_t tx_buffer[UART_TX_BUF_SIZE];
static volatile uint16_t rx_head = 0, rx_tail = 0;
static volatile uint16_t tx_head = 0, tx_tail = 0;

static volatile uint8_t uart_pc_ready;

uint8_t UART_PC_IsReady(void)
{
    return uart_pc_ready;
}

void UART_PC_Init(void){
  /* Включить тактирование UART2 */ 
///  RCC->APB1ENR |= RCC_APB1ENR_USART2EN; 
crm_periph_clock_enable(UART_PC_PERIPH_CLOCK, TRUE);
usart_init(UART_PC, DEF_BR, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_parity_selection_config(UART_PC, USART_PARITY_NONE);
  usart_transmitter_enable(UART_PC, TRUE);
  usart_receiver_enable(UART_PC, TRUE);
  usart_interrupt_enable(UART_PC, USART_RDBF_INT, TRUE);
  usart_enable(UART_PC, TRUE);
  /* Lowest preempt priority: must not preempt SysTick/ThreadX (see SHPR in tx_initialize_low_level.s). */
  nvic_irq_enable(UART_PC_IRQn, 15, 0);
  uart_pc_ready = 1;
}

void USART1_IRQHandler(void)
{
 uint8_t rbyte; 
  if(usart_flag_get(UART_PC, USART_RDBF_FLAG) != RESET)
  {
    rbyte=usart_data_receive(UART_PC);
    uint16_t head = rx_head;
    uint16_t tail = rx_tail;
    uint16_t next = (head + 1) % UART_RX_BUF_SIZE;
    if (next != tail) {
      rx_buffer[head] = rbyte;
      rx_head = next;
    }
  }
  if (usart_flag_get(UART_PC, USART_TDBE_FLAG) != RESET) {
    uint16_t head = tx_head;
    uint16_t tail = tx_tail;
    if (head != tail) {
      usart_data_transmit(UART_PC, tx_buffer[tail]);
      tx_tail = (tail + 1) % UART_TX_BUF_SIZE;
    } else {
      usart_interrupt_enable(UART_PC, USART_TDBE_INT, FALSE);
    }
  }
}

uint8_t UART_PC_ReadByte(void){
  uint16_t head = rx_head;
  uint16_t tail = rx_tail;
  if (head == tail) {
    return 0;
  }
  uint8_t byte = rx_buffer[tail];
  rx_tail = (tail + 1) % UART_RX_BUF_SIZE;
  return byte;
}
uint8_t UART_PC_Available(void){
  if (!uart_pc_ready) {
    return 0;
  }
  uint16_t head = rx_head;
  uint16_t tail = rx_tail;
  return (head != tail) ? 1u : 0u;
}
void UART_PC_WriteByte(uint8_t byte){
  if (!uart_pc_ready) {
    return;
  }
  uint16_t head = tx_head;
  uint16_t next = (head + 1) % UART_TX_BUF_SIZE;
  uint16_t tail;
  do {
    tail = tx_tail;
  } while (next == tail);
  tx_buffer[head] = byte;
  tx_head = next;
///  USART2->CR1 |= (1 << 7);
  usart_interrupt_enable(UART_PC, USART_TDBE_INT, TRUE);
}
void UART_PC_WriteString(const char *str){  
  while (*str)
    UART_PC_WriteByte(*str++);
}
void UART_PC_WriteBuffer(const uint8_t *data, uint16_t len){
 /// return;
  for (uint16_t i = 0; i < len; i++) 
    UART_PC_WriteByte(data[i]);
}

void UART_DBG_Init(void)
{
crm_periph_clock_enable(UART_DBG_PERIPH_CLOCK, TRUE);

usart_init(UART_DBG, DBG_BR, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_parity_selection_config(UART_DBG, USART_PARITY_NONE);
  usart_transmitter_enable(UART_DBG, TRUE);
  usart_receiver_enable(UART_DBG, TRUE);
  usart_enable(UART_DBG, TRUE);
  
}
int send_char_dbg (int c) 
{ 
while (!(UART_DBG->sts  & USART_TDBE_FLAG));
UART_DBG->dt  = (c & 0x1FF);
return (c);
}
uint8_t check_uart_dbg(void)
{
 if(UART_DBG->sts  & USART_RDBF_FLAG)
    return 1;
 else
   return 0;
}
uint8_t get_byte_dbg(void)
{
  uint8_t rez=0;
  while (!(UART_DBG->sts  & USART_RDBF_FLAG));
  rez=UART_DBG->dt;
  return rez;

}
