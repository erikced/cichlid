/*
 * Copyright © 2015 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha384.c
 *
 * SHA384 algorithm implemented according to FIPS 180-2
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
#include "cichlid_hash_sha384.h"
#include "cichlid_hash_sha2_64.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SHA384_HASH_LENGTH 96

static uint64_t h0[8] = {
    0xcbbb9d5dc1059ed8, 0x629a292a367cd507, 0x9159015a3070dd17, 0x152fecd8f70e5939, 
    0x67332667ffc00b31, 0x8eb44a8768581511, 0xdb0c2e0d64f98fa7, 0x47b5481dbefa4fa4
};

void cichlid_hash_sha384_init(CichlidHashSha384 *self)
{
    cichlid_hash_sha2_64_init(self, h0, SHA384_HASH_LENGTH);
}

void cichlid_hash_sha384_update(CichlidHashSha384 *self, const char *data, size_t data_size)
{
    cichlid_hash_sha2_64_update(self, data, data_size);
}

char *cichlid_hash_sha384_get_hash(CichlidHashSha384 *self)
{
    return cichlid_hash_sha2_64_get_hash(self);
}

