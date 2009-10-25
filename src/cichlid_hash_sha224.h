/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_md5.h
 *
 * Class for computing checksums according to the
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

#ifndef CICHLID_HASH_SHA224_H
#define CICHLID_HASH_SHA224_H

#include <glib-object.h>
#include <stdint.h>

#define CICHLID_TYPE_HASH_SHA224       		(cichlid_hash_sha224_get_type())
#define CICHLID_HASH_SHA224(obj)       		(G_TYPE_CHECK_INSTANCE_CAST((obj), CICHLID_TYPE_HASH_SHA224, CichlidHashSha224))
#define CICHLID_IS_HASH_SHA224(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), CICHLID_TYPE_HASH_SHA224))
#define CICHLID_HASH_SHA224_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), CICHLID_TYPE_HASH_SHA224, CichlidHashSha224Class))
#define CICHLID_IS_HASH_SHA224_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), CICHLID_TYPE_HASH_SHA224))
#define CICHLID_HASH_SHA224_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), CICHLID_TYPE_HASH_SHA224, CichlidHashSha224Class))

typedef struct _CichlidHashSha224        CichlidHashSha224;
typedef struct _CichlidHashSha224Class   CichlidHashSha224Class;

struct _CichlidHashSha224
{
  GObject parent_instance;

  /* Private */
  uint32_t  p[8];
  uint8_t   data_left[63];
  uint8_t   data_left_size;
  uint64_t  total_size;
  gboolean  hash_computed;
};

struct _CichlidHashSha224Class
{
  GObjectClass parent_class;
};

GType cichlid_hash_sha224_get_type(void);

CichlidHash* cichlid_hash_sha224_new();

#endif /* CICHLID_HASH_SHA224_H */
