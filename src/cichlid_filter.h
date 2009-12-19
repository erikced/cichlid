/*
 * Copyright © 2008-2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_file.h
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

#ifndef CICHLID_FILTER_H
#define CICHLID_FILTER_H

#include <glib-object.h>
#include <gio/gio.h>

#define CICHLID_TYPE_FILTER       		(cichlid_filter_get_type())
#define CICHLID_FILTER(obj)       		(G_TYPE_CHECK_INSTANCE_CAST((obj), CICHLID_TYPE_FILTER, CichlidFilter))
#define CICHLID_IS_FILTER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), CICHLID_TYPE_FILTER))
#define CICHLID_FILTER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),  CICHLID_TYPE_FILTER, CichlidFilterClass))
#define CICHLID_IS_FILTER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),  CICHLID_TYPE_FILTER))
#define CICHLID_FILTER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj),  CICHLID_TYPE_FILTER, CichlidFilterClass))

typedef struct _CichlidFilter        CichlidFilter;
typedef struct _CichlidFilterClass   CichlidFilterClass;

struct _CichlidFilter
{
	GtkTreeModelFilter parent_instance;
	gpointer           priv;
};

struct _CichlidFilterClass
{
	GtkTreeModelFilterClass parent_class;
};

GType          cichlid_filter_get_type();

CichlidFilter *cichlid_filter_new();

#endif /* CICHLID_FILTER_H */
