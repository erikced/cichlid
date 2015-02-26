/*
 * Copyright © 2012015 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_crc32.h
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

#ifndef CICHLID_HASH_CRC32_H
#define CICHLID_HASH_CRC32_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct _CichlidHashCrc32 CichlidHashCrc32;
struct _CichlidHashCrc32
{
    bool hash_computed;
    uint32_t hash;
};

void cichlid_hash_crc32_init(CichlidHashCrc32 *self);
void cichlid_hash_crc32_update(CichlidHashCrc32 *self, const char *data, size_t data_size);
char *cichlid_hash_crc32_get_hash(CichlidHashCrc32 *self);

#endif /* CICHLID_HASH_CRC32_H */
