/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha2_64.h
 *
 * Implementation of the calculation of 64-bit SHA-2 hashes (SHA384 and SHA512)
 * as defined in FIPS 180-2, with the limitation that the message can at most be
 * 2^64-1 bit (instead of the standard's 2^128-1 bit).
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

#ifndef CICHLID_HASH_SHA2_64_H
#define CICHLID_HASH_SHA2_64_H

#include <glib-object.h>
#include <stdint.h>

#define CICHLID_TYPE_HASH_SHA2_64       		(cichlid_hash_sha2_64_get_type())
#define CICHLID_HASH_SHA2_64(obj)       		(G_TYPE_CHECK_INSTANCE_CAST((obj), CICHLID_TYPE_HASH_SHA2_64, CichlidHashSha2_64))
#define CICHLID_IS_HASH_SHA2_64(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), CICHLID_TYPE_HASH_SHA2_64))
#define CICHLID_HASH_SHA2_64_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), CICHLID_TYPE_HASH_SHA2_64, CichlidHashSha2_64Class))
#define CICHLID_IS_HASH_SHA2_64_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), CICHLID_TYPE_HASH_SHA2_64))
#define CICHLID_HASH_SHA2_64_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), CICHLID_TYPE_HASH_SHA2_64, CichlidHashSha2_64Class))

typedef struct _CichlidHashSha2_64        CichlidHashSha2_64;
typedef struct _CichlidHashSha2_64Class   CichlidHashSha2_64Class;

struct _CichlidHashSha2_64
{
	GObject parent_instance;

	/* Private */
	uint8_t  data_left[127];
	uint8_t  data_left_size;
	uint64_t h[8];
	uint64_t h0[8];
	gboolean hash_computed;
	uint32_t hash_size;
	uint8_t  hash_type;
	gboolean initialized; 
	uint64_t total_size;
};

struct _CichlidHashSha2_64Class
{
	GObjectClass parent_class;

	void (* get_hash_properties) (uint64_t *h0, uint32_t* hash_length);
};

GType cichlid_hash_sha2_64_get_type(void);

#endif /* CICHLID_HASH_SHA2_64_H */
