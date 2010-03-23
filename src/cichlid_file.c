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
#include <gio/gio.h>
#include <stdint.h>

#include "cichlid_file.h"

typedef struct _CichlidFilePrivate CichlidFilePrivate;

struct _CichlidFilePrivate
{
	GFile *file;
	char  *hash;
	char  *name;
	cichlid_file_status_t  status;
};

/* Properties */
enum
{
	P_FILE = 1,
	P_HASH_STRING,
	P_FILENAME,
	P_STATUS_STRING,
	P_STATUS
};

G_DEFINE_TYPE(CichlidFile, cichlid_file, G_TYPE_OBJECT)

static void cichlid_file_get_property(GObject *object,
											   guint property_id,
											   GValue *value,
											   GParamSpec *pspec);
static void cichlid_file_set_property(GObject *object,
									  guint property_id,
									  const GValue *value,
									  GParamSpec *pspec);
static void cichlid_file_set_file(CichlidFile *self, GFile *file);


static void
cichlid_file_dispose(GObject *gobject)
{
	CichlidFile *self = CICHLID_FILE(gobject);
	CichlidFilePrivate *priv = self->priv;

	if (priv->hash)
		g_free(priv->hash);

	if (priv->name)
		g_free(priv->name);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(cichlid_file_parent_class)->dispose(gobject);
}

static void
cichlid_file_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(cichlid_file_parent_class)->finalize(gobject);
}


static void
cichlid_file_class_init(CichlidFileClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(CichlidFilePrivate));
	
	gobject_class->dispose = cichlid_file_dispose;
	gobject_class->finalize = cichlid_file_finalize;

	gobject_class->get_property = cichlid_file_get_property;
	gobject_class->set_property = cichlid_file_set_property;

	g_object_class_install_property(gobject_class, P_FILE,
									g_param_spec_object("file", NULL, "GFile",
														G_TYPE_FILE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	
	g_object_class_install_property(gobject_class, P_HASH_STRING,
									g_param_spec_string("hash-string", NULL, "Hash String",
														NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property(gobject_class, P_FILENAME,
									g_param_spec_string("filename", NULL, "Filename",
														NULL, G_PARAM_READABLE));

	g_object_class_install_property(gobject_class, P_STATUS_STRING,
									g_param_spec_string("status-string", NULL, "Status string",
														NULL, G_PARAM_READABLE));

	g_object_class_install_property(gobject_class, P_STATUS,
									g_param_spec_int("status", NULL, "Status",
													 0, N_STATUS-1, 0, G_PARAM_READWRITE));
}

static void
cichlid_file_init(CichlidFile *self)
{
	CichlidFilePrivate *priv = self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self, CICHLID_TYPE_FILE, CichlidFilePrivate);

	/* Initiera variabler */
	priv->file = NULL;
	priv->name = NULL;
	priv->hash = NULL;
	priv->status = STATUS_NOT_VERIFIED;
}

CichlidFile *
cichlid_file_new(GFile *file, char *hash)
{	
	return g_object_new(CICHLID_TYPE_FILE, "file", file, "hash-string", hash, NULL);
}

static void
cichlid_file_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	CichlidFile        *self = CICHLID_FILE(object);
	CichlidFilePrivate *priv = self->priv;

	switch(property_id)
	{
	case P_FILE:
		g_value_set_object(value, priv->file);
		break;
	case P_HASH_STRING:
		g_value_set_string(value, cichlid_file_get_hash_string(self));
		break;
	case P_FILENAME:
		g_value_set_string(value, NULL);
		break;
	case P_STATUS_STRING:
		g_value_set_static_string(value, cichlid_file_get_status_string(self));
		break;
	case P_STATUS:
		g_value_set_int(value, priv->status);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

GFile *
cichlid_file_get_file(CichlidFile *self)
{
	g_return_val_if_fail(CICHLID_IS_FILE(self), NULL);
	CichlidFilePrivate *priv = self->priv;

	g_assert(priv->file != NULL);
	return priv->file;	
}

const char *
cichlid_file_get_filename(CichlidFile *self)
{
	g_return_val_if_fail(CICHLID_IS_FILE(self), NULL);
	CichlidFilePrivate *priv = self->priv;

	g_assert(priv->name != NULL);	
	return priv->name;
}

const char *
cichlid_file_get_hash_string(CichlidFile *self)
{
	g_return_val_if_fail(CICHLID_IS_FILE(self), NULL);
	CichlidFilePrivate *priv = self->priv;

	g_assert(priv->hash != NULL);
	return priv->hash;
}

cichlid_file_status_t
cichlid_file_get_status(CichlidFile *self)
{
	g_return_val_if_fail(CICHLID_IS_FILE(self), 0);
	CichlidFilePrivate *priv = self->priv;
	
	return priv->status;
}

const char *
cichlid_file_get_status_string(CichlidFile *self)
{
	g_return_val_if_fail(CICHLID_IS_FILE(self), NULL);

	CichlidFilePrivate *priv = self->priv;
	const char *status;
	
	switch (priv->status)
	{
	case STATUS_GOOD:
		status = _("Ok");
		break;
	case STATUS_BAD:
		status = _("Corrupt");
		break;
	case STATUS_NOT_VERIFIED:
		status = _("Not verified");
		break;
	case STATUS_NOT_FOUND:
		status = _("Missing");
		break;
	default:
		g_assert_not_reached();
	}

	return status;
}

static void
cichlid_file_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	CichlidFile        *self = CICHLID_FILE(object);
	CichlidFilePrivate *priv = self->priv;
		
	switch(property_id)
	{
	case P_FILE:
		cichlid_file_set_file(self, G_FILE(g_value_get_object(value)));
		break;
	case P_HASH_STRING:
		priv->hash = g_value_dup_string(value);
		break;
	case P_STATUS:
		cichlid_file_set_status(self, g_value_get_int(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

static void
cichlid_file_set_file(CichlidFile *self, GFile *file)
{
	g_return_if_fail(CICHLID_IS_FILE(self));
	g_return_if_fail(file != NULL);
	CichlidFilePrivate *priv = self->priv;

	priv->file = file;
	g_object_ref(file);
	
	GFileInfo *info = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, G_FILE_QUERY_INFO_NONE, NULL, NULL);
	if (info != NULL)
	{
		priv->name = g_strdup(g_file_info_get_attribute_string(info, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME));
		g_object_unref(info);
	}
	else
	{
		char *tmpname = g_file_get_basename(file);
		priv->name = g_filename_to_utf8(tmpname, -1, NULL, NULL, NULL);
		g_free(tmpname);
	}
}

void
cichlid_file_set_status(CichlidFile *self, cichlid_file_status_t status)
{
	g_return_if_fail(CICHLID_IS_FILE(self));
	if (status == 0 || status >= N_STATUS)
	{
		g_warning("Incorrect status number %i. Number must be 0 < status < %i", status, N_STATUS);
		return;
	}

	CichlidFilePrivate *priv = self->priv;
	priv->status = status;
}
