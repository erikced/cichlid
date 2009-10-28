/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha2_32.h
 *
 * Implementation of the calculation of 32-bit SHA-2 hashes (SHA224 and SHA256)
 * as defined in FIPS 180-2
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

#ifndef CICHLID_HASH_SHA2_32_H
#define CICHLID_HASH_SHA2_32_H

#include <glib-object.h>
#include <stdint.h>

#define CICHLID_TYPE_HASH_SHA2_32       		(cichlid_hash_sha2_32_get_type())
#define CICHLID_HASH_SHA2_32(obj)       		(G_TYPE_CHECK_INSTANCE_CAST((obj), CICHLID_TYPE_HASH_SHA2_32, CichlidHashSha2_32))
#define CICHLID_IS_HASH_SHA2_32(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), CICHLID_TYPE_HASH_SHA2_32))
#define CICHLID_HASH_SHA2_32_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), CICHLID_TYPE_HASH_SHA2_32, CichlidHashSha2_32Class))
#define CICHLID_IS_HASH_SHA2_32_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), CICHLID_TYPE_HASH_SHA2_32))
#define CICHLID_HASH_SHA2_32_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), CICHLID_TYPE_HASH_SHA2_32, CichlidHashSha2_32Class))

typedef struct _CichlidHashSha2_32        CichlidHashSha2_32;
typedef struct _CichlidHashSha2_32Class   CichlidHashSha2_32Class;

struct _CichlidHashSha2_32
{
	GObject parent_instance;

	/* Private */
	uint8_t  data_left[63];
	uint8_t  data_left_size;
	uint32_t h[8];
	uint32_t h0[8];
	gboolean hash_computed;
	uint8_t  hash_size;
	uint8_t  hash_type;
	gboolean initialized; 
	uint64_t total_size;
};

struct _CichlidHashSha2_32Class
{
	GObjectClass parent_class;

	void (* get_hash_properties) (uint32_t *h0, uint32_t* hash_length);
};

GType cichlid_hash_sha2_32_get_type(void);

#endif /* CICHLID_HASH_SHA2_32_H */
