/*
 * Copyright © 2008 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - CichlidFile.c
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
#include "CichlidFile.h"

G_DEFINE_TYPE (CichlidFile, cichlid_file, G_TYPE_OBJECT)

static void
cichlid_file_dispose (GObject *gobject)
{
	CichlidFile *self = CICHLID_FILE (gobject);

	if (self->file != NULL)
		g_object_unref(self->file);

	/* Chain up to the parent class */
	G_OBJECT_CLASS (cichlid_file_parent_class)->dispose (gobject);
}

static void
cichlid_file_finalize (GObject *gobject)
{
	CichlidFile *self = CICHLID_FILE (gobject);

	G_OBJECT_CLASS (cichlid_file_parent_class)->finalize (gobject);
}


static void
cichlid_file_class_init (CichlidFileClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->dispose = cichlid_file_dispose;
	gobject_class->finalize = cichlid_file_finalize;

	/* Initiera funktioner */
}


static void
cichlid_file_init (CichlidFile *self)
{
	/* Initiera variabler */
}

CichlidFile*
cichlid_file_new(GFile* file, gint filetype, gconstpointer checksum)
{
	g_assert(file != NULL);

	CichlidFile *_file;
	_file = g_object_new(CICHLID_TYPE_FILE,NULL);
	_file->file = file;
	g_object_ref(file);
	_file->filetype = filetype;
	//file->checksum = checksum;

	return _file;
}

