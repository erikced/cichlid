/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
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

#include <glib-object.h>

#define CICHLID_TYPE_HASH_CRC32       		(cichlid_hash_crc32_get_type ())
#define CICHLID_HASH_CRC32(obj)       		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CICHLID_TYPE_HASH_CRC32, CichlidHashCrc32))
#define CICHLID_IS_HASH_CRC32(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CICHLID_TYPE_HASH_CRC32))
#define CICHLID_HASH_CRC32_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CICHLID_TYPE_HASH_CRC32, CichlidHashCrc32Class))
#define CICHLID_IS_HASH_CRC32_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CICHLID_TYPE_HASH_CRC32))
#define CICHLID_HASH_CRC32_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CICHLID_TYPE_HASH_CRC32, CichlidHashCrc32Class))

typedef struct _CichlidHashCrc32        CichlidHashCrc32;
typedef struct _CichlidHashCrc32Class   CichlidHashCrc32Class;

struct _CichlidHashCrc32
{
  GObject parent_instance;

  gboolean hash_computed;
  guint32 hash;
};

struct _CichlidHashCrc32Class
{
  GObjectClass parent_class;
};

GType cichlid_hash_crc32_get_type (void);

CichlidHash* cichlid_hash_crc32_new();

#endif /* CICHLID_HASH_CRC32_H */
