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

/* Error Definitions */
enum
{
	CICHLID_ERROR_BADCF
};

enum
{
	CICHLID_CHECKSUM_FILE_FILENAME = 0,
	CICHLID_CHECKSUM_FILE_STATUS
};

#define CICHLID_TYPE_CHECKSUM_FILE       		(cichlid_checksum_file_get_type ())
#define CICHLID_CHECKSUM_FILE(obj)       		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CICHLID_TYPE_CHECKSUM_FILE, CichlidChecksumFile))
#define CICHLID_IS_CHECKSUM_FILE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CICHLID_TYPE_CHECKSUM_FILE))
#define CICHLID_CHECKSUM_FILE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CICHLID_TYPE_CHECKSUM_FILE, CichlidChecksumFileClass))
#define CICHLID_IS_CHECKSUM_FILE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CICHLID_TYPE_CHECKSUM_FILE))
#define CICHLID_CHECKSUM_FILE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CICHLID_TYPE_CHECKSUM_FILE, CichlidChecksumFileClass))

/* Hash types */
enum
{
  HASH_UNKNOWN = 0,
  HASH_CRC32,
  HASH_MD5,
  HASH_SHA1
};

/* File statuses */
enum
{
  STATUS_BAD = 0,
  STATUS_GOOD,
  STATUS_NOT_FOUND,
  STATUS_NOT_VERIFIED
};

typedef struct _CichlidChecksumFile        CichlidChecksumFile;
typedef struct _CichlidChecksumFileClass   CichlidChecksumFileClass;

struct _CichlidChecksumFile
{
	GtkListStore parent_instance;

	/* Private */
	GFile   *file;
	GQueue	*file_queue;
	GMutex	*file_queue_lock;
	gboolean file_parsed;

	/* Checksum Options */
	char    cs_comment;    /* Character used to prepend comments in the checksum file */
	uint8_t cs_length;    /* Length of the hash (in hex chars) */
	uint8_t cs_order;     /* Order of filename / hash */
	char    cs_separator; /* Character separating the checksum from the filename if checksum_order = CHECKSUM_LAST
	                         otherwise the number of characters between the checksum and the filename */
	uint8_t cs_type;
};

struct _CichlidChecksumFileClass
{
	GtkListStoreClass parent_class;

	void (* file_loaded) (CichlidChecksumFile *checksum_file);
};

GType cichlid_checksum_file_get_type (void);

void                 cichlid_checksum_file_load(CichlidChecksumFile *self, GFile *checksum_file);

#endif /* CICHLID_CHECKSUM_FILE_H */
