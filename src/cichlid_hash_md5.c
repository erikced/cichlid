/*
 * Copyright © 2015 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_md5.c
 *
 * Functions for computing checksums according to the
 * RSA Data Security, Inc. MD5 Message-Digest Algorithm
 * as defined in RFC 1321 from April 1992.
 *
 * cichlid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * cichlid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with cichlid.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "cichlid_hash_md5.h"

#include "cichlid_hash_common.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void cichlid_hash_md5_calc(CichlidHashMd5 *self, const char *buf, size_t bytes_read);
static void cichlid_hash_md5_finalize(CichlidHashMd5 *self);

static const int shift_lookup_table[64] = {
    0x07, 0x0C, 0x11, 0x16, 0x07, 0x0C, 0x11, 0x16,
    0x07, 0x0C, 0x11, 0x16, 0x07, 0x0C, 0x11, 0x16,
    0x05, 0x09, 0x0E, 0x14, 0x05, 0x09, 0x0E, 0x14,
    0x05, 0x09, 0x0E, 0x14, 0x05, 0x09, 0x0E, 0x14,
    0x04, 0x0B, 0x10, 0x17, 0x04, 0x0B, 0x10, 0x17,
    0x04, 0x0B, 0x10, 0x17, 0x04, 0x0B, 0x10, 0x17,
    0x06, 0x0A, 0x0F, 0x15, 0x06, 0x0A, 0x0F, 0x15,
    0x06, 0x0A, 0x0F, 0x15, 0x06, 0x0A, 0x0F, 0x15
};

static const uint32_t shift_angle_table[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

void cichlid_hash_md5_init(CichlidHashMd5 *self)
{
    /* Partial result variables */
    self->h[0] = 0x67452301;
    self->h[1] = 0xEFCDAB89;
    self->h[2] = 0x98BADCFE;
    self->h[3] = 0x10325476;

    self->hash_computed = false;
    self->total_size = 0;
    self->data_left_size = 0;
}

void cichlid_hash_md5_update(CichlidHashMd5 *self, const char *data, size_t data_size)
{
    if (self->hash_computed) {
        cichlid_hash_md5_init(self);
    }

    if (!data_size) {
        return;
    }

    self->total_size += data_size;

    if (self->data_left_size == 0) {
        /* If there is no data left since the previous update */
        self->data_left_size = data_size % 64;
        memcpy(self->data_left, data + data_size - self->data_left_size, self->data_left_size);

        cichlid_hash_md5_calc(self, data, data_size - self->data_left_size);
    } else if (data_size + self->data_left_size < 64) {
        /* If there is data left since the previous update but the total data size < 64 bytes */
        memcpy(self->data_left + self->data_left_size, data, data_size);
        self->data_left_size += data_size;
    } else {
        /* If there is data left since the previous update */
        char buf[64];
        size_t new_data_left_size = (data_size + self->data_left_size) % 64;
        memcpy(buf, self->data_left, self->data_left_size);
        memcpy(buf + self->data_left_size, data, data_size - new_data_left_size);
        memcpy(self->data_left, data + data_size - new_data_left_size, new_data_left_size);

        cichlid_hash_md5_calc(self, buf, data_size + self->data_left_size - new_data_left_size);
        self->data_left_size = (uint8_t)new_data_left_size;
    }
}

char *cichlid_hash_md5_get_hash(CichlidHashMd5 *self)
{
    uint32_t hash[4];
    char *hash_string;

    cichlid_hash_md5_finalize(self);
    cichlid_change_endianness_32(hash, self->h, 4);
    hash_string = malloc(sizeof(*hash_string) * 33);
    snprintf(hash_string, 33, "%.8x%.8x%.8x%.8x", hash[0], hash[1], hash[2], hash[3]);
    return hash_string;
}

static void cichlid_hash_md5_calc(CichlidHashMd5 *self, const char *buf, size_t bytes_read)
{
    uint32_t a, b, c, d, f, g, tmp, *w;

    /* Split buf into 512-bit chunks and process them */
    for (int j = 0; j < bytes_read/64; j++) {
        w = (uint32_t*)(buf + j * 64);
        /* Initialize with the current values */
        a = self->h[0];
        b = self->h[1];
        c = self->h[2];
        d = self->h[3];

        /* Calculate */
        for (int i = 0; i < 64; i++) {
            if (i < 16) {
                f = (b & c) | ((~b) & d);
                g = (uint32_t)i;
            } else if (i < 32) {
                f = (b & d) | (c & (~d));
                g = (5*i + 1) % 16;
            } else if (i < 48) {
                f = b ^ c ^ d;
                g = (3*i + 5) % 16;
            } else {
                f = c ^ (b | (~d));
                g = (7*i) % 16;
            }

            tmp = d;
            d = c;
            c = b;
            b = b + cichlid_rotate_left_32(a + f + shift_angle_table[i] + w[g], shift_lookup_table[i]);
            a = tmp;
        }

        /* Add this chunk's result to the total result */
        self->h[0] += a;
        self->h[1] += b;
        self->h[2] += c;
        self->h[3] += d;
    }
}

static void cichlid_hash_md5_finalize(CichlidHashMd5 *self)
{
    char     buf[128];
    size_t   size_offset;

    memset(buf, 0, sizeof(buf));
    if (!self->hash_computed) {
        if (self->data_left_size < 56) {
            size_offset = 56;
        } else {
            size_offset = 64 + 56;
        }

        /* Copy the data that is left, the ending 1 and the size to the buffer */
        memcpy(buf, self->data_left, self->data_left_size);
        buf[self->data_left_size] = -0x80; /* Signed char */
        self->total_size = self->total_size * 8; /* Convert size to bits */
        memcpy(buf + size_offset, &self->total_size, 8);

        cichlid_hash_md5_calc(self, buf, size_offset + 8);
        self->hash_computed = true;
    }
}

