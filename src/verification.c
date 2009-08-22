/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - verification.c
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

#include "cichlid_checksum_file.h"
#include "cichlid.h"
#include "cichlid_hash.h"
#include "cichlid_hash_crc32.h"
#include "cichlid_hash_md5.h"
#include "gui.h"
#include "verification.h"

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

static void     verify_files(gpointer data);
static gboolean verification_complete(gboolean *started);
static gboolean update_file_status(gpointer data);

static gboolean 	 active = FALSE;			/* True if verification is in progress */
static guint 		 timeout_id = 0,			/* ID of the g_timeout which runs update_file_status */
                     current_file_num = 0,	/* Index of current file */
                     total_file_num = 0;		/* Total number of files */
static int 	     	 total_file_size = 0;	/* Size of all files (bytes / 1024) */
static volatile int  verified_file_size = 0;/* Size of verified data (bytes / 1024) */
static char 		*current_file;			/* Filename of current file */
static GList 		*status_updates = NULL;	/* List of StatusUpdates for updated entries */
static GMutex 		*status_updates_lock;	/* Lock used when adding/removing StatusUpdates from status_updates */
static GMutex 		*progress_update_lock;	/* Lock used when changing current_file */
static volatile int  abort = 0;			/* Abort flag, set to >0 to abort */

gboolean
cichlid_verification_start()
{
	GtkTreeIter iter;
	GFile* file;
	GFileInfo* info;

	/* Set status as active */
	if (active || hash_type == HASH_UNKNOWN)
		return FALSE;
	active = TRUE;

	ck_progress_dialog_new(&abort);

	/* Calculate the number of files and their total size
	 * Size might end up negative on a 32bit system if total file size > ~2TB */
//	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(files),&iter))
//	{
//		do
//		{
//			gtk_tree_model_get(GTK_TREE_MODEL(files), &iter, GFILE, &file, -1);
//			info = g_file_query_info(file,G_FILE_ATTRIBUTE_STANDARD_SIZE,0,NULL,NULL);
//			if (info != NULL)
//			{
//				total_file_size += (int)(g_file_info_get_attribute_uint64(info,G_FILE_ATTRIBUTE_STANDARD_SIZE) >> 10);
//				g_object_unref(info);
//			}
//			++total_file_num;
//		} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(files),&iter));
//	}

	/* If there are no files to verify */
	if (total_file_size == 0)
	{
		gboolean *started = g_slice_alloc(sizeof(gboolean));
		*started = FALSE;
		verification_complete(started);
		return FALSE;
	}

	status_updates_lock = g_mutex_new();
	progress_update_lock = g_mutex_new();
	g_thread_create((GThreadFunc)verify_files,NULL,FALSE,NULL);
	timeout_id = g_timeout_add(STATUS_UPDATE_INTERVAL,(GSourceFunc)update_file_status,NULL);

	return TRUE;
}

static void
verify_files(gpointer data)
{
	CichlidHash      *hashfunc;
	GError           *error = NULL;
	GFile            *file;
	GFileInputStream *filestream;
	GtkTreeIter       iter;
	StatusUpdate     *status_update;
	char             *buf;
	char             *filename;
	uint32_t         *checksum;
	uint32_t         *precalculated_checksum;
	gboolean         *started;
	ssize_t           bytes_read;

	switch (hash_type)
	{
	case HASH_CRC32:
		hashfunc = cichlid_hash_crc32_new();
		break;
	case HASH_MD5:
		hashfunc = cichlid_hash_md5_new();
		break;
	default:
		hashfunc = NULL;
		break;
	}

	buf = g_malloc(BUFFER_SIZE);

	/* If the ListStore is not empty, iterate over it and verify the files */
	//if (hashfunc != NULL && gtk_tree_model_get_iter_first(GTK_TREE_MODEL(files),&iter))
	{
		current_file = NULL;
		//do
		{
			/* Get the filename */
		//	gtk_tree_model_get(GTK_TREE_MODEL(files), &iter, NAME, &filename, GFILE, &file, PRECALCULATED_CHECKSUM, &precalculated_checksum, -1);

			/* Update the verification status */
			g_mutex_lock(progress_update_lock);
			++current_file_num;
			if (current_file)
				g_free(current_file);
			current_file = filename;
			g_mutex_unlock(progress_update_lock);

			filestream = g_file_read(file, NULL, &error);

			/* Calculate the checksum */
			if (error == NULL)
			{
				do
				{
					bytes_read = g_input_stream_read (G_INPUT_STREAM(filestream), buf, BUFFER_SIZE, NULL, &error);
					cichlid_hash_update(hashfunc, buf, bytes_read);
					g_atomic_int_add(&verified_file_size, (bytes_read >> 10));
				}
				while (bytes_read > 0 && !g_atomic_int_get(&abort));

				checksum = cichlid_hash_get_hash(hashfunc);
			}

			/* Create a status update and add it to the list */
			status_update = cichlid_status_update_new();
			status_update->iter = iter;

			if (cichlid_hash_equals(hashfunc, checksum, precalculated_checksum))
				status_update->status = STATUS_GOOD;
			else if(error != NULL)
				status_update->status = STATUS_NOT_FOUND;
			else if(g_atomic_int_get(&abort))
				status_update->status = STATUS_NOT_VERIFIED;
			else
				status_update->status = STATUS_BAD;

			g_mutex_lock(status_updates_lock);
			status_updates = g_list_prepend(status_updates,status_update);
			g_mutex_unlock(status_updates_lock);


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

//			if (g_atomic_int_get(&abort))
//				break;
		}
		//while(gtk_tree_model_iter_next(GTK_TREE_MODEL(files),&iter));

		g_free(buf);
		g_object_unref(hashfunc);
	}

	/* Run a callback in the main thread when done */
	started = g_slice_alloc(sizeof(gboolean));
	*started = TRUE;
	g_idle_add((GSourceFunc)verification_complete, started);
}

static gboolean
verification_complete(gboolean* started)
{
	/* Clean up if started */
	if(*started)
	{
		g_source_remove(timeout_id);
		update_file_status(NULL);
		g_free(current_file);
		g_mutex_free(status_updates_lock);
		g_mutex_free(progress_update_lock);
	}

	g_slice_free1(sizeof(gboolean),started);
	timeout_id			= 0;
	current_file 		= NULL;
	current_file_num 	= 0;
	total_file_num		= 0;
	verified_file_size	= 0;
	total_file_size		= 0;
	abort				= 0;
	active 				= FALSE;

	/* Remove the progress window */
	ck_progress_dialog_delete();

	return FALSE;
}

void
cichlid_verification_cancel()
{
	g_atomic_int_set(&abort,1);
}

static gboolean
update_file_status(gpointer data)
{
	StatusUpdate	  *status_update;
	GtkTreePath       *path,
	                  *filter_path = NULL;
	GtkTreeViewColumn *column;
	gboolean 		   updated = FALSE;

	/* Update the progress dialog */
	int verified_file_size_tmp = g_atomic_int_get(&verified_file_size);
	ck_progress_dialog_update(progress_update_lock,
			current_file,
			&current_file_num,
			&total_file_num,
			&verified_file_size_tmp,
			&total_file_size);

	/* Update the filelist */
	g_mutex_lock(status_updates_lock);

	while(status_updates)
	{
		status_update = status_updates->data;
		//gtk_list_store_set(files,&status_update->iter,STATUS,status_update->status,-1);
		status_updates = g_list_remove(status_updates,status_update);

		/* If this is the last updated post shown and the treeview is not selected, update its position */
		if (!updated && !GTK_WIDGET_HAS_FOCUS(filelist))
		{
			//path = gtk_tree_model_get_path (GTK_TREE_MODEL(files), &status_update->iter);
			if (path)
				filter_path = gtk_tree_model_filter_convert_child_path_to_path (GTK_TREE_MODEL_FILTER(files_filter), path);

			if (filter_path)
			{
				column = gtk_tree_view_get_column (GTK_TREE_VIEW(filelist), 0);
				gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW(filelist), filter_path, column, TRUE, 1, 0);
				updated = TRUE;
			}
			gtk_tree_path_free(path);
			gtk_tree_path_free(filter_path);
		}

		cichlid_status_update_free(status_update);
	}
	g_mutex_unlock(status_updates_lock);
	return TRUE;
}
