/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - checksum_file.c
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
#include <stdlib.h>
#include <stdint.h>

#include "cichlid.h"
#include "gui.h"
#include "checksum_file.h"
#include "CichlidFile.h"

typedef struct
{
	GFile		   *file;
	char 		   *name;
	int			status;
	gconstpointer	checksum;
} CichlidParsedLineInfo;

#define cichlid_parsed_line_info_new() g_slice_alloc(sizeof(CichlidParsedLineInfo))
#define cichlid_parsed_line_info_free(p_pli) g_slice_free1(sizeof(CichlidParsedLineInfo),p_pli)

enum
{
	CHECKSUM_FIRST = 1,
	CHECKSUM_LAST
};

static guint        checksum_length = 0;		  /* Length of the hash (in hex chars) */
static int          checksum_order = 0;			  /* Order of filename / hash */
static char         checksum_separator = '\0';	  /* Character separating the checksum from the filename if checksum_order = CHECKSUM_LAST
												     otherwise the number of characters between the checksum and the filename */
static char         checksum_comment_char = '\0'; /* Character used to prepend comments in the checksum file */
static GList       *parsed_lines = NULL;
static GStaticMutex parsed_lines_lock = G_STATIC_MUTEX_INIT;
static guint        idle_func_id = 0;
static GdkCursor    *cursor;

static gboolean	   checksum_file_add_file_to_list (gpointer data);
static gboolean	   checksum_file_checksum_valid (char *const p_checksum, guint checksum_length);
static gboolean    checksum_file_finish_load (gpointer data);
static char       *checksum_file_get_base_path (GFile *file);
static char       *checksum_file_get_extension (const char *filename);
static void        checksum_file_parse (GFile *checksum_file);
static inline void checksum_file_read_checksum (uint32_t *checksum, const char *checksum_start);
static void        checksum_file_set_filetype (GFile *file, GError **error);
static void        checksum_file_set_filetype_options (int hash, guint length, int order, char separator, char comment_char);
static void        checksum_file_queue_line (const char *filename, const char *base_path, gconstpointer checksum);


/**
 * Returns the file extension of the specified file.
 * @param the filename as a NULL-terminated string.
 * @return NULL if unsuccessful, a newly created string containing the file extension otherwise.
 * Should be freed with g_free() when no longer needed.
 */
static char *
checksum_file_get_extension(const char *filename)
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
checksum_file_set_filetype(GFile *file, GError **error)
{
	GFileInfo *info;
	const char *filename;
	char *extension;

	info = g_file_query_info(file,G_FILE_ATTRIBUTE_STANDARD_NAME,0,NULL,error);
	if (*error != NULL)
		return;

	filename = g_file_info_get_attribute_byte_string(info,G_FILE_ATTRIBUTE_STANDARD_NAME);
	extension = checksum_file_get_extension(filename);

	if (g_strcmp0(extension,"sfv") == 0)
		checksum_file_set_filetype_options (HASH_CRC32, 8, CHECKSUM_LAST, ' ', ';');
	else if (g_strcmp0(extension,"md5") == 0 || g_utf8_collate(filename,"MD5SUM") == 0 || g_utf8_collate(filename,"MD5SUMS") == 0)
		checksum_file_set_filetype_options (HASH_MD5, 32, CHECKSUM_FIRST, 2, '#');
	else
	{
		checksum_file_set_filetype_options(HASH_UNKNOWN, 0, 0, '\0','\0');
		*error = g_error_new(g_quark_from_string("cichlid"),CICHLID_ERROR_BADCF,"The file type of %s could not be determined",filename);
	}

	g_object_unref(info);
	g_free(extension);
}

static void
checksum_file_set_filetype_options(int hash, guint length, int order, char separator, char comment_char)
{
	hash_type = hash;
	checksum_length = length;
	checksum_order = order;
	checksum_separator = separator;
	checksum_comment_char = comment_char;
}

/**
 * Makes sure that the checksum contains of only hexadecimal characters
 * @param a pointer to the first character of the checksum
 * @param the length the checksum should have
 * @return TRUE if the first length characters of p_checksum are hexadecimal, FALSE otherwise
 */
static gboolean
checksum_file_checksum_valid(char *const p_checksum, guint checksum_length)
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
checksum_file_get_base_path(GFile *file)
{
	char *file_base_path;
	GFile *file_parent;

	file_parent = g_file_get_parent(file);
	file_base_path = g_file_get_path(file_parent);
	g_object_unref(file_parent);

	return file_base_path;
}

gboolean
checksum_file_load_init(char *filename)
{
	GFile *checksum_file;

	checksum_file = g_file_new_for_commandline_arg(filename);
	checksum_file_load(checksum_file);
	g_object_unref(checksum_file);

	return FALSE;
}

void
checksum_file_load(GFile *checksum_file)
{
	GError *error = NULL;
	GFileInfo *file_info;
	char *filename;

	g_assert(checksum_file != NULL);

	checksum_file_set_filetype(checksum_file, &error);

	/* Set cursor */
	cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_WATCH);
	gdk_window_set_cursor(main_window->window,cursor);
	while (g_main_context_iteration(NULL, FALSE));

	/* Set the window title to the file name if possible
	 * else just use the app name */
	file_info = g_file_query_info(checksum_file,G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,0,NULL,NULL);
	if (file_info != NULL)
	{
		filename = g_strdup(g_file_info_get_attribute_string(file_info,G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME));
		gtk_window_set_title(GTK_WINDOW(main_window),filename);
		g_free(filename);
		g_object_unref(file_info);

	}
	else
		gtk_window_set_title(GTK_WINDOW(main_window),"cichlid");

	/* Parse the file */
	if (error == NULL)
	{
		ck_gui_allow_actions(FALSE);
		//checksum_file_parse(checksum_file);
		g_thread_create((GThreadFunc)checksum_file_parse,checksum_file,FALSE,NULL);
		idle_func_id = g_idle_add((GSourceFunc)checksum_file_add_file_to_list, NULL);
	}
	else
	{
		gtk_window_set_title(GTK_WINDOW(main_window),"cichlid");
		g_error_free(error);
	}
}

static void
checksum_file_parse (GFile *checksum_file)
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

	g_assert(checksum_file != NULL && *error == NULL);

	if (hash_type == HASH_UNKNOWN)
		return;

	filestream = g_file_read(checksum_file, NULL, error);
	datastream = g_data_input_stream_new(G_INPUT_STREAM(filestream));

	base_path = checksum_file_get_base_path(checksum_file);

	/* For each line in the file */
	while (1)
	{
		read_line = g_data_input_stream_read_line(datastream, &line_length, NULL, error);

		/* If there was an error reading the file or the entire file is read */
		if (*error != NULL || line_length == 0)
			break;

		/* If the number of read chars is too short, the line begins with a comment
		 * or a newline, skip it. */
		if (line_length < checksum_length + 2 ||
				*read_line == checksum_comment_char ||
				*read_line == '\n' ||
				*read_line == '\r')
		{
			g_free(read_line);
			continue;
		}

		/* Find start of the checksum if the checksum is last */
		if (checksum_order == CHECKSUM_LAST)
		{
			checksum_start = strrchr(read_line,' ') + 1;
			filename_start = read_line;
			filename_length = MAX(checksum_start - filename_start - 1, 0);
		}
		else
		{
			checksum_start = read_line;
			filename_start = read_line + checksum_length + checksum_separator;
			filename_length = MAX(line_length - checksum_length - checksum_separator, 0);
		}

		/* Make sure the checksum is valid (only hex chars, correct length) */
		if (checksum_start == NULL ||
				filename_length == 0 ||
				line_length - (checksum_start - read_line) < checksum_length ||
				!checksum_file_checksum_valid(checksum_start, checksum_length))
		{
			g_free(read_line);
			continue;
		}

		/* Copy the filename to a new buffer and append '\0' */
		filename = g_slice_alloc(sizeof(char) * (filename_length + 1));
		strncpy(filename, filename_start, filename_length);
		filename[filename_length] = '\0';

		/* Do the same for the checksum */
		checksum = g_malloc(sizeof(char)*checksum_length/2);
		checksum_file_read_checksum(checksum, checksum_start);

		/* Add the file to the liststore */
		checksum_file_queue_line (filename, base_path, checksum);

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

	g_free(base_path);
	g_object_unref(datastream);
	g_object_unref(filestream);

	g_idle_add((GSourceFunc)checksum_file_finish_load, NULL);
}

/*
 * Reads the checksum from checksum_start and stores it as an array of unsigned 32-bit integers
 */
static inline void
checksum_file_read_checksum (uint32_t *checksum, const char *checksum_start)
{
	char num[9];
	num[8] = '\0';
	for (int i = 0; i < checksum_length/8; i++)
	{
		memcpy(num, checksum_start + 8*i, 8);
		checksum[i] = strtoul(num, NULL, 16);
	}
}

static void
checksum_file_queue_line (const char *filename, const char *base_path, gconstpointer checksum)
{
	GFile *file;
	GFileInfo *info;
	CichlidParsedLineInfo *lineinfo;
	const char *name = NULL;
	char *file_path;
	int status;

	file_path = g_strconcat(base_path,G_DIR_SEPARATOR_S,filename,NULL);

	file = g_file_new_for_commandline_arg(file_path);

	/* Get the filename */
	info = g_file_query_info(file,G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,0,NULL,NULL);
	if (info != NULL)
		name = g_file_info_get_attribute_string(info,G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);

	if (name == NULL)
	{
		name = filename;
		status = NOT_FOUND;
	}
	else
		status = NOT_VERIFIED;

	lineinfo = cichlid_parsed_line_info_new();
	lineinfo->file = file;
	lineinfo->name = g_strdup(name);
	lineinfo->status = status;
	lineinfo->checksum = checksum;

	g_static_mutex_lock(&parsed_lines_lock);
	parsed_lines = g_list_append(parsed_lines, lineinfo);
	g_static_mutex_unlock(&parsed_lines_lock);

	if (info != NULL)
		g_object_unref(info);
	g_free(file_path);
}

static gboolean
checksum_file_add_file_to_list (gpointer data)
{
	GList *list = NULL;
	CichlidParsedLineInfo *pli;
	int i;

	/* Copy the list to a local variable */
	g_static_mutex_lock(&parsed_lines_lock);
	for (i = 0; i < 100 && parsed_lines; i++)
	{
		pli = parsed_lines->data;
		parsed_lines = g_list_remove(parsed_lines, pli);
		list = g_list_prepend(list, pli);
	}
	g_static_mutex_unlock(&parsed_lines_lock);

	list = g_list_reverse(list);

	/* Remove files from the GList and add them to the GtkListStore */
	while (list)
	{
		pli = list->data;
		list = g_list_remove(list, pli);

		gtk_list_store_insert_with_values(files, NULL, G_MAXINT,
				GFILE, pli->file,
				NAME, pli->name,
				STATUS, pli->status,
				PRECALCULATED_CHECKSUM, pli->checksum,
				-1);
		g_free(pli->name);
		cichlid_parsed_line_info_free(pli);
	}

	return TRUE;
}

static gboolean
checksum_file_finish_load (gpointer data)
{
	if (parsed_lines)
		return TRUE;

	g_source_remove(idle_func_id);
	idle_func_id = 0;
	gdk_window_set_cursor(main_window->window,NULL);
	g_free(cursor);
	ck_gui_allow_actions(TRUE);

	return FALSE;
}

