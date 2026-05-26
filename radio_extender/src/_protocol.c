/********************************************************************************* * protocol.c * Протокол радиоудлинителя UART: [LEN][ID][DATA...][CRC-CCITT] * ******************************************************************************/
#include "protocol.h"
uint16_t Protocol_CalcCRC(const uint8_t *data, uint8_t len){ 
  uint16_t crc = 0xFFFF;    
  for (uint8_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;        
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000) 
        crc = (crc << 1) ^ PROTO_CRC_POLY;            
      else crc <<= 1;       
    }    
  }   
  return crc;
}
uint8_t Protocol_BuildPacket(uint8_t id, const uint8_t *data, uint8_t len, uint8_t *out){ 
  if (len > PROTO_MAX_PACKET - 4) 
    return 0;    
  out[0] = len + 2;           
/* LEN = ID + DATA */   
  out[1] = id;    
  for (uint8_t i = 0; i < len; i++) 
    out[2 + i] = data[i];    
  uint16_t crc = Protocol_CalcCRC(out, len + 2);   
  out[len + 2] = crc >> 8;    
  out[len + 3] = crc & 0xFF;   
  return len + 4;
}
uint8_t Protocol_ParsePacket(const uint8_t *in, uint8_t in_len, uint8_t *id, uint8_t *data, uint8_t *out_len){
  if (in_len < 4) 
    return 0;
  uint8_t len = in[0];   
  if (len + 3 != in_len) 
    return 0;    
  *id = in[1];    
  for (uint8_t i = 0; i < len - 2; i++) 
    data[i] = in[2 + i];    
  *out_len = len - 2;    
  uint16_t crc_calc = Protocol_CalcCRC(in, len);    
  uint16_t crc_recv = (in[len] << 8) | in[len + 1];    
  if (crc_calc != crc_recv) 
    return 0;    
  return 1;
}