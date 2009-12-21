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

#include "cichlid_file.h"

typedef struct _CichlidFilePrivate CichlidFilePrivate;

struct _CichlidFilePrivate
{
	char             *hash;
	char             *name;
	cichlid_file_status_t  status;
	char             *status_string;
};

/* Properties */
enum
{
	P_HASH_STRING = 1,
	P_FILENAME,
	P_STATUS_STRING,
	P_STATUS
};

G_DEFINE_TYPE(CichlidFile, cichlid_file, G_TYPE_OBJECT)

void        cichlid_file_set_status(CichlidFile *self, cichlid_file_status_t status);
static void cichlid_file_get_property(GObject *object,
											   guint property_id,
											   GValue *value,
											   GParamSpec *pspec);
static void cichlid_file_set_property(GObject *object,
											   guint property_id,
											   const GValue *value,
											   GParamSpec *pspec);


static void
cichlid_file_dispose(GObject *gobject)
{
	CichlidFile *self = CICHLID_FILE(gobject);
	CichlidFilePrivate *priv = self->priv;

	if (priv->name)
		g_free(priv->name);

	if (priv->hash)
		g_free(priv->hash);

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

	g_type_class_add_private(klass, sizeof(CichlidFilePrivate));
	
	gobject_class->dispose = cichlid_file_dispose;
	gobject_class->finalize = cichlid_file_finalize;

	gobject_class->get_property = cichlid_file_get_property;
	gobject_class->set_property = cichlid_file_set_property;

	g_object_class_install_property(gobject_class, P_HASH_STRING,
									g_param_spec_string("hash-string", NULL, "Hash String",
														NULL, G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property(gobject_class, P_FILENAME,
									g_param_spec_string("filename", NULL, "Filename",
														NULL, G_PARAM_READABLE));

	g_object_class_install_property(gobject_class, P_STATUS_STRING,
									g_param_spec_string("status-string", NULL, "Status string",
														NULL, G_PARAM_READABLE));

	g_object_class_install_property(gobject_class, P_STATUS,
									g_param_spec_string("status", NULL, "Status",
														NULL, G_PARAM_READWRITE));
}

static void
cichlid_file_init(CichlidFile *self)
{
	CichlidFilePrivate *priv = self->priv;
	/* Initiera variabler */
	priv->name = NULL;
	priv->hash = NULL;
	priv->status = STATUS_NOT_VERIFIED;
	priv->status_string = NULL;
}

CichlidFile *
cichlid_file_new()
{
	CichlidFile *obj;
	obj = g_object_new(CICHLID_TYPE_FILE, NULL);

	return obj;
}

static void
cichlid_file_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	CichlidFile        *self = CICHLID_FILE(object);
	CichlidFilePrivate *priv = self->priv;

	switch(property_id)
	{
	case P_HASH_STRING:
		g_value_set_string(value, priv->hash);
		break;
	case P_FILENAME:
		g_value_set_string(value, NULL);
		break;
	case P_STATUS_STRING:
		g_value_set_string(value, priv->status_string);
		break;
	case P_STATUS:
		g_value_set_int(value, priv->status);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

static void
cichlid_file_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	CichlidFile        *self = CICHLID_FILE(object);
	CichlidFilePrivate *priv = self->priv;

	switch(property_id)
	{
	case P_STATUS:
		cichlid_file_set_status(self, g_value_get_int(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

cichlid_file_status_t
cichlid_file_get_status(CichlidFile *self)
{
	g_return_val_if_fail(CICHLID_IS_FILE(self), 0);;
	CichlidFilePrivate *priv = self->priv;

	return priv->status;
}

void
cichlid_file_set_status(CichlidFile *self, cichlid_file_status_t status)
{
	g_return_if_fail(CICHLID_IS_FILE(self));
	if (status == 0 || status >= N_STATUS)
	{
		g_warning("Incorrect status number %i. Number must be 0 < status < %i",status,N_STATUS);
		return;
	}

	CichlidFilePrivate *priv = self->priv;
	priv->status = status;

	if (priv->status_string)
		g_free(priv->status_string);
	
	switch (status)
	{
		case STATUS_GOOD:
			priv->status_string = _("Ok");
			break;
		case STATUS_BAD:
			priv->status_string = _("Corrupt");
			break;
		case STATUS_NOT_VERIFIED:
			priv->status_string = _("Not verified");
			break;
		case STATUS_NOT_FOUND:
			priv->status_string = _("Missing");
			break;
		default:
			g_assert_not_reached();
	}
}

const char *
cichlid_file_get_status_string(CichlidFile *self)
{
	g_return_val_if_fail(CICHLID_IS_FILE(self), NULL);
	CichlidFilePrivate *priv = self->priv;

	return (const char *)priv->status_string;
}
