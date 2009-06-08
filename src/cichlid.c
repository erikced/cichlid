/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid.c
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
#include <stdio.h>

#include "CichlidFile.h"
#include "cichlid.h"
#include "gui.h"
#include "checksum_file.h"
#include "verification.h"

#include "cichlid_hash.h"
#include "cichlid_hash_md5.h"


GtkListStore *files;
GtkTreeModel *files_filter;
GtkTreeModel *files_sort;
static guint filter_flags = (1 << GOOD) | (1 << BAD) | (1 << NOT_VERIFIED) | (1 << NOT_FOUND);

int hash_type = HASH_UNKNOWN;

static void clear_filelist();
void on_verify_clicked(GtkWidget *widget, gpointer user_data);

void
on_main_window_destroy(GtkWidget *widget, gpointer user_data)
{
	clear_filelist();
	gtk_main_quit();
}

void
on_file_menu_quit_activate(GtkWidget *widget, gpointer user_data)
{
	clear_filelist();
	gtk_main_quit();
}

void
on_file_menu_open_activate(GtkWidget *widget, gpointer user_data)
{
	GtkWidget *filechooser;
	GtkFileFilter *filter;
	GFile *file = NULL;

	filter = gtk_file_filter_new();
	/* TODO: Add more SHAx filetypes */
	gtk_file_filter_add_pattern(filter, "*.sfv");
	gtk_file_filter_add_pattern(filter, "*.md5");
	gtk_file_filter_add_pattern(filter, "MD5SUM");
	gtk_file_filter_add_pattern(filter, "MD5SUMS");

	filechooser = gtk_file_chooser_dialog_new ("Open File",
											   GTK_WINDOW(main_window),
											   GTK_FILE_CHOOSER_ACTION_OPEN,
											   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
											   NULL);

	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(filechooser), filter);

	if (gtk_dialog_run(GTK_DIALOG(filechooser)) == GTK_RESPONSE_ACCEPT)
		file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(filechooser));
	gtk_widget_destroy(filechooser);

	if (file)
	{
		ck_main_window_treeview_enable(FALSE);
		clear_filelist();
		ck_main_window_treeview_enable(TRUE);
		checksum_file_load(file);
		g_object_unref(file);
	}
}

void
on_verify_clicked(GtkWidget *widget, gpointer user_data)
{
	cichlid_verification_start();
}

/**
 * Changes the filter parameter and updates the filter
 */
void
on_filter_changed(GtkWidget *cb_filter, gpointer user_data)
{
	int active;
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(cb_filter));
	switch (active)
	{
		case 0:
			filter_flags = (1 << GOOD) | (1 << BAD) | (1 << NOT_VERIFIED) | (1 << NOT_FOUND);
			break;
		case 1:
			filter_flags = (1 << GOOD);
			break;
		case 2:
			filter_flags = (1 << BAD);
			break;
		case 3:
			filter_flags = (1 << NOT_FOUND);
			break;
	}
	ck_main_window_treeview_enable(FALSE);
	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(files_filter));
	ck_main_window_treeview_enable(TRUE);
}
/**
 * Adds the file to the file list
 * @param the path to the file (relative or absolute) as a NULL-terminated string.
 * @param a pointer to a precalculated checksum from the SFV/MD5/SHA1 file.
 */
void
add_file_to_list(const char *filename, const char* base_path, gconstpointer precalculated_checksum)
{
	GFile *file;
	GFileInfo *info;
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

	/* Add to the liststore */
	gtk_list_store_insert_with_values(files, NULL, G_MAXINT,
									  GFILE, file,
									  NAME, name,
									  STATUS, status,
									  PRECALCULATED_CHECKSUM, precalculated_checksum,
									  -1);

	if (info != NULL)
		g_object_unref(info);
	g_free(file_path);
}

static void
clear_filelist()
{
	gpointer checksum;
	GFile *file;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(files), &iter))
	{
		do
		{
			gtk_tree_model_get(GTK_TREE_MODEL(files), &iter, GFILE, &file, PRECALCULATED_CHECKSUM, &checksum, -1);
			g_object_unref(file);
			if (checksum != NULL)
				g_free(checksum);
		}
		while (gtk_list_store_remove(files,&iter));
	}
	gtk_list_store_clear(files);
}

void
render_status_text(GtkTreeViewColumn *column, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
	int status;
	char *status_text;
	gtk_tree_model_get(model, iter, STATUS, &status, -1);

	switch (status)
	{
		case GOOD:
			status_text = "Ok";
			break;
		case BAD:
			status_text = "Corrupt";
			break;
		case NOT_VERIFIED:
			status_text = "Not verified";
			break;
		case NOT_FOUND:
			status_text = "Missing";
			break;
		default:
			status_text = "";
	}

	g_object_set(renderer, "text", status_text, NULL);
}

/**
 * Filtering function for filtering out specific results
 */
gboolean
filelist_filter_func(GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
	gboolean visible;

	if (filter_flags == ((1 << GOOD) | (1 << BAD) | (1 << NOT_VERIFIED) | (1 << NOT_FOUND)))
		visible = TRUE;
	else
	{
		int status;
		gtk_tree_model_get(model, iter, STATUS, &status, -1);

		if ((1 << status) & filter_flags)
			visible = TRUE;
		else
			visible = FALSE;
	}

	return visible;
}

/**
 * Sort function for sorting the file list by name
 */
int
filelist_name_sort_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
	char *filename_a, *filename_b, *filename_cmp_a, *filename_cmp_b;
	int result;

	/* Read filenames */
	gtk_tree_model_get (model, a,
						NAME, &filename_a,
						-1);
	gtk_tree_model_get (model, b,
						NAME, &filename_b,
						-1);

	/* Convert into something which can be compared */
	filename_cmp_a = g_utf8_casefold(filename_a,-1);
	filename_cmp_b = g_utf8_casefold(filename_b,-1);

	/* Compare */
	result = g_utf8_collate(filename_cmp_a, filename_cmp_b);

	g_free(filename_a);
	g_free(filename_b);
	g_free(filename_cmp_a);
	g_free(filename_cmp_b);

	return result;
}

/**
 * Sort function for sorting the file list by status
 */
int
filelist_status_sort_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
	int status_a, status_b;

	/* Read status codes */
	gtk_tree_model_get (model, a,
						STATUS, &status_a,
						-1);

	gtk_tree_model_get (model, b,
						STATUS, &status_b,
						-1);

	return (status_a - status_b);
}

static void
init_filelist()
{
	/* Create and configure the ListStore which holds all files */
	files = gtk_list_store_new(N_COLUMNS,
							   G_TYPE_POINTER, /* GFile */
							   G_TYPE_STRING,  /* Filename */
							   G_TYPE_INT,	   /* Status */
							   G_TYPE_POINTER);/* Precalculated checksum */

	files_filter = gtk_tree_model_filter_new(GTK_TREE_MODEL(files), NULL);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(files_filter), filelist_filter_func, NULL, NULL);

	/* files_sort = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(files_filter));
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(files_sort), NAME, filelist_name_sort_func, NULL, NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(files_sort), STATUS, filelist_status_sort_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(files_sort),NAME,GTK_SORT_ASCENDING); */
}

int
main(int argc, char **argv)
{
	char *filename = NULL;
	GError *error = NULL;

	if (argc > 1)
		filename = argv[1];

	g_thread_init(NULL);
	gtk_init(&argc,&argv);

	init_filelist();

	/* Read and parse the file with checksums if possible */
	if (filename != NULL)
		g_idle_add((GSourceFunc)checksum_file_load_init,filename);

	/* Build the UI */
	ck_main_window_new(GTK_TREE_MODEL(files_filter),&error);
	if (error == NULL)
	{
		gtk_widget_show(main_window);
		gtk_main();
	}
	else
	{
		fprintf(stderr," %s\n",error->message);
		g_error_free(error);
	}

	return 0;
}
