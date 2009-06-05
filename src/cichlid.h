/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid.h
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
#ifndef CICHLID_H
#define CICHLID_H

/* Error Definitions */
enum
{
	CICHLID_ERROR_BADCF
};

enum
{
  GFILE = 0,
  NAME,
  STATUS,
  PRECALCULATED_CHECKSUM,
  N_COLUMNS
};

extern gint hash_type;
extern GtkListStore *files;
extern GtkTreeModel *files_filter;
extern GtkTreeModel *files_sort;

void add_file_to_list(const gchar*, const gchar*, gconstpointer);
void on_verify_clicked(GtkWidget *widget, gpointer user_data);
void on_file_menu_open_activate(GtkWidget *widget, gpointer user_data);
void on_file_menu_quit_activate(GtkWidget *widget, gpointer user_data);
void on_filter_changed(GtkWidget *widget, gpointer user_data);
void render_status_text(GtkTreeViewColumn *column, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data);

#endif /* CICHLID_H */

