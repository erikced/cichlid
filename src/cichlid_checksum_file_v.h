/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_checksum_file_verifier.h
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

#ifndef CICHLID_CHECKSUM_FILE_VERIFIER_H
#define CICHLID_CHECKSUM_FILE_VERIFIER_H

#include "cichlid_checksum_file.h"

#define CICHLID_TYPE_CHECKSUM_FILE_VERIFIER       		(cichlid_checksum_file_verifier_get_type ())
#define CICHLID_CHECKSUM_FILE_VERIFIER(obj)       		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CICHLID_TYPE_CHECKSUM_FILE_VERIFIER, CichlidChecksumFileVerifier))
#define CICHLID_IS_CHECKSUM_FILE_VERIFIER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CICHLID_TYPE_CHECKSUM_FILE_VERIFIER))
#define CICHLID_CHECKSUM_FILE_VERIFIER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CICHLID_TYPE_CHECKSUM_FILE_VERIFIER, CichlidChecksumFileVerifierClass))
#define CICHLID_IS_CHECKSUM_FILE_VERIFIER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CICHLID_TYPE_CHECKSUM_FILE_VERIFIER))
#define CICHLID_CHECKSUM_FILE_VERIFIER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CICHLID_TYPE_CHECKSUM_FILE_VERIFIER, CichlidChecksumFileVerifierClass))

typedef struct _CichlidChecksumFileVerifier        CichlidChecksumFileVerifier;
typedef struct _CichlidChecksumFileVerifierClass   CichlidChecksumFileVerifierClass;

struct _CichlidChecksumFileVerifier
{
	GObject parent_instance;

	/* Private */
	CichlidChecksumFile *checksum_file;

	gboolean             active;

	uint64_t             total_file_size;
	uint32_t			 total_file_num;

    uint32_t             current_file_num;	/* Index of current file */
    char                *current_file;

	volatile int         abort;
	volatile int		 verified_file_size;

	GMutex              *status_update_lock;
	gboolean             status_update_queued;
	GList               *status_updates;
};

struct _CichlidChecksumFileVerifierClass
{
  GObjectClass parent_class;

  void (* verification_complete) (CichlidChecksumFileVerifier *ver);
};

#endif /* CICHLID_CHECKSUM_FILE_VERIFIER_H */

CichlidChecksumFileVerifier *cichlid_checksum_file_verifier_new(CichlidChecksumFile *checksum_file);
gboolean                     cichlid_checksum_file_verifier_start(CichlidChecksumFileVerifier *self, GError **error);
void                         cichlid_checksum_file_verifier_cancel(CichlidChecksumFileVerifier *self);
