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

#include "cichlid.h"
#include "cichlid_checksum_file_v.h"
#include "cichlid_file.h"
#include "cichlid_cell_renderer.h"
#include "cichlid_checksum_file.h"

#define MAIN_UI_SYSTEM PKGDATADIR"/cichlid.ui"
#define MAIN_UI_SOURCE "data/cichlid.ui"

static gboolean  created = FALSE;

static GtkWidget *main_window;
static GtkWidget *filelist;
static GtkWidget *btn_verify;
static GtkWidget *cb_filter;
static GtkWidget *file_menu_open;
static GtkWidget *file_menu_quit;
static GtkWidget *about;
static GtkWidget *progress;

static void filelist_init(GtkTreeModel *model);
static void filelist_render_status_text(GtkTreeViewColumn *column,
										GtkCellRenderer *renderer,
										GtkTreeModel *model,
										GtkTreeIter *iter,
										gpointer data);
static void on_about_activate();
static void on_verify_clicked();
static void on_main_window_destroyed();

void
ck_gui_allow_actions(gboolean allow)
{
	gtk_widget_set_sensitive(btn_verify, allow);
	gtk_widget_set_sensitive(cb_filter, allow);
	gtk_widget_set_sensitive(file_menu_open, allow);
}

GtkWidget *cichlid_main_window_get_window()
{
	if (created)
		return main_window;
	return NULL;
}

gboolean
cichlid_main_window_new(GtkTreeModel *model,
						GError **error)
{
	GtkBuilder *WindowBuilder;

	if (created)
	{
		g_set_error(error, g_quark_from_static_string("cichlid-error"), CICHLID_ERROR_MAIN_WINDOW_EXISTS,
					"A Main Window is already open");
		return FALSE;
	}

	WindowBuilder = gtk_builder_new();
	if (!gtk_builder_add_from_file(WindowBuilder, MAIN_UI_SOURCE, NULL) &&
		!gtk_builder_add_from_file(WindowBuilder, MAIN_UI_SYSTEM, error))
	{
		g_object_unref(WindowBuilder);
		return FALSE;
	}
	created = TRUE;
	
	main_window = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"main_window"));
	g_signal_connect(G_OBJECT(main_window), "destroy-event", G_CALLBACK(on_main_window_destroyed), NULL);
	
	/* Progress Bar */
	progress = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"pb_progress"));
	
	/* File list */
	filelist = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"tv_files"));
	filelist_init(model);
	
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


void
cichlid_main_window_show_filelist(gboolean enable)
{
	static GtkTreeModel *old_model = NULL;
	if (enable)
		gtk_tree_view_set_model(GTK_TREE_VIEW(filelist), old_model);
	else
	{
		old_model = gtk_tree_view_get_model(GTK_TREE_VIEW(filelist));
		gtk_tree_view_set_model(GTK_TREE_VIEW(filelist),NULL);
	}
}

static void
filelist_init(GtkTreeModel *model)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	/* Filename column */
	//renderer = cichlid_cell_renderer_new();
	//column = gtk_tree_view_column_new_with_attributes(C_("Computer file","File"), renderer, "file", CICHLID_CHECKSUM_FILE_FILE,  NULL);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(C_("Computer file","File"), renderer,
													  "text", CICHLID_CHECKSUM_FILE_FILENAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(filelist), column);
	
	/* Status column */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Status"), renderer, NULL);
	gtk_tree_view_column_set_cell_data_func(column, renderer, filelist_render_status_text, NULL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(filelist), column);
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(filelist), model);
}

static void
filelist_render_status_text(GtkTreeViewColumn *column,
							GtkCellRenderer *renderer,
							GtkTreeModel *model,
							GtkTreeIter *iter,
							gpointer data)
{
	GValue value = { 0, };
	CichlidFile *ck_file;
	char *status;

	gtk_tree_model_get_value(model, iter, CICHLID_CHECKSUM_FILE_FILE, &value);
	ck_file = g_value_get_object(&value);
	g_value_unset(&value);
	
	status = cichlid_file_get_status_string(ck_file);
	g_object_set(renderer, "text", status, NULL);
}

static void
on_about_activate()
{
	gtk_show_about_dialog(GTK_WINDOW(main_window),
	                      "program-name", "cichlid",
	                      "title", _("About cichlid"),
	                      "copyright", "Copyright \xc2\xa9 2009 Erik Cederberg",
	                      NULL);
}

static void
on_main_window_destroyed()
{
	created = FALSE;
}


static void
on_verification_progress_updated(double _progress)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), _progress);
}

static void
on_verification_complete()
{
	g_object_set(progress, "visible", FALSE, NULL);
}

static void
on_verify_clicked()
{
	CichlidChecksumFile *cfile = CICHLID_CHECKSUM_FILE(gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(gtk_tree_view_get_model(GTK_TREE_VIEW(filelist)))));
	g_signal_connect(G_OBJECT(cfile), "verification-progress-update",
					 G_CALLBACK(on_verification_progress_updated),NULL);
	g_object_set(progress, "visible", TRUE, NULL);
	cichlid_checksum_file_verify(cfile);
}
