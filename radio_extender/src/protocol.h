#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdint.h>
#define PROTO_MAX_PACKET  64
#define PROTO_CRC_POLY    0x1021

#define PROTO_TYPE_DATA   0x00
#define PROTO_TYPE_PING   0x01
#define PROTO_TYPE_PONG   0x02

uint16_t Protocol_CalcCRC(const uint8_t *data, uint8_t len);
uint8_t Protocol_BuildPacket(uint8_t id, uint8_t type, const uint8_t *data, uint8_t len, uint8_t *out);
uint8_t Protocol_ParsePacket(const uint8_t *in, uint8_t in_len, uint8_t *id, uint8_t *type, uint8_t *data, uint8_t *out_len);
#endif
