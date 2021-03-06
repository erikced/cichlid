/*
 * Copyright © 2015 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha224.h
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

#ifndef CICHLID_HASH_SHA224_H
#define CICHLID_HASH_SHA224_H

#include "cichlid_hash_sha2_32.h"
#include <stddef.h>
#include <stdint.h>

typedef CichlidHashSha2_32 CichlidHashSha224;

void cichlid_hash_sha224_init(CichlidHashSha224 *self);
void cichlid_hash_sha224_update(CichlidHashSha224 *self, const char *data, size_t data_size);
char *cichlid_hash_sha224_get_hash(CichlidHashSha224 *self);

#endif /* CICHLID_HASH_SHA224_H */
