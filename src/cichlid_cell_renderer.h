/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_cell_renderer.h
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

#ifndef CICHLID_CELL_RENDERER_H
#define CICHLID_CELL_RENDERER_H

#include <glib-object.h>
#include <gtk/gtk.h>

#define CICHLID_TYPE_CELL_RENDERER       		(cichlid_cell_renderer_get_type ())
#define CICHLID_CELL_RENDERER(obj)       		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CICHLID_TYPE_CELL_RENDERER, CichlidCellRenderer))
#define CICHLID_IS_CELL_RENDERER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CICHLID_TYPE_CELL_RENDERER))
#define CICHLID_CELL_RENDERER_CLASS(klass)	    (G_TYPE_CHECK_CLASS_CAST ((klass), CICHLID_TYPE_CELL_RENDERER, CichlidCellRendererClass))
#define CICHLID_IS_CELL_RENDERER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CICHLID_TYPE_CELL_RENDERER))
#define CICHLID_CELL_RENDERER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CICHLID_TYPE_CELL_RENDERER, CichlidCellRendererClass))

typedef struct _CichlidCellRenderer        CichlidCellRenderer;
typedef struct _CichlidCellRendererClass   CichlidCellRendererClass;

struct _CichlidCellRenderer
{
    GtkCellRenderer parent_instance;
    gpointer		priv;
};

struct _CichlidCellRendererClass
{
    GtkCellRendererClass parent_class;
};

GType            cichlid_cell_renderer_get_type();

GtkCellRenderer *cichlid_cell_renderer_new();

#endif /* CICHLID_CELL_RENDERER_H */
