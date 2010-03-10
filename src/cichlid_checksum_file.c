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

#include "cichlid.h"
#include "cichlid_file.h"
#include "cichlid_hash.h"
#include "cichlid_checksum_file.h"
#include "cichlid_checksum_file_v.h"
#include "cichlid_marshal.h"

typedef struct _CichlidChecksumFilePrivate CichlidChecksumFilePrivate;

struct _CichlidChecksumFilePrivate
{
	CichlidChecksumFileVerifier *verifier;
	hash_t cs_type; /* Checksum type (HASH_...) */

	/* File stuff */
	GFile   *file;
	GQueue	*file_queue;
	GMutex	*file_queue_lock;
	gboolean file_parsed;

	/* Checksum Options */
	char    cs_comment;   /* Character used to prepend comments in the checksum file */
	uint8_t cs_length;    /* Length of the hash (in hex chars) */
	uint8_t cs_order;     /* Order of filename / hash */
	char    cs_separator; /* Character separating the checksum from the filename if checksum_order = CHECKSUM_LAST
	                         otherwise the number of characters between the checksum and the filename */
};

/* Properties */
enum
{
	P_HASH = 1
};

/* Column order */
enum
{
	FILENAME_COLUMN = 0,
	STATUS_COLUMN,
	GFILE_COLUMN,
	CHECKSUM_COLUMN,
	FILE_COLUMN,
	N_COLUMNS
};

enum
{
	LS_FILE_COLUMN = 0,
	LS_N_COLUMNS
};

/* Checksum position */
enum
{
	CHECKSUM_FIRST = 0,
	CHECKSUM_LAST
};

/* Signals */
enum
{
	S_FILE_LOADED = 0,
	S_VERIFICATION_PROGRESS_UPDATE,
	S_VERIFICATION_COMPLETE,
	N_SIGNALS
};

/* Parsed line info */
typedef struct
{
	GFile          *file;
	char           *name;
	int             status;
	gconstpointer	checksum;
} CichlidParsedLineContext;

static gboolean    cichlid_checksum_file_insert_files(CichlidChecksumFile *self);
static gboolean	   cichlid_checksum_file_checksum_valid(char *const p_checksum,
														guint checksum_length);
static char       *cichlid_checksum_file_get_base_path(GFile *file);
static GType       cichlid_checksum_file_get_column_type(GtkTreeModel *self,
														 int column);
static char       *cichlid_checksum_file_get_extension(const char *filename);
static int         cichlid_checksum_file_get_n_columns(GtkTreeModel *self);
static void        cichlid_checksum_file_get_value(GtkTreeModel *self,
												   GtkTreeIter *iter,
												   int column,
												   GValue *value);
static void        cichlid_checksum_file_get_property(GObject    *object,
		                                              guint       property_id,
		                                              GValue     *value,
		                                              GParamSpec *pspec);
static void        cichlid_checksum_file_parse(CichlidChecksumFile *self);
static void        cichlid_checksum_file_queue_file(CichlidChecksumFile *self,
													const char   *filename,
													const char   *base_path,
													gconstpointer checksum);
static inline void cichlid_checksum_file_read_checksum(CichlidChecksumFile *self,
													   uint32_t            *checksum,
													   const char          *checksum_start);
static void        cichlid_checksum_file_set_property(GObject      *object,
		                                              guint         property_id,
		                                              const GValue *value,
		                                              GParamSpec   *pspec);

static void        cichlid_checksum_file_set_filetype(CichlidChecksumFile *self,
													  GFile *file,
													  GError **error);
static void        cichlid_checksum_file_set_filetype_options(CichlidChecksumFile *self,
															  int hash,
															  guint length,
															  int order,
															  char separator,
															  char comment_char);
static void        gtk_tree_model_interface_init(GtkTreeModelIface* iface);



static GtkTreeModelIface parent_iface;
static unsigned int signals[N_SIGNALS];

G_DEFINE_TYPE_WITH_CODE(CichlidChecksumFile, cichlid_checksum_file, GTK_TYPE_LIST_STORE,
						G_IMPLEMENT_INTERFACE(GTK_TYPE_TREE_MODEL, gtk_tree_model_interface_init))

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
	CichlidChecksumFilePrivate *priv = self->priv;

	while (!g_queue_is_empty(priv->file_queue))
	{
		g_object_unref(g_queue_pop_head(priv->file_queue));
	}

	if (priv->file)
		g_object_unref(priv->file);
	g_object_unref(priv->verifier);
	g_queue_free(priv->file_queue);
	g_mutex_free(priv->file_queue_lock);

	
	G_OBJECT_CLASS(cichlid_checksum_file_parent_class)->dispose(gobject);
}

static void
cichlid_checksum_file_finalize(GObject *gobject)
{
	CichlidChecksumFile *self = CICHLID_CHECKSUM_FILE(gobject);

	G_OBJECT_CLASS(cichlid_checksum_file_parent_class)->finalize(gobject);
}


static void
cichlid_checksum_file_class_init(CichlidChecksumFileClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(CichlidChecksumFilePrivate));

	gobject_class->dispose = cichlid_checksum_file_dispose;
	gobject_class->finalize = cichlid_checksum_file_finalize;


	gobject_class->get_property = cichlid_checksum_file_get_property;
    gobject_class->set_property = cichlid_checksum_file_set_property;

    g_object_class_install_property(gobject_class, P_HASH,
    		                        g_param_spec_int("hash-type", NULL, "cs_type",
												     0, INT_MAX, 0,
    		                        		         G_PARAM_READWRITE));

	signals[S_FILE_LOADED] = g_signal_new("file-loaded",
										  G_TYPE_FROM_CLASS(gobject_class),
										  G_SIGNAL_RUN_LAST,
										  G_STRUCT_OFFSET(CichlidChecksumFileClass, file_loaded),
										  NULL, NULL,
										  g_cclosure_marshal_VOID__VOID,
										  G_TYPE_NONE, 0);

	signals[S_VERIFICATION_PROGRESS_UPDATE] = g_signal_new("verification-progress-update",
														   G_TYPE_FROM_CLASS(gobject_class),
														   G_SIGNAL_RUN_LAST,
														   G_STRUCT_OFFSET(CichlidChecksumFileClass,
																		   verification_progress_update),
														   NULL, NULL,
														   cichlid_marshal_VOID__DOUBLE_DOUBLE,
														   G_TYPE_NONE, 2,
														   G_TYPE_DOUBLE, G_TYPE_DOUBLE);
	
	signals[S_VERIFICATION_COMPLETE] = g_signal_new("verification-complete",
													G_TYPE_FROM_CLASS(gobject_class),
													G_SIGNAL_RUN_LAST,
													G_STRUCT_OFFSET(CichlidChecksumFileClass,
																	verification_complete),
													NULL, NULL,
													g_cclosure_marshal_VOID__VOID,
													G_TYPE_NONE, 0);
}

static void
on_verifier_progress_update(double progress, double speed, CichlidChecksumFileVerifier *ver, CichlidChecksumFile *self)
{
	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));
	g_signal_emit(G_OBJECT(self), signals[S_VERIFICATION_PROGRESS_UPDATE], 0, progress, speed);
}

static void
on_verifier_verification_complete(CichlidChecksumFile *self)
{
	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));
	g_signal_emit(G_OBJECT(self), signals[S_VERIFICATION_COMPLETE], 0);
}

static void
cichlid_checksum_file_init(CichlidChecksumFile *self)
{
	CichlidChecksumFilePrivate *priv;
	priv = self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self, CICHLID_TYPE_CHECKSUM_FILE, CichlidChecksumFilePrivate);

	/* Initiera variabler */
	GType types[] = { CICHLID_TYPE_FILE };
	gtk_list_store_set_column_types(GTK_LIST_STORE(self), LS_N_COLUMNS, types);

	priv->verifier = cichlid_checksum_file_verifier_new(self);
	g_signal_connect(G_OBJECT(priv->verifier), "progress-update",
					 G_CALLBACK(on_verifier_progress_update), self);
	g_signal_connect_swapped(G_OBJECT(priv->verifier), "verification-complete",
							 G_CALLBACK(on_verifier_verification_complete), self);

	priv->file = NULL;

	priv->file_queue = g_queue_new();
	priv->file_queue_lock = g_mutex_new();
	priv->file_parsed = FALSE;

	priv->cs_comment = '\0';
	priv->cs_length = 0;

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
		G_TYPE_OBJECT,
		G_TYPE_POINTER,
		CICHLID_TYPE_FILE
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
        CichlidFile *ck_file;

        /* validate our parameters */
        g_return_val_if_fail(CICHLID_IS_CHECKSUM_FILE(self), NULL);
        g_return_val_if_fail(iter != NULL, NULL);

        parent_iface.get_value(GTK_TREE_MODEL (self), iter, 0, &value);
        ck_file = g_value_get_object(&value);

        g_value_unset(&value);

        return ck_file;
}

static void
cichlid_checksum_file_get_value(GtkTreeModel *self, GtkTreeIter *iter, int column, GValue *value)
{
        CichlidFile *ck_file;

        g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));
        g_return_if_fail(iter != NULL);
        g_return_if_fail(column >= 0 && column < N_COLUMNS);
        g_return_if_fail(value != NULL);

        ck_file = cichlid_checksum_file_get_object(CICHLID_CHECKSUM_FILE(self), iter);
        g_return_if_fail(CICHLID_IS_FILE(ck_file));

        g_value_init(value, cichlid_checksum_file_get_column_type(self, column));

        switch (column)
        {
        case FILENAME_COLUMN:
        	g_value_set_string(value, cichlid_file_get_filename(ck_file));
        	break;
        case STATUS_COLUMN:
        	g_value_set_int(value, cichlid_file_get_status(ck_file));
        	break;
        case GFILE_COLUMN:
        	g_value_set_object(value, cichlid_file_get_file(ck_file));
        	break;
        case CHECKSUM_COLUMN:
			/* TODO */
        	g_value_set_pointer(value, NULL);
        	break;
        case FILE_COLUMN:
        	g_value_set_object(value, ck_file);
        	break;
        default:
        	g_assert_not_reached();
        }
}

static void
cichlid_checksum_file_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	CichlidChecksumFile        *self = CICHLID_CHECKSUM_FILE(object);
	CichlidChecksumFilePrivate *priv = self->priv;

	switch(property_id)
	{
	case P_HASH:
		g_value_set_int(value, priv->cs_type);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

void
cichlid_checksum_file_load_from_cmd(CichlidChecksumFile *self, const char *filename)
{	
	GFile *file;

	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));
	
	file = g_file_new_for_commandline_arg(filename);

	cichlid_checksum_file_load(self, file);
}

void
cichlid_checksum_file_load(CichlidChecksumFile *self, GFile *checksum_file)
{
	GError *error = NULL;

    g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));
	g_return_if_fail(G_IS_FILE(checksum_file));

	CichlidChecksumFilePrivate *priv = self->priv;

	if (priv->file)
	{
		g_object_unref(priv->file);
		gtk_list_store_clear(GTK_LIST_STORE(self));
	}
	
	g_object_ref(checksum_file);
	priv->file = checksum_file;

	cichlid_checksum_file_set_filetype(self, checksum_file, &error);

	priv->file_parsed = FALSE;
	
	g_thread_create((GThreadFunc)cichlid_checksum_file_parse, self, FALSE, NULL);
	g_idle_add((GSourceFunc)cichlid_checksum_file_insert_files, self);

	if (error != NULL)
	{
		g_debug("%s", error->message);
		g_error_free(error);
	}
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
 * @return returns any error as a GError.
 */
static void
cichlid_checksum_file_set_filetype(CichlidChecksumFile *self, GFile *file, GError **error)
{
	GFileInfo *info;
	const char *filename;
	char *extension;

	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));
	g_return_if_fail(file != NULL);
	g_return_if_fail(error == NULL || *error == NULL);

	info = g_file_query_info(file,G_FILE_ATTRIBUTE_STANDARD_NAME,0,NULL,error);

	if (*error != NULL)
		return;

	filename = g_file_info_get_attribute_byte_string(info,G_FILE_ATTRIBUTE_STANDARD_NAME);
	extension = cichlid_checksum_file_get_extension(filename);

	if (g_strcmp0(extension, "sfv") == 0)
		cichlid_checksum_file_set_filetype_options(self, HASH_CRC32, 8, CHECKSUM_LAST, ' ', ';');
	else if (g_strcmp0(extension, "md5") == 0 ||
			 g_utf8_collate(filename,"MD5SUM") == 0 ||
			 g_utf8_collate(filename,"MD5SUMS") == 0)
		cichlid_checksum_file_set_filetype_options(self, HASH_MD5, 32, CHECKSUM_FIRST, 2, '#');
	else if (g_strcmp0(extension, "sha256") == 0 ||
			 g_utf8_collate(filename, "SHA256SUM") == 0 ||
			 g_utf8_collate(filename, "SHA256SUMS") == 0)
		cichlid_checksum_file_set_filetype_options(self, HASH_SHA256, 64, CHECKSUM_FIRST, 2, '#');
	else
	{
		cichlid_checksum_file_set_filetype_options(self, HASH_UNKNOWN, 0, 0, '\0','\0');
		g_set_error(error, g_quark_from_string("cichlid"), CICHLID_ERROR_BADCF, "The file type of %s could not be determined",filename);
	}

	g_object_unref(info);
	g_free(extension);
}

static void
cichlid_checksum_file_set_filetype_options(CichlidChecksumFile *self, int hash, guint length, int order, char separator, char comment_char)
{
	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));

	CichlidChecksumFilePrivate *priv = self->priv;

	priv->cs_type = hash;
	priv->cs_length = length;
	priv->cs_order = order;
	priv->cs_separator = separator;
	priv->cs_comment = comment_char;
}

static void
cichlid_checksum_file_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	CichlidChecksumFile        *self = CICHLID_CHECKSUM_FILE(object);
	CichlidChecksumFilePrivate *priv = self->priv;

	switch(property_id)
	{
	case P_HASH:
		priv->cs_type = g_value_get_int(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
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

char *
cichlid_checksum_file_get_filename(CichlidChecksumFile *self)
{
	GFileInfo *info;
	gchar *filename;
	GError *error = NULL;
	
	g_return_val_if_fail(CICHLID_IS_CHECKSUM_FILE(self), NULL);
	
	CichlidChecksumFilePrivate *priv = self->priv;
	g_return_val_if_fail(G_IS_FILE(priv->file), NULL);

	info =	g_file_query_info(priv->file, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
							  G_FILE_QUERY_INFO_NONE, NULL, &error);
	if (error)
	{
		g_error_free(error);
		filename = NULL;
	}
	else
	{
		filename = g_strdup(g_file_info_get_display_name(info));
		g_object_unref(info);
	}

	return filename;
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

	CichlidChecksumFilePrivate *priv = self->priv;
	GFile *checksum_file = priv->file;
	g_assert(checksum_file != NULL);

	if (priv->cs_type == HASH_UNKNOWN)
		return;

	filestream = g_file_read(checksum_file, NULL, error);
	datastream = g_data_input_stream_new(G_INPUT_STREAM(filestream));

	base_path = cichlid_checksum_file_get_base_path(checksum_file);

	/* For each line in the file */
	while (1)
	{
		read_line = g_data_input_stream_read_line(datastream, &line_length, NULL, error);

		/* If there was an error reading the file or the entire file is read */
		if (*error != NULL || line_length == 0)
			break;

		/* If the number of read chars is too short, the line begins with a comment
		 * or a newline, skip it. */
		if (line_length < priv->cs_length + 2 ||
				*read_line == priv->cs_comment ||
				*read_line == '\n' ||
				*read_line == '\r')
		{
			g_free(read_line);
			continue;
		}

		/* Find start of the checksum if the checksum is last */
		if (priv->cs_order == CHECKSUM_LAST)
		{
			checksum_start = strrchr(read_line,' ') + 1;
			filename_start = read_line;
			filename_length = MAX(checksum_start - filename_start - 1, 0);
		}
		else
		{
			checksum_start = read_line;
			filename_start = read_line + priv->cs_length + priv->cs_separator;
			filename_length = MAX(line_length - priv->cs_length - priv->cs_separator, 0);
		}

		/* Make sure the checksum is valid (only hex chars, correct length) */
		if (checksum_start == NULL ||
				filename_length == 0 ||
				line_length - (checksum_start - read_line) < priv->cs_length ||
				!cichlid_checksum_file_checksum_valid(checksum_start, priv->cs_length))
		{
			g_free(read_line);
			continue;
		}

		/* Copy the filename to a new buffer and append '\0' */
		filename = g_slice_alloc(sizeof(char) * (filename_length + 1));
		strncpy(filename, filename_start, filename_length);
		filename[filename_length] = '\0';

		/* Do the same for the checksum */
		checksum = g_malloc(sizeof(char)*priv->cs_length/2);
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

	priv->file_parsed = TRUE;

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
	CichlidChecksumFilePrivate *priv = self->priv;
	char num[9];

	num[8] = '\0';
	for (int i = 0; i < priv->cs_length/8; i++)
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
	GFile *folder;
	GFileInfo *info;
	CichlidFile *f;
	char *name;
	char *rel_path;
	char *file_path;
	cichlid_file_status_t status;
	gboolean exists;

	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));
	g_return_if_fail(filename != NULL);
	g_return_if_fail(base_path != NULL);
	g_return_if_fail(checksum != NULL);

	CichlidChecksumFilePrivate *priv = self->priv;

	file_path = g_strconcat(base_path,G_DIR_SEPARATOR_S,filename,NULL);
	file = g_file_new_for_commandline_arg(file_path);
	
	folder = g_file_get_parent(priv->file);
	rel_path = g_file_get_relative_path(folder, file);
	name = g_filename_to_utf8(rel_path, -1, NULL, NULL, NULL);

	if (g_file_query_exists(file, NULL))
		status = STATUS_NOT_VERIFIED;
	else
		status = STATUS_NOT_FOUND;

	f = cichlid_file_new(file, checksum);
	cichlid_file_set_status(f, status);

	g_mutex_lock(priv->file_queue_lock);
	g_queue_push_tail(priv->file_queue, f);
	g_mutex_unlock(priv->file_queue_lock);

	g_object_unref(folder);
	g_free(file_path);
	g_free(rel_path);
}

static gboolean
cichlid_checksum_file_insert_files(CichlidChecksumFile *self)
{
	int i = 0;
	CichlidFile *ck_file;

	g_return_val_if_fail(CICHLID_IS_CHECKSUM_FILE(self), FALSE);

	CichlidChecksumFilePrivate *priv = self->priv;

	g_mutex_lock(priv->file_queue_lock);
	while (i < 100 && !g_queue_is_empty(priv->file_queue))
	{
		ck_file = g_queue_pop_head(priv->file_queue);
		gtk_list_store_insert_with_values(GTK_LIST_STORE(self), NULL, G_MAXINT, LS_FILE_COLUMN, ck_file, -1);
		++i;
		g_object_unref(ck_file);
	}
	g_mutex_unlock(priv->file_queue_lock);

	/* All queued files are added and the file is completely parsed */
	if (priv->file_parsed && g_queue_is_empty(priv->file_queue))
	{
		g_signal_emit(G_OBJECT (self), signals[S_FILE_LOADED], 0);
		return FALSE;
	}

	return TRUE;
}

void
cichlid_checksum_file_set(CichlidChecksumFile *self, GtkTreeIter *iter, int column, GValue *value)
{
	CichlidFile *f;
	GtkTreePath *path;

	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));
	g_return_if_fail(column >= 0 && column < N_COLUMNS);
	g_return_if_fail(G_IS_VALUE(value));

	f = cichlid_checksum_file_get_object(self, iter);

	switch (column)
	{
	case FILENAME_COLUMN:
		g_error("File name cannot be changed.");
	case STATUS_COLUMN:
		cichlid_file_set_status(f, g_value_get_int(value));
		break;
	case GFILE_COLUMN:
		g_error("GFile cannot be changed.");
		break;
	case CHECKSUM_COLUMN:
		g_error("Checksum cannot be updated");

	default:
		g_assert_not_reached();
	}

	path = gtk_tree_model_get_path(GTK_TREE_MODEL(self), iter);
	gtk_tree_model_row_changed(GTK_TREE_MODEL(self), path, iter);
	gtk_tree_path_free (path);
}
void cichlid_checksum_file_cancel_verification(CichlidChecksumFile *self)
{
	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));

	CichlidChecksumFilePrivate *priv = self->priv;
	cichlid_checksum_file_verifier_cancel(priv->verifier);
}

void cichlid_checksum_file_verify(CichlidChecksumFile *self)
{
	g_return_if_fail(CICHLID_IS_CHECKSUM_FILE(self));

	CichlidChecksumFilePrivate *priv = self->priv;
	if (!cichlid_checksum_file_verifier_start(priv->verifier, NULL))
		g_signal_emit(G_OBJECT(self), signals[S_VERIFICATION_COMPLETE], 0);
}
