/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - gui.c
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
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtk/gtkaboutdialog.h>

#include "gui.h"
#include "cichlid.h"
#include "cichlid_checksum_file.h"
#include "verification.h"

#define MAIN_UI_SYSTEM PKGDATADIR"/cichlid.ui"
#define MAIN_UI_SOURCE "data/cichlid.ui"
#define PROGRESS_DIALOG_UI_SYSTEM PKGDATADIR"/cichlid_progress_dialog.ui"
#define PROGRESS_DIALOG_UI_SOURCE "data/cichlid_progress_dialog.ui"

/* Main Window */
GtkWidget *main_window = NULL;
GtkWidget *filelist;
static GtkWidget *btn_verify;
static GtkWidget *cb_filter;
static GtkWidget *file_menu_open;
static GtkWidget *file_menu_quit;
static GtkWidget *about;

static void ck_main_window_treeview_init(GtkTreeModel *model);
static void on_about_activate();

void
ck_gui_allow_actions(gboolean allow)
{
	gtk_widget_set_sensitive(btn_verify, allow);
	gtk_widget_set_sensitive(cb_filter, allow);
	gtk_widget_set_sensitive(file_menu_open, allow);
}

gboolean
ck_main_window_new(GtkTreeModel *model, GError **error)
{
	GtkBuilder *WindowBuilder;

	WindowBuilder = gtk_builder_new();
	if (!gtk_builder_add_from_file(WindowBuilder,MAIN_UI_SOURCE,NULL) &&
		!gtk_builder_add_from_file(WindowBuilder,MAIN_UI_SYSTEM,error))
	{
		g_object_unref(WindowBuilder);
		return FALSE;
	}

	main_window = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"main_window"));

	/* File list */
	filelist = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"tv_files"));
	ck_main_window_treeview_init(model);

	/* File filter combobox */
	cb_filter = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"cb_filter"));
	g_signal_connect(G_OBJECT(cb_filter), "changed", G_CALLBACK(on_filter_changed), NULL);


	/* Verify button */
	btn_verify = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"btn_verify"));
	g_signal_connect(G_OBJECT(btn_verify), "clicked", G_CALLBACK(on_verify_clicked), NULL);

	/* Menu */
	file_menu_open = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"file_menu_open"));
	g_signal_connect(G_OBJECT(file_menu_open),"activate", G_CALLBACK(on_file_menu_open_activate),NULL);
	file_menu_quit = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"file_menu_quit"));
	g_signal_connect(G_OBJECT(file_menu_quit),"activate", G_CALLBACK(on_file_menu_quit_activate),NULL);
	about = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"about"));
	g_signal_connect(G_OBJECT(about),"activate", G_CALLBACK(on_about_activate),NULL);


	gtk_builder_connect_signals(WindowBuilder, NULL);
	g_object_unref(WindowBuilder);

	return TRUE;
}

/**
 * Initializes the tree view listing the files, uses the 'files' ListStore as a model
 * @param a tree view to be initialized
 */
static void
ck_main_window_treeview_init(GtkTreeModel *model)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	/* Filename column */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(C_("Computer file","File"),renderer, "text", CICHLID_CHECKSUM_FILE_FILENAME, NULL);
	//gtk_tree_view_column_set_sort_column_id(column, NAME);
	gtk_tree_view_append_column(GTK_TREE_VIEW(filelist), column);

	/* Status column */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Status"),renderer, "text", CICHLID_CHECKSUM_FILE_STATUS, NULL);
	gtk_tree_view_column_set_cell_data_func(column, renderer, render_status_text, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id(column, STATUS);
	gtk_tree_view_append_column(GTK_TREE_VIEW(filelist), column);

	gtk_tree_view_set_model(GTK_TREE_VIEW(filelist), model);
}

void
ck_main_window_treeview_enable(gboolean enable)
{
	static GtkTreeModel *old_model = NULL;
	if (enable)
		gtk_tree_view_set_model(GTK_TREE_VIEW(filelist),old_model);
	else
	{
		old_model = gtk_tree_view_get_model(GTK_TREE_VIEW(filelist));
		gtk_tree_view_set_model(GTK_TREE_VIEW(filelist),NULL);
	}
}

/* Progress Dialog */
static gboolean			progress_dialog_open = FALSE;
static GtkDialog		*progress_dialog = NULL;
static GtkProgressBar 	*progress_dialog_progress_bar = NULL;
static GtkLabel 		*progress_dialog_file_label = NULL;
static GtkButton		*progress_dialog_abort;

void
ck_progress_dialog_new(volatile int *abort)
{
	GtkBuilder *WindowBuilder;

	if (progress_dialog_open)
		return;
	progress_dialog_open = TRUE;

	gtk_widget_set_sensitive (btn_verify, FALSE);
	gtk_widget_set_sensitive(file_menu_open, FALSE);

	WindowBuilder = gtk_builder_new();
	if (!gtk_builder_add_from_file(WindowBuilder,PROGRESS_DIALOG_UI_SOURCE,NULL) &&
		!gtk_builder_add_from_file(WindowBuilder,PROGRESS_DIALOG_UI_SYSTEM,NULL))
	{
		g_object_unref(WindowBuilder);
		return;
	}

	progress_dialog = GTK_DIALOG(gtk_builder_get_object(WindowBuilder,"progress_dialog"));
	g_signal_connect(G_OBJECT(progress_dialog), "close", G_CALLBACK(cichlid_verification_cancel), NULL);
	progress_dialog_progress_bar = GTK_PROGRESS_BAR(gtk_builder_get_object(WindowBuilder,"progressbar"));
	progress_dialog_file_label = GTK_LABEL(gtk_builder_get_object(WindowBuilder,"current_file"));
	progress_dialog_abort = GTK_BUTTON(gtk_builder_get_object(WindowBuilder,"btn_cancel"));
	g_signal_connect(G_OBJECT(progress_dialog_abort), "clicked", G_CALLBACK(cichlid_verification_cancel), NULL);
	g_object_unref(WindowBuilder);

	gtk_widget_show_all(GTK_WIDGET(progress_dialog));
}

void
ck_progress_dialog_delete()
{
	g_assert(progress_dialog != NULL);

	if (GTK_IS_WIDGET(progress_dialog))
	{
		gtk_widget_hide_all(GTK_WIDGET(progress_dialog));
		gtk_widget_destroy(GTK_WIDGET(progress_dialog));
	}

	progress_dialog = NULL;
	progress_dialog_progress_bar = NULL;
	progress_dialog_file_label = NULL;
	progress_dialog_abort = NULL;

	progress_dialog_open = FALSE;

	gtk_widget_set_sensitive(btn_verify, TRUE);
	gtk_widget_set_sensitive(file_menu_open, TRUE);
}

void
ck_progress_dialog_update(GMutex *lock,
						  const char *current_file,
						  const guint *current_file_num,
						  const guint *total_file_num,
						  const int *verified_file_size,
						  const int *total_file_size)
{
	if (!GTK_IS_WIDGET(progress_dialog))
		return;

	char *verification_message, *progressbar_message;
	double verification_progress;

	/* Prepare the messages */
	verification_progress =	(*verified_file_size / (double)*total_file_size);
	if (verification_progress > 1)
		verification_progress = 1;
	g_mutex_lock(lock);
	progressbar_message = g_strdup_printf(_("Verifying file %u of %u"), *current_file_num, *total_file_num);
	verification_message = g_strdup_printf(_("<i>Verifying %s</i>"), current_file);
	g_mutex_unlock(lock);

	/* Update the progress bar and the text below it */
	gtk_progress_bar_set_text(progress_dialog_progress_bar,progressbar_message);
	gtk_progress_bar_set_fraction(progress_dialog_progress_bar, verification_progress);
	gtk_label_set_markup(progress_dialog_file_label, verification_message);

	/* Clean up */
	g_free(progressbar_message);
	g_free(verification_message);
}

/* About Dialog */
static void
on_about_activate()
{
	gtk_show_about_dialog(GTK_WINDOW(main_window),
	                      "program-name", "cichlid",
	                      "title", _("About cichlid"),
	                      "copyright", "Copyright \xc2\xa9 2009 Erik Cederberg",
	                      NULL);

}

