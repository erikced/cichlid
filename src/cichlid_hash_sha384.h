/*
 * Copyright © 2015 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha384.h
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

#ifndef CICHLID_HASH_SHA384_H
#define CICHLID_HASH_SHA384_H

#include "cichlid_hash_sha2_64.h"
#include <stddef.h>
#include <stdint.h>

typedef CichlidHashSha2_64 CichlidHashSha384;

/*!
 * Initialize or reinitialize a SHA-384 hash calculator
 * \param self Hash calculator instance
 */
void cichlid_hash_sha384_init(CichlidHashSha384 *self);
/*!
 * Update the calculator with new data
 * \param self Hash calculator instance
 * \param data Pointer to data stream
 * \param data_size Size of available data
 */
void cichlid_hash_sha384_update(CichlidHashSha384 *self, const char *data, size_t data_size);
/*!
 * Retrieve current hash.
 * \returns A null-terminated string containing the hash
 */
char *cichlid_hash_sha384_get_hash(CichlidHashSha384 *self);

#endif /* CICHLID_HASH_SHA384_H */
