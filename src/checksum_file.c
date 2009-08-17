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



static GList       *parsed_lines = NULL;
static GStaticMutex parsed_lines_lock = G_STATIC_MUTEX_INIT;
static guint        idle_func_id = 0;
static GdkCursor    *cursor;


