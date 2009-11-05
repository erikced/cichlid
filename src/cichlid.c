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

#include "cichlid.h"
#include "cichlid_main_window.h"
#include "cichlid_file.h"
#include "cichlid_checksum_file.h"

enum
{
	GFILE = 0,
	NAME,
	STATUS,
	PRECALCULATED_CHECKSUM,
	N_COLUMNS
};

GtkTreeModel *files_sort;
CichlidChecksumFile *cfile;

int hash_type = HASH_UNKNOWN;

void
on_main_window_destroy(GtkWidget *widget, gpointer user_data)
{
	gtk_main_quit();
}

/*
 * Filtering function for filtering out specific results
 */

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

int
main(int argc, char **argv)
{
	char *filename = NULL;
	GError *error = NULL;

	if (argc > 1)
		filename = argv[1];

	g_thread_init(NULL);
	gtk_init(&argc,&argv);

	/* Initialize the CichlidChecksumFile and the Filter */
	cfile = g_object_new(CICHLID_TYPE_CHECKSUM_FILE, NULL);

	/* Read and parse the file with checksums if possible */
	if (filename != NULL)
		cichlid_checksum_file_load_from_cmd(cfile, filename);

	/* Build the UI */
	cichlid_main_window_new(GTK_TREE_MODEL(cfile), &error);
	if (error == NULL)
	{
		gtk_widget_show(cichlid_main_window_get_window());
		gtk_main();
	}
	else
	{
		fprintf(stderr," %s\n", error->message);
		g_error_free(error);
	}

	g_object_unref(cfile);

	return 0;
}
