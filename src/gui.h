/*
 * Copyright © 2008 Erik Cederberg <erikced@gmail.com>
 *
 * crcchk - gui.h
 *
 * crcchk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * crcchk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with crcchk.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GUI_H
#define GUI_H

extern GtkWidget *main_window;
extern GtkWidget *filelist;

void ck_gui_allow_actions (gboolean allow);
gboolean ck_main_window_new(GtkTreeModel *model, GError **error);
void ck_main_window_treeview_enable (gboolean enable);
void ck_progress_dialog_new ();
void ck_progress_dialog_delete ();
void ck_progress_dialog_update (GMutex *lock,
							    const gchar *current_file,
							    const guint *current_file_num,
							    const guint *total_file_num,
							    const gint *verified_file_size,
							    const gint *total_file_size);

#endif /* GUI_H */

