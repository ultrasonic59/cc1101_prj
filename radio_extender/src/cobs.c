#include "cobs.h"
#include "protocol.h"

size_t cobs_encode(const uint8_t *src, size_t src_len, uint8_t *dst)
{
    size_t read_index = 0;
    size_t write_index = 1;
    size_t code_index = 0;
    uint8_t code = 1;

    if (src == NULL || dst == NULL) {
        return 0;
    }

    while (read_index < src_len) {
        if (src[read_index] == 0) {
            dst[code_index] = code;
            code = 1;
            code_index = write_index++;
            read_index++;
        } else {
            dst[write_index++] = src[read_index++];
            code++;
            if (code == 0xFFu) {
                dst[code_index] = code;
                code = 1;
                code_index = write_index++;
            }
        }
    }

    dst[code_index] = code;
    return write_index;
}

size_t cobs_decode(const uint8_t *src, size_t src_len, uint8_t *dst)
{
    size_t read_index = 0;
    size_t write_index = 0;

    if (src == NULL || dst == NULL || src_len == 0) {
        return 0;
    }

    while (read_index < src_len) {
        uint8_t code = src[read_index++];

        if (code == 0) {
            return 0;
        }
        for (uint8_t i = 1; i < code; i++) {
            if (read_index >= src_len) {
                return 0;
            }
            if (src[read_index] == 0) {
                return 0;
            }
            dst[write_index++] = src[read_index++];
        }
        if (code < 0xFFu && read_index < src_len) {
            dst[write_index++] = 0;
        }
    }

    return write_index;
}

void cobs_decoder_init(cobs_decoder_t *dec, uint8_t *buf, size_t cap,
                       cobs_frame_handler_t handler, void *ctx)
{
    if (dec == NULL) {
        return;
    }
    dec->buf = buf;
    dec->cap = cap;
    dec->len = 0;
    dec->handler = handler;
    dec->ctx = ctx;
}

void cobs_decoder_reset(cobs_decoder_t *dec)
{
    if (dec != NULL) {
        dec->len = 0;
    }
}

void cobs_decoder_feed(cobs_decoder_t *dec, const uint8_t *data, size_t len)
{
    size_t i;

    if (dec == NULL || data == NULL || dec->buf == NULL || dec->handler == NULL) {
        return;
    }

    for (i = 0; i < len; i++) {
        uint8_t b = data[i];

        if (b == 0) {
            if (dec->len > 0) {
                uint8_t frame[PROTO_MAX_PACKET];
                size_t raw_len;

                if (dec->len <= dec->cap) {
                    raw_len = cobs_decode(dec->buf, dec->len, frame);
                    if (raw_len > 0) {
                        dec->handler(frame, raw_len, dec->ctx);
                    }
                }
            }
            dec->len = 0;
            continue;
        }

        if (dec->len < dec->cap) {
            dec->buf[dec->len++] = b;
        } else {
            dec->len = 0;
        }
    }
}
