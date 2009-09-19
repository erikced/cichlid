/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_checksum_file_verifier.c
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

#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <stdint.h>
#include <unistd.h>

#include "cichlid.h"
#include "cichlid_file.h"
#include "cichlid_hash.h"
#include "cichlid_hash_crc32.h"
#include "cichlid_hash_md5.h"
#include "cichlid_checksum_file_v.h"

typedef struct
{
	GtkTreeIter iter;
	int status;
} StatusUpdate;

#define BUFFER_SIZE 1024*512
#define STATUS_UPDATE_INTERVAL 125 /* Milliseconds */
/* Macros for creating and freeing status updates */
#define cichlid_status_update_new() g_slice_alloc(sizeof(StatusUpdate))
#define cichlid_status_update_free(p_status_update) g_slice_free1(sizeof(StatusUpdate),p_status_update)


static void     cichlid_checksum_file_verifier_parent_finalized(CichlidChecksumFileVerifier *self, GObject *parent_address);
static void     cichlid_checksum_file_verifier_verify(CichlidChecksumFileVerifier *self);
static gboolean cichlid_checksum_file_verifier_update_status(CichlidChecksumFileVerifier *self);

static guint signal_verification_complete = 0;

G_DEFINE_TYPE(CichlidChecksumFileVerifier, cichlid_checksum_file_verifier, G_TYPE_OBJECT);

static void
cichlid_checksum_file_verifier_dispose(GObject *gobject)
{
	CichlidChecksumFileVerifier *self = CICHLID_CHECKSUM_FILE_VERIFIER(gobject);

	g_object_unref(self->checksum_file);
	g_mutex_free(self->status_update_lock);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(cichlid_checksum_file_verifier_parent_class)->dispose(gobject);
}

static void
cichlid_checksum_file_verifier_finalize(GObject *gobject)
{
	CichlidChecksumFileVerifier *self = CICHLID_CHECKSUM_FILE_VERIFIER(gobject);

	G_OBJECT_CLASS(cichlid_checksum_file_verifier_parent_class)->finalize(gobject);
}


static void
cichlid_checksum_file_verifier_class_init(CichlidChecksumFileVerifierClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = cichlid_checksum_file_verifier_dispose;
	gobject_class->finalize = cichlid_checksum_file_verifier_finalize;

	signal_verification_complete = g_signal_new("verification-complete",
			G_OBJECT_CLASS_TYPE(gobject_class),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET(CichlidChecksumFileVerifierClass, verification_complete),
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE, 0);
}

static void
cichlid_checksum_file_verifier_init(CichlidChecksumFileVerifier *self)
{
	/* Initiera variabler */
	self->status_update_lock = g_mutex_new();
	self->status_update_queued = FALSE;
}

CichlidChecksumFileVerifier *
cichlid_checksum_file_verifier_new(CichlidChecksumFile *checksum_file)
{
	CichlidChecksumFileVerifier *obj;
	obj = g_object_new(CICHLID_TYPE_CHECKSUM_FILE_VERIFIER, NULL);
	g_object_weak_ref(G_OBJECT(checksum_file), (GWeakNotify)cichlid_checksum_file_verifier_parent_finalized, obj);
	obj->checksum_file = checksum_file;

	return obj;
}

static void
cichlid_checksum_file_verifier_parent_finalized(CichlidChecksumFileVerifier *self, GObject *parent_address)
{
	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE_VERIFIER(self));

	g_object_unref(self);
}

gboolean
cichlid_checksum_file_verifier_start(CichlidChecksumFileVerifier *self, GError **error)
{
	GtkTreeIter iter;
	GFile* file;
	GFileInfo* info;
	hash_t cs_type;


	g_return_val_if_fail(CICHLID_IS_CHECKSUM_FILE_VERIFIER(self), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail(!self->active, FALSE);
	g_return_val_if_fail(self->checksum_file != NULL, FALSE);

	g_object_get(self->checksum_file, "hash-type", &cs_type, NULL);

	if (cs_type == HASH_UNKNOWN)
	{
		g_set_error(error, g_quark_from_string("cichlid"), CICHLID_ERROR_UNKNOWN_HASH, "The hash type cannot be unknown.");
		return FALSE;
	}

	self->active = TRUE;

	/* Calculate the number of files and their total size
	 * Size might end up negative on a 32bit system if total file size > ~2TB */
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->checksum_file),&iter))
	{
		do
		{
			gtk_tree_model_get(GTK_TREE_MODEL(self->checksum_file), &iter, CICHLID_CHECKSUM_FILE_GFILE, &file, -1);
			info = g_file_query_info(file,G_FILE_ATTRIBUTE_STANDARD_SIZE,0,NULL,NULL);
			if (info != NULL)
			{
				self->total_file_size += (int)(g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_STANDARD_SIZE) >> 10);
				g_object_unref(info);
			}
			++self->total_file_num;

			g_object_unref(file);
		} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(self->checksum_file),&iter));
	}

	/* If there are no files to verify */
	if (self->total_file_size == 0)
		return FALSE;

//	status_updates_lock = g_mutex_new();
//	progress_update_lock = g_mutex_new();
	g_thread_create((GThreadFunc)cichlid_checksum_file_verifier_verify, self, FALSE, NULL);
//	timeout_id = g_timeout_add(STATUS_UPDATE_INTERVAL,(GSourceFunc)update_file_status,NULL);

	return TRUE;
}

static void
cichlid_checksum_file_verifier_verify(CichlidChecksumFileVerifier *self)
{
	CichlidHash      *hashfunc;
	GError           *error = NULL;
	GFile            *file;
	GFileInputStream *filestream;
	GtkTreeIter       iter;
	StatusUpdate     *status_update;
	char             *buf;
	char             *filename;
	hash_t			  cs_type;
	uint32_t         *checksum;
	uint32_t         *precalculated_checksum;
	gboolean         *started;
	ssize_t           bytes_read;

	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE_VERIFIER(self));

	g_object_get(self->checksum_file, "hash-type", &cs_type, NULL);

	switch (cs_type)
	{
	case HASH_CRC32:
		hashfunc = cichlid_hash_crc32_new();
		break;
	case HASH_MD5:
		hashfunc = cichlid_hash_md5_new();
		break;
	default:
    	g_assert_not_reached();
	}

	buf = g_malloc(BUFFER_SIZE);

	/* If the ListStore is not empty, iterate over it and verify the files */
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->checksum_file), &iter))
	{
		self->current_file = NULL;
		do
		{
			/* Get the filename */
			gtk_tree_model_get(GTK_TREE_MODEL(self->checksum_file), &iter,
					CICHLID_CHECKSUM_FILE_FILENAME, &filename,
					CICHLID_CHECKSUM_FILE_GFILE, &file,
					CICHLID_CHECKSUM_FILE_CHECKSUM, &precalculated_checksum, -1);

			filestream = g_file_read(file, NULL, &error);

			/* Calculate the checksum */
			if (error == NULL)
			{
				do
				{
					bytes_read = g_input_stream_read (G_INPUT_STREAM(filestream), buf, BUFFER_SIZE, NULL, &error);
					cichlid_hash_update(hashfunc, buf, bytes_read);
					g_atomic_int_add(&self->verified_file_size, (bytes_read >> 10));
				}
				while (bytes_read > 0 && !g_atomic_int_get(&self->abort));

				checksum = cichlid_hash_get_hash(hashfunc);
			}

			/* Create a status update and add it to the list */
			status_update = cichlid_status_update_new();
			status_update->iter = iter;

			if (cichlid_hash_equals(hashfunc, checksum, precalculated_checksum))
				status_update->status = STATUS_GOOD;
			else if(error != NULL)
				status_update->status = STATUS_NOT_FOUND;
			else if(g_atomic_int_get(&self->abort))
				status_update->status = STATUS_NOT_VERIFIED;
			else
				status_update->status = STATUS_BAD;

			g_mutex_lock(self->status_update_lock);
			self->status_updates = g_list_prepend(self->status_updates,status_update);
			if (!self->status_update_queued)
				g_idle_add((GSourceFunc)cichlid_checksum_file_verifier_update_status, self);
			g_mutex_unlock(self->status_update_lock);

			/* Clean up */
			if (error != NULL)
			{
				g_error_free(error);
				error = NULL;
			}

			if (checksum != NULL)
			{
				g_free(checksum);
				checksum = NULL;
			}

			g_object_unref(file);

			if (g_atomic_int_get(&self->abort))
				break;
		}
		while(gtk_tree_model_iter_next(GTK_TREE_MODEL(self->checksum_file), &iter));

		g_free(buf);
		g_object_unref(hashfunc);
	}

	self->active = FALSE;
	/* Verification Complete signal */
	g_signal_emit(G_OBJECT (self), signal_verification_complete, 0);
}

static gboolean
cichlid_checksum_file_verifier_update_status(CichlidChecksumFileVerifier *self)
{
	GValue value = {0, };
	StatusUpdate *status_update;
	gboolean	 queue;

	g_mutex_lock(self->status_update_lock);
	for (int i = 0; i < 100 && self->status_updates; ++i)
	{
		status_update = self->status_updates->data;
		self->status_updates = g_list_remove(self->status_updates, status_update);

		g_value_init(&value, G_TYPE_INT);
		g_value_set_int(&value, status_update->status);
		cichlid_checksum_file_set(self->checksum_file, &status_update->iter, CICHLID_CHECKSUM_FILE_STATUS, &value);
		g_value_unset(&value);

		cichlid_status_update_free(status_update);
	}
	queue = (self->status_updates != NULL);

	self->status_update_queued = queue;
	g_mutex_unlock(self->status_update_lock);
	return queue;
}
