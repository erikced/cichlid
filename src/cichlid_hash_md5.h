/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_md5.h
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

#include <glib-object.h>

#define CICHLID_TYPE_HASH_MD5       		(cichlid_hash_md5_get_type ())
#define CICHLID_HASH_MD5(obj)       		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CICHLID_TYPE_HASH_MD5, CichlidHashMd5))
#define CICHLID_IS_HASH_MD5(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CICHLID_TYPE_HASH_MD5))
#define CICHLID_HASH_MD5_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CICHLID_TYPE_HASH_MD5, CichlidHashMd5Class))
#define CICHLID_IS_HASH_MD5_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CICHLID_TYPE_HASH_MD5))
#define CICHLID_HASH_MD5_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CICHLID_TYPE_HASH_MD5, CichlidHashMd5Class))

typedef struct _CichlidHashMd5        CichlidHashMd5;
typedef struct _CichlidHashMd5Class   CichlidHashMd5Class;

struct _CichlidHashMd5
{
  GObject parent_instance;

  /* Private */
  guint32 p[4];
  gchar data_left[63];
  gchar data_left_size;
  guint64 total_size;
  gboolean hash_computed;
};

struct _CichlidHashMd5Class
{
  GObjectClass parent_class;
};

GType cichlid_hash_md5_get_type (void);

CichlidHash* cichlid_hash_md5_new();

#endif /* CICHLID_HASH_MD5_H */
