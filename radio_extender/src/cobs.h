#ifndef COBS_H
#define COBS_H

#include <stddef.h>
#include <stdint.h>

/* Макс. длина COBS-кадра на проводе (encode + 0x00) для raw_len байт */
#define COBS_ENCODED_MAX(raw_max)  ((size_t)(raw_max) + ((raw_max) / 254u) + 2u)

/* Кодирование без завершающего 0x00 (его добавляет вызывающий). 0 = ошибка. */
size_t cobs_encode(const uint8_t *src, size_t src_len, uint8_t *dst);

/* Декодирование блока без завершающего 0x00. 0 = ошибка. */
size_t cobs_decode(const uint8_t *src, size_t src_len, uint8_t *dst);

typedef void (*cobs_frame_handler_t)(const uint8_t *frame, size_t len, void *ctx);

typedef struct {
    uint8_t  *buf;
    size_t    cap;
    size_t    len;
    cobs_frame_handler_t handler;
    void     *ctx;
} cobs_decoder_t;

void cobs_decoder_init(cobs_decoder_t *dec, uint8_t *buf, size_t cap,
                       cobs_frame_handler_t handler, void *ctx);

/* Подача байт (в т.ч. из CC1101 FIFO); кадры отделяются 0x00. */
void cobs_decoder_feed(cobs_decoder_t *dec, const uint8_t *data, size_t len);

void cobs_decoder_reset(cobs_decoder_t *dec);

#endif /* COBS_H */
