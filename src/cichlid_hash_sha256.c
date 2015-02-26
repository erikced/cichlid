/*
 * Copyright © 2015 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha256.c
 *
 * SHA256 algorithm implemented according to FIPS 180-2
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
#include "cichlid_hash_sha256.h"
#include "cichlid_hash_sha2_32.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SHA256_HASH_LENGTH (64)

static uint32_t h0[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

void cichlid_hash_sha256_init(CichlidHashSha256 *self)
{
    cichlid_hash_sha2_32_init(self, h0, SHA256_HASH_LENGTH);
}

void cichlid_hash_sha256_update(CichlidHashSha256 *self, const char *data, size_t data_size)
{
    cichlid_hash_sha2_32_update(self, data, data_size);
}

char *cichlid_hash_sha256_get_hash(CichlidHashSha256 *self)
{
    return cichlid_hash_sha2_32_get_hash(self);
}

