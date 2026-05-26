/********************************************************************************* 
* tasks.c * FreeRTOS задачи для радиоудлинителя UART
 * ******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "uart.h"
#include "cc1101.h"
#include "protocol.h"
#include "bat_brd.h"

#define RF_TX_QUEUE_LEN  8
#define RF_RX_QUEUE_LEN  8
#define MAX_CNT 500
///static uint16_t tcnt=0;
///static uint8_t cur_led=0;
static QueueHandle_t q_rf_tx, q_rf_rx;

void task_uart_rx(void *pvParameters){
  uint8_t buf[PROTO_MAX_PACKET];    
  uint8_t idx = 0;    
  uint8_t id = 0;    
  for (;;) { 
    if (UART_PC_Available()) { 
      uint8_t b = UART_PC_ReadByte();           
      if (idx == 0) { 
        if (b > 0 && b <= PROTO_MAX_PACKET - 4) { 
          buf[idx++] = b; 
        }            
      } else {               
        buf[idx++] = b;   
        if (idx >= buf[0] + 3) { 
          uint8_t data[PROTO_MAX_PACKET];
          uint8_t out_len;  
          if (Protocol_ParsePacket(buf, idx, &id, data, &out_len)) {  
            xQueueSend(q_rf_tx, data, 0); 
          } 
          idx = 0;  
        }   
      }   
    }
/*    
    tcnt++;
    if(tcnt>=MAX_CNT){
      tcnt=0;
      cur_led++;
      put_led(cur_led);
    }
 */   
    vTaskDelay(1);
  }
}
void task_rf_send(void *pvParameters){
  uint8_t data[PROTO_MAX_PACKET]; 
  uint8_t pkt[PROTO_MAX_PACKET];  
  uint8_t id = 0;    
  for (;;) {
    if (xQueueReceive(q_rf_tx, data, portMAX_DELAY)) { 
      uint8_t len = Protocol_BuildPacket(id++, data, sizeof(data), pkt);
      CC1101_SendCmd(CC1101_CMD_SIDLE); 
      CC1101_WriteBurst(CC1101_REG_FIFO, pkt, len); 
      CC1101_SendCmd(CC1101_CMD_STX);  
      vTaskDelay(10);  
      CC1101_SendCmd(CC1101_CMD_SRX); 
    }
  }
}
void task_rf_receive(void *pvParameters){
  uint8_t pkt[PROTO_MAX_PACKET];
  uint8_t data[PROTO_MAX_PACKET]; 
  uint8_t id; 
  uint8_t len;  
  for (;;) { 
    if (CC1101_ReadReg(CC1101_REG_RXBYTES) > 0) {
      CC1101_ReadBurst(CC1101_REG_FIFO, pkt, CC1101_ReadReg(CC1101_REG_RXBYTES)); 
      if (Protocol_ParsePacket(pkt, pkt[0] + 3, &id, data, &len)) { 
        UART_PC_WriteBuffer(data, len);           
      }       
    }    
    vTaskDelay(5);  
  }
}
void idle_task(void *pvParameters){ 
  for (;;) {  
    /* LED heartbeat */      
    LED_UART_RX_On();  
    vTaskDelay(500);    
    LED_UART_RX_Off();   
    vTaskDelay(500);  
  }
}
void tasks_init(void){  
  q_rf_tx = xQueueCreate(RF_TX_QUEUE_LEN, PROTO_MAX_PACKET);  
  q_rf_rx = xQueueCreate(RF_RX_QUEUE_LEN, PROTO_MAX_PACKET);  
  xTaskCreate(task_uart_rx, "UART_RX", 256, NULL, 2, NULL);  
  xTaskCreate(task_rf_send, "RF_SEND", 256, NULL, 3, NULL);   
     }