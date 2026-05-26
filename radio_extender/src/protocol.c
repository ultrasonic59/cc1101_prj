#include "protocol.h"
uint16_t Protocol_CalcCRC(const uint8_t *data, uint8_t len){
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) crc = (crc << 1) ^ PROTO_CRC_POLY;
            else crc <<= 1;
        }
    }
    return crc;
}
uint8_t Protocol_BuildPacket(uint8_t id, uint8_t type, const uint8_t *data, uint8_t len, uint8_t *out){
  ///return 0;
    if (len > PROTO_MAX_PACKET - 5) 
      return 0;
    out[0] = len + 3;
    out[1] = id;
    out[2] = type;
    for (uint8_t i = 0; i < len; i++) out[3 + i] = data[i];
    uint16_t crc = Protocol_CalcCRC(out, len + 3);
    out[len + 3] = crc >> 8;
    out[len + 4] = crc & 0xFF;
    return len + 5;
}
uint8_t Protocol_ParsePacket(const uint8_t *in, uint8_t in_len, uint8_t *id, uint8_t *type, uint8_t *data, uint8_t *out_len){
  ///return 0;
    if (in_len < 5) 
      return 0;
    uint8_t len = in[0];
    if (len + 2 != in_len) 
      return 0;
    *id = in[1];
    *type = in[2];
    for (uint8_t i = 0; i < len - 2; i++) 
      data[i] = in[3 + i];
    *out_len = len - 2;
    uint16_t crc_calc = Protocol_CalcCRC(in, len);
    uint16_t crc_recv = (in[len] << 8) | in[len + 1];
    return (crc_calc == crc_recv);
}
