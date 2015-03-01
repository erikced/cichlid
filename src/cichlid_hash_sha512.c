/*
 * Copyright © 2015 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha512.c
 *
 * SHA512 algorithm implemented according to FIPS 180-2
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
#include "cichlid_hash_sha512.h"
#include "cichlid_hash_sha2_64.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SHA512_HASH_LENGTH 128

static uint64_t h0[8] = {
    0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
    0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179
};

void cichlid_hash_sha512_init(CichlidHashSha512 *self)
{
    cichlid_hash_sha2_64_init(self, h0, SHA512_HASH_LENGTH);
}

void cichlid_hash_sha512_update(CichlidHashSha512 *self, const char *data, size_t data_size)
{
    cichlid_hash_sha2_64_update(self, data, data_size);
}

char *cichlid_hash_sha512_get_hash(CichlidHashSha512 *self)
{
    return cichlid_hash_sha2_64_get_hash(self);
}

