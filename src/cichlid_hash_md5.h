/*
 * Copyright © 2015 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_md5.h
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
#ifndef CICHLID_HASH_MD5_H
#define CICHLID_HASH_MD5_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct _CichlidHashMd5 CichlidHashMd5;
struct _CichlidHashMd5
{
  uint32_t h[4];
  uint8_t  data_left[63];
  uint8_t  data_left_size;
  uint64_t total_size;
  bool     hash_computed;
};

void cichlid_hash_md5_init(CichlidHashMd5 *self);
void cichlid_hash_md5_update(CichlidHashMd5 *self, const char *data, size_t data_size);
char *cichlid_hash_md5_get_hash(CichlidHashMd5 *self);

#endif /* CICHLID_HASH_MD5_H */
