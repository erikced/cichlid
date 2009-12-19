/*
 * Copyright © 2008-2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_filter.c
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

#include "cichlid_filter.h"

G_DEFINE_TYPE(CichlidTreeModelFilter, cichlid_filter, GTK_TYPE_TREE_MODEL_FILTER);

static void
cichlid_filter_dispose(GObject *gobject)
{
	CichlidTreeModelFilter *self = CICHLID_FILTER(gobject);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(cichlid_filter_parent_class)->dispose(gobject);
}

static void
cichlid_filter_finalize(GObject *gobject)
{
	CichlidTreeModelFilter *self = CICHLID_FILTER(gobject);

	G_OBJECT_CLASS(cichlid_filter_parent_class)->finalize(gobject);
}


static void
cichlid_filter_class_init(CichlidTreeModelFilterClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = cichlid_filter_dispose;
	gobject_class->finalize = cichlid_filter_finalize;
}

static void
cichlid_filter_init(CichlidTreeModelFilter *self)
{
	/* Initiera variabler */
	priv->filter = NULL;
	priv->filter_string = NULL;
}

CichlidFilter *
cichlid_filter_new(GtkTreeModel *model)
{
	g_return_val_if_fail(GTK_IS_TREE_MODEL(model), NULL);
	
	return g_object_new(CICHLID_TYPE_FILTER, NULL);
}

static gboolean
cichild_filter_visible_func(GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
	gboolean visible;
	
	if (filter_flags == ((1 << STATUS_GOOD) | (1 << STATUS_BAD) | (1 << STATUS_NOT_VERIFIED)
						 | (1 << STATUS_NOT_FOUND)) && !filter_string)
		visible = TRUE;
	else if (filter_string)
	{
		int status;
		char *filename;
		gtk_tree_model_get(model, iter, CICHLID_CHECKSUM_FILE_STATUS, &status,
						   CICHLID_CHECKSUM_FILE_FILENAME, &filename, -1);
		
		if ((1 << status) & filter_flags && strstr(filename, filter_string))
			visible = TRUE;
		else
			visible = FALSE;
	}
	else
	{
		int status;
		gtk_tree_model_get(model, iter, CICHLID_CHECKSUM_FILE_STATUS, &status, -1);
		
		if ((1 << status) & filter_flags)
			visible = TRUE; 
		else
			visible = FALSE;
	}
	
	return visible;
}
