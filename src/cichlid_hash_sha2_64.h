/*
 * Copyright © 2015 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha2_64.h
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

#ifndef CICHLID_HASH_SHA2_64_H
#define CICHLID_HASH_SHA2_64_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CICHLID_HASH_SHA2_64_HASH_SIZE (128)
#define CICHLID_HASH_SHA2_64_WORD_SIZE (16)
#define CICHLID_HASH_SHA2_64_N_WORDS (CICHLID_HASH_SHA2_64_HASH_SIZE / CICHLID_HASH_SHA2_64_WORD_SIZE)

typedef struct CichlidHashSha2_64_ CichlidHashSha2_64;
struct CichlidHashSha2_64_
{
    uint8_t  data_left[127];
    uint8_t  data_left_size;
    uint64_t h[CICHLID_HASH_SHA2_64_N_WORDS];
    uint64_t hash_size;
    uint64_t total_size;
};

void cichlid_hash_sha2_64_init(CichlidHashSha2_64 *self, const uint64_t *h0, uint64_t hash_length);
void cichlid_hash_sha2_64_update(CichlidHashSha2_64 *self, const char *data, size_t data_size);
char *cichlid_hash_sha2_64_get_hash(CichlidHashSha2_64 *self);

#endif /* CICHLID_HASH_SHA2_64_H */

