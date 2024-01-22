#include "bitwriter.h"

#include "io.h"

#include <stdio.h>
#include <stdlib.h>
struct BitWriter {
    Buffer *underlying_stream;
    uint8_t byte;
    uint8_t bit_position;
};

BitWriter *bit_write_open(const char *filename) {
    BitWriter *buf = malloc(sizeof(BitWriter));
    if (buf == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for BitWriter.\n");
        exit(EXIT_FAILURE);
    }
    Buffer *underlying_stream = write_open(filename);
    if (underlying_stream == NULL) {
        fprintf(stderr, "Error: Failed to open file '%s' for writing.\n", filename);
        exit(EXIT_FAILURE);
    }
    buf->underlying_stream = underlying_stream;
    buf->byte = 0;
    buf->bit_position = 0;

    return buf;
}
void bit_write_close(BitWriter **pbuf) {
    BitWriter *buf = *pbuf;
    if (buf->bit_position > 0) {
        write_uint8(buf->underlying_stream, buf->byte);
    }
    write_close(&buf->underlying_stream);
    free(buf);
    *pbuf = NULL;
}

void bit_write_bit(BitWriter *buf, uint8_t x) {
    if (buf->bit_position > 7) {
        write_uint8(buf->underlying_stream, buf->byte);
        buf->byte = 0x00;
        buf->bit_position = 0;
    }

    if (x & 1) {
        buf->byte |= (x & 1) << buf->bit_position;
    }

    ++buf->bit_position;
}
void bit_write_uint8(BitWriter *buf, uint8_t x) {
    for (int i = 0; i < 8; ++i) {
        bit_write_bit(buf, x & 1);
        x >>= 1;
    }
}
void bit_write_uint16(BitWriter *buf, uint16_t x) {
    for (int i = 0; i < 16; ++i) {
        bit_write_bit(buf, x & 1);
        x >>= 1;
    }
}
void bit_write_uint32(BitWriter *buf, uint32_t x) {
    for (int i = 0; i < 32; ++i) {
        bit_write_bit(buf, x & 1);
        x >>= 1;
    }
}
