/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_checksum_file.h
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

#ifndef CICHLID_CHECKSUM_FILE_H
#define CICHLID_CHECKSUM_FILE_H

#include <gtk/gtk.h>
#include <stdint.h>

#define CICHLID_TYPE_CHECKSUM_FILE       		(cichlid_checksum_file_get_type ())
#define CICHLID_CHECKSUM_FILE(obj)       		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CICHLID_TYPE_CHECKSUM_FILE, CichlidChecksumFile))
#define CICHLID_IS_CHECKSUM_FILE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CICHLID_TYPE_CHECKSUM_FILE))
#define CICHLID_CHECKSUM_FILE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CICHLID_TYPE_CHECKSUM_FILE, CichlidChecksumFileClass))
#define CICHLID_IS_CHECKSUM_FILE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CICHLID_TYPE_CHECKSUM_FILE))
#define CICHLID_CHECKSUM_FILE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CICHLID_TYPE_CHECKSUM_FILE, CichlidChecksumFileClass))

enum
{
	CICHLID_CHECKSUM_FILE_FILENAME = 0,
	CICHLID_CHECKSUM_FILE_STATUS,
	CICHLID_CHECKSUM_FILE_GFILE,
	CICHLID_CHECKSUM_FILE_CHECKSUM,
	CICHLID_CHECKSUM_FILE_FILE
};

/* Hash types */
typedef enum
{
  HASH_UNKNOWN = 0,
  HASH_CRC32,
  HASH_MD5,
  HASH_SHA1
} hash_t;

typedef struct _CichlidChecksumFile        CichlidChecksumFile;
typedef struct _CichlidChecksumFileClass   CichlidChecksumFileClass;

struct _CichlidChecksumFile
{
	GtkListStore parent_instance;

	gpointer	 priv;
};

struct _CichlidChecksumFileClass
{
	GtkListStoreClass parent_class;

	void (* file_loaded) (CichlidChecksumFile *checksum_file);
};

GType cichlid_checksum_file_get_type(void);

void cichlid_checksum_file_load_from_cmd(CichlidChecksumFile *self, const char *filename);
void cichlid_checksum_file_load(CichlidChecksumFile *self, GFile *checksum_file);
void cichlid_checksum_file_set(CichlidChecksumFile *self, GtkTreeIter *iter, int column, GValue *value);
void cichlid_checksum_file_verify(CichlidChecksumFile *self);
#endif /* CICHLID_CHECKSUM_FILE_H */
