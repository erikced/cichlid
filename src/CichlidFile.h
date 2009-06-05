/*
 * Copyright © 2008 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - CichlidFile.h
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

#ifndef CICHLID_FILE_R_H
#define CICHLID_FILE_VERIFER_H

#include <glib-object.h>

#define CICHLID_TYPE_FILE       		(cichlid_file_get_type ())
#define CICHLID_FILE(obj)       		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CICHLID_TYPE_FILE, CichlidFile))
#define CICHLID_IS_FILE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CICHLID_TYPE_FILE))
#define CICHLID_FILE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CICHLID_TYPE_FILE, CichlidFileClass))
#define CICHLID_IS_FILE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CICHLID_TYPE_FILE))
#define CICHLID_FILE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CICHLID_TYPE_FILE, CichlidFileClass))

typedef struct _CichlidFile        CichlidFile;
typedef struct _CichlidFileClass   CichlidFileClass;

/* File types */
enum
{
	HASH_UNKNOWN,
	HASH_CRC32,
	HASH_MD5,
	FILE_SHA1
};

/* File statuses */
enum
{
	GOOD,
	BAD,
	NOT_VERIFIED,
	NOT_FOUND
};

struct _CichlidFile
{
  GObject parent_instance;
  GFile *file;
  gint filetype;
  gint status;
  gconstpointer checksum;

};

struct _CichlidFileClass
{
  GObjectClass parent_class;
};

GType cichlid_file_get_type (void);

CichlidFile* cichlid_file_new(GFile* file, gint filetype, gconstpointer checksum);


#endif /* CICHLID_FILE_VERIFER_H */
