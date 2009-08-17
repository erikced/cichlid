/*
 * Copyright © 2008-2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_checksum_file.c
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
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cichlid_hash.h"
#include "cichlid_checksum_file.h"

/* Column order */
enum
{
	FILENAME_COLUMN = 0,
	STATUS_COLUMN,
	N_COLUMNS
};

enum
{
	LS_FILE_COLUMN = 0
};

/* Checksum position */
enum
{
	CHECKSUM_FIRST = 0,
	CHECKSUM_LAST
};

/* File */
typedef struct
{
	GFile         *file;
	char          *name;
	int            status;
	gconstpointer  checksum;
} CichlidFile;

#define cichlid_file_new() g_slice_alloc(sizeof(CichlidFile))
#define cichlid_file_free(p_file) g_slice_free1(sizeof(CichlidFile),p_file)

/* Parsed line info */
typedef struct
{
	GFile          *file;
	char           *name;
	int             status;
	gconstpointer	checksum;
} CichlidParsedLineContext;

static gboolean    cichlid_checksum_file_insert_files(CichlidChecksumFile *self);
static gboolean	   cichlid_checksum_file_checksum_valid(char *const p_checksum, guint checksum_length);
static GType       cichlid_checksum_file_get_column_type(GtkTreeModel *self, int column);
static int         cichlid_checksum_file_get_n_columns(GtkTreeModel *self);
static void        cichlid_checksum_file_get_value(GtkTreeModel *self, GtkTreeIter *iter, int column, GValue *value);
static char       *cichlid_checksum_file_get_base_path(GFile *file);
static char       *cichlid_checksum_file_get_extension(const char *filename);
static void        cichlid_checksum_file_parse(CichlidChecksumFile *self);
static void        cichlid_checksum_file_queue_file(CichlidChecksumFile *self, const char *filename, const char *base_path, gconstpointer checksum);
static inline void cichlid_checksum_file_read_checksum(CichlidChecksumFile *self, uint32_t *checksum, const char *checksum_start);
static void        cichlid_checksum_file_set_filetype(CichlidChecksumFile *self, GFile *file, GError **error);
static void        cichlid_checksum_file_set_filetype_options(CichlidChecksumFile *self, int hash, guint length, int order, char separator, char comment_char);
static void        gtk_tree_model_interface_init(GtkTreeModelIface* iface);


static GtkTreeModelIface parent_iface;
static guint signal_file_loaded = 0;

G_DEFINE_TYPE_WITH_CODE(CichlidChecksumFile, cichlid_checksum_file, GTK_TYPE_LIST_STORE,
		G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_MODEL, gtk_tree_model_interface_init))

/*
 * Initialize the interface
 */
static void
gtk_tree_model_interface_init(GtkTreeModelIface *iface)
{
	/* Rely on the parents implementation for most methods */
	parent_iface = *iface;

	iface->get_n_columns = cichlid_checksum_file_get_n_columns;
	iface->get_column_type = cichlid_checksum_file_get_column_type;
	iface->get_value = cichlid_checksum_file_get_value;
}

static void
cichlid_checksum_file_dispose(GObject *gobject)
{
	CichlidChecksumFile *self = CICHLID_CHECKSUM_FILE(gobject);

	g_object_unref(self->file);
	g_queue_free(self->file_queue);
	g_mutex_free(self->file_queue_lock);

	/* Chain up to the parent class */
	G_OBJECT_CLASS (cichlid_checksum_file_parent_class)->dispose (gobject);
}

static void
cichlid_checksum_file_finalize(GObject *gobject)
{
	CichlidChecksumFile *self = CICHLID_CHECKSUM_FILE(gobject);

	G_OBJECT_CLASS (cichlid_checksum_file_parent_class)->finalize (gobject);
}


static void
cichlid_checksum_file_class_init(CichlidChecksumFileClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->dispose = cichlid_checksum_file_dispose;
	gobject_class->finalize = cichlid_checksum_file_finalize;

	signal_file_loaded = g_signal_new ("file-loaded",
			G_OBJECT_CLASS_TYPE(gobject_class),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET (CichlidChecksumFileClass, file_loaded),
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE, 0);

}


static void
cichlid_checksum_file_init(CichlidChecksumFile *self)
{


	/* Initiera variabler */
	GType types[] = { G_TYPE_POINTER };
	gtk_list_store_set_column_types(GTK_LIST_STORE(self), 1, types);

	self->file_queue = g_queue_new();
	self->file_queue_lock = g_mutex_new ();
	self->file_parsed = FALSE;

	self->cs_comment = '\0';
	self->cs_length = 0;
}

static int
cichlid_checksum_file_get_n_columns(GtkTreeModel *self)
{
        g_return_val_if_fail(CICHLID_IS_CHECKSUM_FILE(self), 0);

        return N_COLUMNS;
}
static GType
cichlid_checksum_file_get_column_type(GtkTreeModel *self, int column)
{
	GType types[] = {
		G_TYPE_STRING,
		G_TYPE_INT,
	};

	/* validate our parameters */
	g_return_val_if_fail(CICHLID_IS_CHECKSUM_FILE(self), G_TYPE_INVALID);
	g_return_val_if_fail(column >= 0 && column < N_COLUMNS, G_TYPE_INVALID);

	return types[column];
}

static CichlidFile *
cichlid_checksum_file_get_object(CichlidChecksumFile *self, GtkTreeIter *iter)
{
        GValue value = { 0, };
        CichlidFile *obj;

        /* validate our parameters */
        g_return_val_if_fail(CICHLID_IS_CHECKSUM_FILE(self), NULL);
        g_return_val_if_fail(iter != NULL, NULL);

        parent_iface.get_value(GTK_TREE_MODEL (self), iter, 0, &value);
        obj = g_value_get_pointer(&value);

        g_value_unset(&value);

        return obj;
}

static void
cichlid_checksum_file_get_value(GtkTreeModel *self, GtkTreeIter *iter, int column, GValue *value)
{
        CichlidFile *cf;

        g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));
        g_return_if_fail(iter != NULL);
        g_return_if_fail(column >= 0 && column < N_COLUMNS);
        g_return_if_fail(value != NULL);

        cf = cichlid_checksum_file_get_object(CICHLID_CHECKSUM_FILE(self), iter);

        g_value_init(value, cichlid_checksum_file_get_column_type(self, column));

        switch (column)
        {
        case FILENAME_COLUMN:
        	g_value_set_string(value, cf->name);
        	break;

        case STATUS_COLUMN:
        	g_value_set_int(value, cf->status);
        	break;

        default:
        	g_assert_not_reached();
        }
}

gboolean
cichlid_checksum_file_load_async(CichlidChecksumFile *self, const char *filename)
{
	GFile *file;
	file = g_file_new_for_commandline_arg(filename);

	return FALSE;
}

void
cichlid_checksum_file_load(CichlidChecksumFile *self, GFile *checksum_file)
{
	GError *error = NULL;
	GFileInfo *file_info;
	char *filename;

    g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));
	g_return_if_fail(checksum_file != NULL);

	g_object_ref(checksum_file);
	self->file = checksum_file;

	cichlid_checksum_file_set_filetype(self, checksum_file, &error);

	/* Set cursor */
	//cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_WATCH);
	//gdk_window_set_cursor(main_window->window,cursor);
	//while (g_main_context_iteration(NULL, FALSE));

	/* Set the window title to the file name if possible
	 * else just use the app name */
	file_info = g_file_query_info(checksum_file,G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,0,NULL,NULL);
	if (file_info != NULL)
	{
		filename = g_strdup(g_file_info_get_attribute_string(file_info,G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME));
		g_free(filename);
		g_object_unref(file_info);

	}

	g_idle_add((GSourceFunc)cichlid_checksum_file_insert_files, self);
	//cichlid_checksum_file_parse(self);
	g_thread_create((GThreadFunc)cichlid_checksum_file_parse, self, FALSE, NULL);

	if (error != NULL)
		g_error_free(error);

	g_signal_emit(G_OBJECT (self), signal_file_loaded, 0);

}

/**
 * Returns the file extension of the specified file.
 * @param the filename as a NULL-terminated string.
 * @return NULL if unsuccessful, a newly created string containing the file extension otherwise.
 * Should be freed with g_free() when no longer needed.
 */
static char *
cichlid_checksum_file_get_extension(const char *filename)
{
	char *p_ext_start;
	char *extension = NULL;

	p_ext_start = strrchr(filename,'.') + 1;
	if (p_ext_start > filename)
		extension = g_ascii_strdown(p_ext_start,-1);

	return extension;
}


/**
 * Sets the file type after comparing the name of the GFile and its extension to a a few predefined values.
 * @param the file as a GFile
 * @param returns any error as a GError.
 */
static void
cichlid_checksum_file_set_filetype(CichlidChecksumFile *self, GFile *file, GError **error)
{
	GFileInfo *info;
	const char *filename;
	char *extension;

	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));
	g_return_if_fail(file != NULL);

	info = g_file_query_info(file,G_FILE_ATTRIBUTE_STANDARD_NAME,0,NULL,error);

	if (*error != NULL)
		return;

	filename = g_file_info_get_attribute_byte_string(info,G_FILE_ATTRIBUTE_STANDARD_NAME);
	extension = cichlid_checksum_file_get_extension(filename);

	if (g_strcmp0(extension,"sfv") == 0)
		cichlid_checksum_file_set_filetype_options(self, HASH_CRC32, 8, CHECKSUM_LAST, ' ', ';');
	else if (g_strcmp0(extension,"md5") == 0 || g_utf8_collate(filename,"MD5SUM") == 0 || g_utf8_collate(filename,"MD5SUMS") == 0)
		cichlid_checksum_file_set_filetype_options(self, HASH_MD5, 32, CHECKSUM_FIRST, 2, '#');
	else
	{
		cichlid_checksum_file_set_filetype_options(self, HASH_UNKNOWN, 0, 0, '\0','\0');
		*error = g_error_new(g_quark_from_string("cichlid"),CICHLID_ERROR_BADCF,"The file type of %s could not be determined",filename);
	}

	g_object_unref(info);
	g_free(extension);
}

static void
cichlid_checksum_file_set_filetype_options(CichlidChecksumFile *self, int hash, guint length, int order, char separator, char comment_char)
{
	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));

	self->cs_type = hash;
	self->cs_length = length;
	self->cs_order = order;
	self->cs_separator = separator;
	self->cs_comment = comment_char;
}

/**
 * Makes sure that the checksum contains of only hexadecimal characters
 * @param a pointer to the first character of the checksum
 * @param the length the checksum should have
 * @return TRUE if the first length characters of p_checksum are hexadecimal, FALSE otherwise
 */
static gboolean
cichlid_checksum_file_checksum_valid(char *const p_checksum, guint checksum_length)
{
	gboolean valid;

	g_assert(p_checksum != NULL && checksum_length > 0);

	valid = TRUE;

	for (guint i = 0; i < checksum_length; ++i)
	{
		if (!g_ascii_isxdigit(p_checksum[i]))
		{
			valid = FALSE;
			break;
		}
	}

	return valid;
}

static char *
cichlid_checksum_file_get_base_path(GFile *file)
{
	char *file_base_path;
	GFile *file_parent;

	g_return_val_if_fail(file != NULL, "");

	file_parent = g_file_get_parent(file);
	file_base_path = g_file_get_path(file_parent);
	g_object_unref(file_parent);

	return file_base_path;
}

static void
cichlid_checksum_file_parse(CichlidChecksumFile *self)
{
	GError *err = NULL;
	GError **error = &err;
	GFileInputStream *filestream;
	GDataInputStream *datastream;
	char *read_line,
	     *checksum_start, /* Start of the checksum in the read line */
	     *filename_start, /* Start of the filename in the read line */
	     *base_path,
	     *filename;
	size_t line_length,
	       filename_length;
	gpointer checksum;

	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));

	GFile *checksum_file = self->file;
	g_assert(checksum_file != NULL);

	self->file_parsed = FALSE;

	if (self->cs_type == HASH_UNKNOWN)
		return;

	filestream = g_file_read(checksum_file, NULL, error);
	datastream = g_data_input_stream_new(G_INPUT_STREAM(filestream));

	base_path = cichlid_checksum_file_get_base_path(checksum_file);

	uint32_t i = 0;
	/* For each line in the file */
	while (1)
	{
		++i;
		read_line = g_data_input_stream_read_line(datastream, &line_length, NULL, error);

		/* If there was an error reading the file or the entire file is read */
		if (*error != NULL || line_length == 0)
			break;

		/* If the number of read chars is too short, the line begins with a comment
		 * or a newline, skip it. */
		if (line_length < self->cs_length + 2 ||
				*read_line == self->cs_comment ||
				*read_line == '\n' ||
				*read_line == '\r')
		{
			g_free(read_line);
			continue;
		}

		/* Find start of the checksum if the checksum is last */
		if (self->cs_order == CHECKSUM_LAST)
		{
			checksum_start = strrchr(read_line,' ') + 1;
			filename_start = read_line;
			filename_length = MAX(checksum_start - filename_start - 1, 0);
		}
		else
		{
			checksum_start = read_line;
			filename_start = read_line + self->cs_length + self->cs_separator;
			filename_length = MAX(line_length - self->cs_length - self->cs_separator, 0);
		}

		/* Make sure the checksum is valid (only hex chars, correct length) */
		if (checksum_start == NULL ||
				filename_length == 0 ||
				line_length - (checksum_start - read_line) < self->cs_length ||
				!cichlid_checksum_file_checksum_valid(checksum_start, self->cs_length))
		{
			g_free(read_line);
			continue;
		}

		/* Copy the filename to a new buffer and append '\0' */
		filename = g_slice_alloc(sizeof(char) * (filename_length + 1));
		strncpy(filename, filename_start, filename_length);
		filename[filename_length] = '\0';

		/* Do the same for the checksum */
		checksum = g_malloc(sizeof(char)*self->cs_length/2);
		cichlid_checksum_file_read_checksum(self, checksum, checksum_start);

		/* Add the file to the list */
		cichlid_checksum_file_queue_file(self, filename, base_path, checksum);

		/* Clean up */
		g_free (read_line);
		g_slice_free1(sizeof(char) * (filename_length + 1), filename);
		filename = NULL;
		checksum = NULL;
		checksum_start = NULL;
		filename_start = NULL;
	}

	if (*error != NULL)
		g_error_free(*error);

	self->file_parsed = TRUE;
	g_debug("Files Added: %i",i);

	g_free(base_path);
	g_object_unref(datastream);
	g_object_unref(filestream);
}

/*
 * Reads the checksum from checksum_start and stores it as an array of unsigned 32-bit integers
 */
static inline void
cichlid_checksum_file_read_checksum(CichlidChecksumFile *self, uint32_t *checksum, const char *checksum_start)
{
	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));

	char num[9];
	num[8] = '\0';
	for (int i = 0; i < self->cs_length/8; i++)
	{
		memcpy(num, checksum_start + 8*i, 8);
		checksum[i] = strtoul(num, NULL, 16);
	}
}

/*
 * Queue file to be added
 */
static void
cichlid_checksum_file_queue_file(CichlidChecksumFile *self, const char *filename, const char *base_path, gconstpointer checksum)
{
	GFile *file;
	GFileInfo *info;
	CichlidFile *f;
	const char *name = NULL;
	char *file_path;
	int status;

	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));
	g_return_if_fail(filename != NULL);
	g_return_if_fail(base_path != NULL);
	g_return_if_fail(checksum != NULL);


	file_path = g_strconcat(base_path,G_DIR_SEPARATOR_S,filename,NULL);

	file = g_file_new_for_commandline_arg(file_path);

	/* Get the filename */
	info = g_file_query_info(file,G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,0,NULL,NULL);
	if (info != NULL)
		name = g_file_info_get_attribute_string(info,G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);

	if (name == NULL)
	{
		name = filename;
		status = STATUS_NOT_FOUND;
	}
	else
		status = STATUS_NOT_VERIFIED;

	f = cichlid_file_new();
	f->file = file;
	f->name = g_strdup(name);
	f->status = status;
	f->checksum = checksum;

	g_mutex_lock(self->file_queue_lock);
	g_queue_push_tail(self->file_queue, f);
	g_mutex_unlock(self->file_queue_lock);

	if (info != NULL)
		g_object_unref(info);
	g_free(file_path);
}

static gboolean
cichlid_checksum_file_insert_files(CichlidChecksumFile *self)
{
	int i = 0;

	g_return_val_if_fail(CICHLID_IS_CHECKSUM_FILE(self), FALSE);

	g_mutex_lock(self->file_queue_lock);
	while (i < 100 && !g_queue_is_empty(self->file_queue))
	{
		gtk_list_store_insert_with_values(GTK_LIST_STORE(self), NULL, G_MAXINT, LS_FILE_COLUMN, g_queue_pop_head(self->file_queue), -1);
		++i;
	}
	g_mutex_unlock(self->file_queue_lock);

	if (self->file_parsed && g_queue_is_empty(self->file_queue))
		return FALSE;

	return TRUE;
}
