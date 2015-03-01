/*
 * Copyright © 2015 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha2_64.c
 *
 * Implementation of the calculation of 64-bit SHA-2 hashes (SHA384 and SHA512)
 * as defined in FIPS 180-2, with the limitation that the message can at most be
 * 2^64-1 bit (instead of the standard's 2^128-1 bit).
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
#include "cichlid_hash_sha2_64.h"
#include "cichlid_hash_common.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*!
 * \param[in,out] hash Current hash state, is updated by the function.
 * \param         data New date to process.
 * \param         bytes_read Number of bytes in data.
 */
static void            calculate(uint64_t hash[8], const char *data, size_t bytes_read);
/*!
 * \param self State struct.
 * \param hash Buffer where the finalized hash is be stored.
 */
static void            finalize(const CichlidHashSha2_64 *self, uint64_t hash[8]);
static inline uint64_t Ch(uint64_t x, uint64_t y, uint64_t z);
static inline uint64_t Maj(uint64_t x, uint64_t y, uint64_t z);
static inline uint64_t Sigma0(uint64_t x);
static inline uint64_t Sigma1(uint64_t x);
static inline uint64_t sigma0(uint64_t x);
static inline uint64_t sigma1(uint64_t x);

static const uint64_t k[80] = {
    0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
    0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
    0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
    0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
    0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
    0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
    0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
    0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
    0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
    0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
    0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
    0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
    0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
    0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
    0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
    0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
    0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
    0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
    0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
    0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};

void cichlid_hash_sha2_64_init(CichlidHashSha2_64 *self, const uint64_t *h0, uint64_t hash_length)
{
    self->total_size = 0;
    self->data_left_size = 0;
    self->hash_size = hash_length;
    for (int i = 0; i < CICHLID_HASH_SHA2_64_N_WORDS; ++i) {
        self->h[i] = h0[i];
    }
}

char *cichlid_hash_sha2_64_get_hash(CichlidHashSha2_64 *self)
{
    char *hash_string;
    uint64_t hash[8];
    finalize(self, hash);
    hash_string = malloc(sizeof(*hash_string) * (self->hash_size + 1));
    for (int i = 0; i < self->hash_size / 16; ++i) {
        sprintf(hash_string + 16 * i, "%.16llx", hash[i]);
    }
    return hash_string;
}

void cichlid_hash_sha2_64_update(CichlidHashSha2_64 *self, const char *data, size_t data_size)
{
    if (data_size == 0) {
        return;
    }

    self->total_size += data_size;

    if (self->data_left_size == 0) {
        /* If there is no data left since the previous update */
        self->data_left_size = data_size % 128;
        memcpy(self->data_left, data + data_size - self->data_left_size, self->data_left_size);
        calculate(self->h, data, data_size - self->data_left_size);
    } else if (data_size + self->data_left_size < 128) {
        /* If there is data left since the previous update but the total data size < 64 bytes */
        memcpy(self->data_left + self->data_left_size, data, data_size);
        self->data_left_size += data_size;
    } else {
    /* If there is data left since the previous update*/
        uint8_t new_data_left_size = (data_size + self->data_left_size) % 128;

        char *buf = malloc(sizeof(char)*(data_size + self->data_left_size - new_data_left_size));
        memcpy(buf, self->data_left, self->data_left_size);
        memcpy(buf + self->data_left_size, data, data_size - new_data_left_size);
        memcpy(self->data_left, data + data_size - new_data_left_size, new_data_left_size);

        calculate(self->h, buf, data_size + self->data_left_size - new_data_left_size);
        self->data_left_size = new_data_left_size;

        free(buf);
    }
}

static void calculate(uint64_t hash[8], const char *data, size_t bytes_read)
{
    uint64_t a, b, c, d, e, f, g, h, t1, t2;
    uint64_t w[80];

    /* Process data in 1024-bit chunks */
    for (int j = 0; j < bytes_read/128; j++) {
        cichlid_change_endianness_64(w, (uint64_t*)(data+j*128), 16);

        /* Extend w to contain 80 uint64_t */
        for (int i = 16; i < 80; i++) {
            w[i] = w[i-16] + sigma0(w[i-15]) + w[i-7] + sigma1(w[i-2]);
        }

        /* Initialize with the current values */
        a = hash[0];
        b = hash[1];
        c = hash[2];
        d = hash[3];
        e = hash[4];
        f = hash[5];
        g = hash[6];
        h = hash[7];

        /* Calculate */
        for (int i = 0; i < 80; i++) {
            t1 = h + Sigma1(e) + Ch(e, f, g) + k[i] + w[i];
            t2 = Sigma0(a) + Maj(a, b, c);

            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        /* Add the current chunk's result to the total */
        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
        hash[4] += e;
        hash[5] += f;
        hash[6] += g;
        hash[7] += h;
    }
}

static void finalize(const CichlidHashSha2_64 *self, uint64_t hash[8])
{
    char     buf[sizeof(char) * 64 * 4] = { 0,};
    size_t   size_offset;
    uint64_t total_size_bits;

    /* Populate hash with the current state */
    memcpy(hash, self->h, sizeof(*hash) * 8);

    if (self->data_left_size < 112) {
        size_offset = 112;
    } else {
        size_offset = 128 + 112;
    }

    /* Copy the data that is left, the ending 1 and the size to the buffer */
    memcpy(buf, self->data_left, self->data_left_size);
    buf[self->data_left_size] = -0x80; /* Trailing '1'-bit */
    /* FIXME: Will overflow for sizes larger than 2^64-1 bits but the algorithm
     *        supports sizes up to 2^128-1 bits */
    total_size_bits = self->total_size * 8; /* Convert size to bits */
    cichlid_change_endianness_64((uint64_t *)&buf[size_offset + 8], &total_size_bits, 1);
    calculate(hash, buf, size_offset + 16);
}

static inline uint64_t Ch(uint64_t x, uint64_t y, uint64_t z)
{
    return (x & y) ^ ((~x) & z);
}

static inline uint64_t Maj(uint64_t x, uint64_t y, uint64_t z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

static inline uint64_t Sigma0(uint64_t x)
{
    return cichlid_rotate_right_64(x, 28) ^ cichlid_rotate_right_64(x, 34) ^ cichlid_rotate_right_64(x, 39);
}

static inline uint64_t Sigma1(uint64_t x)
{
    return cichlid_rotate_right_64(x, 14) ^ cichlid_rotate_right_64(x, 18) ^ cichlid_rotate_right_64(x, 41);
}

static inline uint64_t sigma0(uint64_t x)
{
    return cichlid_rotate_right_64(x, 1) ^ cichlid_rotate_right_64(x, 8) ^ (x >> 7);
}

static inline uint64_t sigma1(uint64_t x)
{
    return cichlid_rotate_right_64(x, 19) ^ cichlid_rotate_right_64(x, 61) ^ (x >> 6);
}
