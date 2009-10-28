/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha256.h
 *
 * SHA256 algorithm implemented according to FIPS 180-2
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

#ifndef CICHLID_HASH_SHA256_H
#define CICHLID_HASH_SHA256_H

#include <glib-object.h>
#include <stdint.h>
#include "cichlid_hash.h"
#include "cichlid_hash_sha2_32.h"

#define CICHLID_TYPE_HASH_SHA256       		(cichlid_hash_sha256_get_type())
#define CICHLID_HASH_SHA256(obj)       		(G_TYPE_CHECK_INSTANCE_CAST((obj), CICHLID_TYPE_HASH_SHA256, CichlidHashSha256))
#define CICHLID_IS_HASH_SHA256(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), CICHLID_TYPE_HASH_SHA256))
#define CICHLID_HASH_SHA256_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), CICHLID_TYPE_HASH_SHA256, CichlidHashSha256Class))
#define CICHLID_IS_HASH_SHA256_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), CICHLID_TYPE_HASH_SHA256))
#define CICHLID_HASH_SHA256_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), CICHLID_TYPE_HASH_SHA256, CichlidHashSha256Class))

typedef struct _CichlidHashSha256        CichlidHashSha256;
typedef struct _CichlidHashSha256Class   CichlidHashSha256Class;

struct _CichlidHashSha256
{
  CichlidHashSha2_32 parent_instance;
};

struct _CichlidHashSha256Class
{
  CichlidHashSha2_32Class parent_class;
};

GType cichlid_hash_sha256_get_type(void);

CichlidHash* cichlid_hash_sha256_new();

#endif /* CICHLID_HASH_SHA256_H */
