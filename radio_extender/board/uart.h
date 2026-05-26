/********************************************************************************* 
* uart.h * UART2 драйвер с кольцевыми буферами для AT32F415RCT7 
* 115200 8N1 
* ******************************************************************************/
#ifndef UART_H
#define UART_H

#include <stdint.h>

#define DEF_BR      115200
#define DBG_BR      115200

#define  UART_PRI 5

#define UART_RX_BUF_SIZE 128    ///256
#define UART_TX_BUF_SIZE 128    ///256

extern void UART_PC_Init(void);
extern uint8_t UART_PC_IsReady(void);
extern void USART1_IRQHandler(void);
extern uint8_t UART_PC_ReadByte(void);
extern uint8_t UART_PC_Available(void);
extern void UART_PC_WriteByte(uint8_t byte);
extern void UART_PC_WriteString(const char *str);
extern void UART_PC_WriteBuffer(const uint8_t *data, uint16_t len);
extern void UART_DBG_Init(void);

#endif /* UART_H */