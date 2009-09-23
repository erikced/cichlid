/*
 * Copyright © 2008 Erik Cederberg <erikced@gmail.com>
 *
 * crcchk - cichlid_main_window.h
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
#ifndef CICHLID_MAIN_WINDOW_H
#define CICHLID_MAIN_WINDOW_H

GtkWidget *cichlid_main_window_get_window();
gboolean   cichlid_main_window_new(GtkTreeModel *model, GError **error);
void       cichlid_main_window_show_filelist(gboolean enable);
#endif /* GUI_H */

