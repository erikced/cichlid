/*
 * Copyright © 2008-2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_file.c
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
#include <glib-object.h>
#include <glib/gi18n.h>

#include "cichlid_file.h"

G_DEFINE_TYPE(CichlidFile, cichlid_file, G_TYPE_OBJECT);

static void
cichlid_file_dispose(GObject *gobject)
{
	CichlidFile *self = CICHLID_FILE(gobject);

	g_object_unref(self->file);

	if (self->name)
		g_free(self->name);

	if (self->checksum)
		g_free(self->checksum);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(cichlid_file_parent_class)->dispose(gobject);
}

static void
cichlid_file_finalize(GObject *gobject)
{
	CichlidFile *self = CICHLID_FILE(gobject);

	G_OBJECT_CLASS(cichlid_file_parent_class)->finalize(gobject);
}


static void
cichlid_file_class_init(CichlidFileClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = cichlid_file_dispose;
	gobject_class->finalize = cichlid_file_finalize;
}

static void
cichlid_file_init(CichlidFile *self)
{
	/* Initiera variabler */
	self->file = NULL;
	self->name = NULL;
	self->status = STATUS_NOT_VERIFIED;
	self->checksum = NULL;
}

CichlidFile *
cichlid_file_new()
{
	CichlidFile *obj;
	obj = g_object_new(CICHLID_TYPE_FILE, NULL);

	return obj;
}

char *
cichlid_file_get_status_string(CichlidFile *self)
{
	g_return_val_if_fail(CICHLID_IS_FILE(self), NULL);
	char *status_text;

	switch (self->status)
	{
		case STATUS_GOOD:
			status_text = _("Ok");
			break;
		case STATUS_BAD:
			status_text = _("Corrupt");
			break;
		case STATUS_NOT_VERIFIED:
			status_text = _("Not verified");
			break;
		case STATUS_NOT_FOUND:
			status_text = _("Missing");
			break;
		default:
			g_assert_not_reached();
	}

	return status_text;
}
