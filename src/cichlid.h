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

#include <gtk/gtk.h>

/* Error Definitions */
enum
{
	CIHLID_ERROR = 0,
	CICHLID_ERROR_BADCF,
	CICHLID_ERROR_UNKNOWN_HASH,
	CICHLID_ERROR_MAIN_WINDOW_EXISTS
};

extern int hash_type;
extern GtkListStore *files;
extern GtkTreeModel *files_filter;
extern GtkTreeModel *files_sort;

void filelist_change_filter(int option);

#endif /* CICHLID_H */

