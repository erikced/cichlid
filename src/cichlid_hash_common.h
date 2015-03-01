/*
 * Copyright © 2015 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_common.h
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

static inline void cichlid_change_endianness_32(uint32_t *dst, uint32_t *src, int n)
{
    for (int i = 0; i < n; ++i) {
        dst[i] = (((src[i] & 0x000000FF) << 24) |
                  ((src[i] & 0x0000FF00) << 8) |
                  ((src[i] & 0x00FF0000) >> 8) |
                  ((src[i] & 0xFF000000) >> 24));
    }
}

static inline void cichlid_change_endianness_64(uint64_t *dst, uint64_t *src, int n)
{
    for (int i = 0; i < n; ++i) {
        dst[i] = (((src[i] & 0x00000000000000FF) << 56) |
                  ((src[i] & 0x000000000000FF00) << 40) |
                  ((src[i] & 0x0000000000FF0000) << 24) |
                  ((src[i] & 0x00000000FF000000) << 8) |
                  ((src[i] & 0x000000FF00000000) >> 8) |
                  ((src[i] & 0x0000FF0000000000) >> 24) |
                  ((src[i] & 0x00FF000000000000) >> 40) |
                  ((src[i] & 0xFF00000000000000) >> 56));
    }
}

static inline uint32_t cichlid_rotate_left_32(uint32_t x, int n)
{
    return (x << n) | (x >> (32 - n));
}

static inline uint64_t cichlid_rotate_left_64(uint64_t x, int n)
{
   return  (x << n) | (x >> (64 - n));
}

static inline uint32_t cichlid_rotate_right_32(uint32_t x, int n)
{
    return (x >> n) | (x << (32 - n));
}

static inline uint64_t cichlid_rotate_right_64(uint64_t x, int n)
{
    return (x >> n) | (x << (64 - n));
}

#endif /* CICHLID_HASH_COMMON_H */
