/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_common.h
 *
 * Common functions for CichlidHash*
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

#ifndef CICHLID_HASH_COMMON_H
#define CICHLID_HASH_COMMON_H

#define CHANGE_ENDIANNESS(dest, src, n) do {			\
	const uint32_t *source = src;						\
	uint32_t *destination = dest;						\
	for (int i = 0; i < n; i++)							\
		destination[i] = ((source[i] & 0xFF) << 24) |	\
			((source[i] & 0xFF00) << 8) |				\
			((source[i] & 0xFF0000) >> 8) |				\
			((source[i] & 0xFF000000) >> 24);			\
	} while (0)

#define CHANGE_ENDIANNESS_64(dest, src, n) do {			\
	const uint64_t *source = src;						\
	uint64_t *destination = dest;						\
	for (int i = 0; i < n; i++)							\
		destination[i] = ((source[i] & 0xFF) << 56) |	\
			((source[i] & 0xFF00) << 40) |				\
			((source[i] & 0xFF0000) << 24) |			\
			((source[i] & 0xFF000000) << 8) |			\
			((source[i] & 0xFF00000000) >> 8) |			\
			((source[i] & 0xFF0000000000) >> 24) |		\
			((source[i] & 0xFF000000000000) >> 40) |	\
			((source[i] & 0xFF00000000000000) >> 56);	\
	} while (0)

#define ROTATE_LEFT(x, y) (((x) << (y)) | ((x) >> (32-y)))

#define ROTATE_LEFT_64(x, y) (((x) << (y)) | ((x) >> (64-y)))

#define ROTATE_RIGHT(x, y) (((x) >> (y)) | ((x) << (32-y)))

#define ROTATE_RIGHT_64(x, y) (((x) >> (y)) | ((x) << (64-y)))

#endif /* CICHLID_HASH_COMMON_H */
