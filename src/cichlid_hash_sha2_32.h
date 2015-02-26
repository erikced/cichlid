/*
 * Copyright © 2015 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha2_32.h
 *
 * Implementation of the calculation of 32-bit SHA-2 hashes (SHA224 and SHA256)
 * as defined in FIPS 180-2
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

#ifndef CICHLID_HASH_SHA2_32_H
#define CICHLID_HASH_SHA2_32_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CICHLID_HASH_SHA2_32_HASH_SIZE (64)
#define CICHLID_HASH_SHA2_32_WORD_SIZE (8)
#define CICHLID_HASH_SHA2_32_N_WORDS (CICHLID_HASH_SHA2_32_HASH_SIZE / CICHLID_HASH_SHA2_32_WORD_SIZE)

typedef struct CichlidHashSha2_32_ CichlidHashSha2_32;
struct CichlidHashSha2_32_
{
    uint8_t  data_left[63];
    uint8_t  data_left_size;
    uint32_t h[CICHLID_HASH_SHA2_32_N_WORDS];
    uint32_t h0[CICHLID_HASH_SHA2_32_N_WORDS];
    bool hash_computed;
    uint32_t hash_size;
    uint64_t total_size;
};

void cichlid_hash_sha2_32_init(CichlidHashSha2_32 *self, const uint32_t *h0, uint32_t hash_length);
void cichlid_hash_sha2_32_update(CichlidHashSha2_32 *self, const char *data, size_t data_size);
char *cichlid_hash_sha2_32_get_hash(CichlidHashSha2_32 *self);

#endif /* CICHLID_HASH_SHA2_32_H */
