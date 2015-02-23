/*
 * Copyright © 2015 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha224.c
 *
 * SHA224 algorithm implemented according to FIPS 180-2
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
#include "cichlid_hash_sha224.h"
#include "cichlid_hash_sha2_32.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SHA224_HASH_LENGTH (56)

static uint32_t h0[8] = {
    0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939, 0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4
};

void cichlid_hash_sha224_init(CichlidHashSha224 *self)
{
    cichlid_hash_sha2_32_init(self, h0, SHA224_HASH_LENGTH);
}

void cichlid_hash_sha224_update(CichlidHashSha224 *self, const char *data, size_t data_size)
{
    cichlid_hash_sha2_32_update(self, data, data_size);
}

uint32_t *cichlid_hash_sha224_get_hash(CichlidHashSha224 *self)
{
    return cichlid_hash_sha2_32_get_hash(self);
}

char *cichlid_hash_sha224_get_hash_string(CichlidHashSha224 *self)
{
    return cichlid_hash_sha2_32_get_hash_string(self);
}

