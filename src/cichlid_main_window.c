/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_main_window.c
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

#include "cichlid_hash.h"
#include "cichlid_hash_sha256.h"

#define MAIN_UI_SYSTEM PKGDATADIR"/cichlid.ui"
#define MAIN_UI_WIN32 "cichlid.ui"
#define MAIN_UI_SOURCE "data/cichlid.ui"


enum
{
	B_ALL = 0,
	B_OK,
	B_CORRUPT,
	B_MISSING,
	N_BTNS
};			

static gboolean   created = FALSE;

static GtkWidget *main_window;
static GtkWidget *filelist;
static GtkWidget *btn_verify;
static GtkWidget *btn_filter[N_BTNS];
static GtkWidget *cb_filter;
static GtkWidget *file_menu_open;
static GtkWidget *file_menu_quit;
static GtkWidget *about;
static GtkWidget *progress;
static GtkWidget *progress_lbl;

static void filelist_init(GtkTreeModel *model);
static void filelist_render_status_text(GtkTreeViewColumn *column,
										GtkCellRenderer *renderer,
										GtkTreeModel *model,
										GtkTreeIter *iter,
										gpointer data);
static CichlidChecksumFile *get_checksum_file();
static void on_about_activate();
static void on_checksum_file_loaded();
static void on_file_menu_quit_activate(GtkWidget *widget,
									   gpointer user_data);
static void on_file_menu_open_activate(GtkWidget *widget,
									   gpointer user_data);
static void on_filter_changed(GtkWidget *btn);
static void on_verify_clicked();
static void on_verification_complete();
static void on_verification_progress_updated();
static void on_main_window_destroyed();

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
		!gtk_builder_add_from_file(WindowBuilder, MAIN_UI_WIN32, NULL) &&
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

	/* Progress label */
	progress_lbl = GTK_WIDGET(gtk_builder_get_object(WindowBuilder, "lbl_progress"));
	
	/* File list */
	filelist = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"tv_files"));
	filelist_init(model);
	g_signal_connect(G_OBJECT(get_checksum_file()), "file-loaded",
					 G_CALLBACK(on_checksum_file_loaded), NULL);
	g_signal_connect(G_OBJECT(get_checksum_file()), "verification-progress-update",
					 G_CALLBACK(on_verification_progress_updated),NULL);
	g_signal_connect(G_OBJECT(get_checksum_file()), "verification-complete",
					 G_CALLBACK(on_verification_complete), NULL);

	
	/* File filter combobox */
	//cb_filter = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"cb_filter"));
	btn_filter[B_ALL] = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"btn_all"));
	g_signal_connect(G_OBJECT(btn_filter[B_ALL]), "toggled", G_CALLBACK(on_filter_changed), NULL);
	btn_filter[B_OK] = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"btn_good"));
	g_signal_connect(G_OBJECT(btn_filter[B_OK]), "toggled", G_CALLBACK(on_filter_changed), NULL);
	btn_filter[B_CORRUPT] = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"btn_corrupt"));
	g_signal_connect(G_OBJECT(btn_filter[B_CORRUPT]), "toggled", G_CALLBACK(on_filter_changed), NULL);
	btn_filter[B_MISSING] = GTK_WIDGET(gtk_builder_get_object(WindowBuilder,"btn_missing"));
	g_signal_connect(G_OBJECT(btn_filter[B_MISSING]), "toggled", G_CALLBACK(on_filter_changed), NULL);
	
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
													  "text", CICHLID_CHECKSUM_FILE_FILENAME,
													  NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
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
	if (ck_file->status == STATUS_BAD)
		g_object_set(renderer, "text", status, "foreground", "red", NULL);
	else
		g_object_set(renderer, "text", status, "foreground", NULL, NULL);
}

static CichlidChecksumFile *
get_checksum_file()
{
	return CICHLID_CHECKSUM_FILE(gtk_tree_model_filter_get_model
								 (GTK_TREE_MODEL_FILTER(gtk_tree_view_get_model
														(GTK_TREE_VIEW(filelist)))));
}

static void
on_about_activate()
{
	gtk_show_about_dialog(GTK_WINDOW(main_window),
	                      "program-name", "Cichlid",
	                      "title", _("About Cichlid"),
	                      "copyright", "Copyright \xc2\xa9 2009 Erik Cederberg",
	                      NULL);
}

static void
on_checksum_file_loaded()
{
	gtk_widget_set_sensitive(btn_verify, TRUE);
	gtk_widget_set_sensitive(file_menu_open, TRUE);

	gdk_window_set_cursor(main_window->window, NULL);
}

void
on_file_menu_quit_activate(GtkWidget *widget, gpointer user_data)
{
	gtk_main_quit();
}

void
on_file_menu_open_activate(GtkWidget *widget, gpointer user_data)
{
	GdkCursor *cursor;
	GtkWidget *filechooser;
	GtkFileFilter *filter;
	GFile *file = NULL;

	filter = gtk_file_filter_new();
	/* TODO: Add more SHAx filetypes */
	gtk_file_filter_add_pattern(filter, "*.sfv");
	gtk_file_filter_add_pattern(filter, "*.md5");
	gtk_file_filter_add_pattern(filter, "MD5SUM");
	gtk_file_filter_add_pattern(filter, "MD5SUMS");
	gtk_file_filter_add_pattern(filter, "*.sha256");
	gtk_file_filter_add_pattern(filter, "SHA256SUM");
	gtk_file_filter_add_pattern(filter, "SHA256SUMS");

	filechooser = gtk_file_chooser_dialog_new ("Open File",
											   GTK_WINDOW(cichlid_main_window_get_window()),
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
		gchar *filename;
		
		cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_WATCH);
		gdk_window_set_cursor(main_window->window, cursor);
		gtk_widget_set_sensitive(btn_verify, FALSE);
		gtk_widget_set_sensitive(file_menu_open, FALSE);

		cichlid_checksum_file_load(get_checksum_file(), file);
		filename = cichlid_checksum_file_get_filename(get_checksum_file());
		if (filename)
		{
			gtk_window_set_title(GTK_WINDOW(main_window), filename);
			g_free(filename);
		}
		else
			gtk_window_set_title(GTK_WINDOW(main_window), "cichlid");

		gdk_cursor_unref(cursor);
		g_object_unref(file);
	}
}

static void
on_filter_changed(GtkWidget *btn)
{
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn)))
		return;
	
	int selected = -1;
	for (int i = 0; i < N_BTNS; ++i)
	{
		if (btn == btn_filter[i])
			selected = i;
		else
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(btn_filter[i]), FALSE);
	}

	filelist_change_filter(selected);
}

static void
on_main_window_destroyed()
{
	created = FALSE;
}

static void
on_verification_progress_updated(double _progress, double speed)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), _progress);	
 	char *txt = g_strdup_printf(_("(%.1f Mb/s)"), speed);
	gtk_label_set_text(GTK_LABEL(progress_lbl), txt);
	g_free(txt);
}

static void
on_verification_complete()
{
	g_object_set(progress, "visible", FALSE, NULL);
	gtk_label_set_text(GTK_LABEL(progress_lbl), NULL);	
	gtk_widget_set_sensitive(file_menu_open, TRUE);
	gtk_widget_set_sensitive(btn_verify, TRUE);
}

static void
on_verify_clicked()
{
	CichlidChecksumFile *cfile = get_checksum_file();
	
	gtk_widget_set_sensitive(btn_verify, FALSE);
	gtk_widget_set_sensitive(file_menu_open, FALSE);

	g_object_set(progress, "visible", TRUE, "fraction", 0.0, NULL);
	cichlid_checksum_file_verify(cfile);
}
