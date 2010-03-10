/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_cell_renderer.c
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
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "cichlid_file.h"
#include "cichlid_cell_renderer.h"

typedef struct _CichlidCellRendererPrivate CichlidCellRendererPrivate;

struct _CichlidCellRendererPrivate
{
	CichlidFile     *ck_file;
	GtkCellRenderer *text_renderer;
};

enum
{
	P_FILE = 1
};

enum
{
	PAD = 6,
	PAD_SMALL = 3
};

G_DEFINE_TYPE(CichlidCellRenderer, cichlid_cell_renderer, GTK_TYPE_CELL_RENDERER)

static void cichlid_cell_renderer_get_property(GObject    *object,
		                                       guint       property_id,
		                                       GValue     *value,
		                                       GParamSpec *pspec);
static void cichlid_cell_renderer_get_size(GtkCellRenderer *cell,
		                                   GtkWidget       *widget,
		                                   GdkRectangle    *cell_area,
		                                   gint            *x_offset,
		                                   gint            *y_offset,
		                                   gint            *width,
		                                   gint            *height);
static void cichlid_cell_renderer_set_property(GObject    *object,
		                                       guint       property_id,
		                                       const GValue *value,
		                                       GParamSpec *pspec);

//static void
//get_size_minimal( TorrentCellRenderer * cell,
//                  GtkWidget           * widget,
//                  gint                * width,
//                  gint                * height )
//{
//    int w, h;
//    GdkRectangle icon_area;
//    GdkRectangle name_area;
//    GdkRectangle stat_area;
//    const char * name;
//    char * status;
//    GdkPixbuf * icon;
//    GtkCellRenderer * text_renderer;
//
//    struct TorrentCellRendererPrivate * p = cell->priv;
//    const tr_torrent * tor = p->tor;
//    const tr_stat * st = tr_torrentStatCached( (tr_torrent*)tor );
//
//    icon = get_icon( tor, MINIMAL_ICON_SIZE, widget );
//    name = tr_torrentInfo( tor )->name;
//    status = getShortStatusString( st, p->upload_speed, p->download_speed );
//
//    /* get the idealized cell dimensions */
//    g_object_set( p->icon_renderer, "pixbuf", icon, NULL );
//    gtk_cell_renderer_get_size( p->icon_renderer, widget, NULL, NULL, NULL, &w, &h );
//    icon_area.width = w;
//    icon_area.height = h;
//    text_renderer = get_text_renderer( st, cell );
//    g_object_set( text_renderer, "text", name, "ellipsize", PANGO_ELLIPSIZE_NONE,  "scale", 1.0, NULL );
//    gtk_cell_renderer_get_size( text_renderer, widget, NULL, NULL, NULL, &w, &h );
//    name_area.width = w;
//    name_area.height = h;
//    g_object_set( text_renderer, "text", status, "scale", SMALL_SCALE, NULL );
//    gtk_cell_renderer_get_size( text_renderer, widget, NULL, NULL, NULL, &w, &h );
//    stat_area.width = w;
//    stat_area.height = h;
//
//    /**
//    *** LAYOUT
//    **/
//
//    if( width != NULL )
//        *width = cell->parent.xpad * 2 + icon_area.width + GUI_PAD + name_area.width + GUI_PAD + stat_area.width;
//    if( height != NULL )
//        *height = cell->parent.ypad * 2 + name_area.height + p->bar_height;
//
//    /* cleanup */
//    g_free( status );
//    g_object_unref( icon );
//}

static void
cichlid_cell_renderer_get_size(GtkCellRenderer  * cell,
                               GtkWidget        * widget,
                               GdkRectangle     * cell_area,
                               gint             * x_offset,
                               gint             * y_offset,
                               gint             * width,
                               gint             * height)
{
	CichlidCellRenderer *self = CICHLID_CELL_RENDERER(cell);
	CichlidCellRendererPrivate *priv = self->priv;
	GdkRectangle name_area;
	GdkRectangle status_area;
	const char *filename;
	const char *status;
	int w, h;

	status = cichlid_file_get_status_string(priv->ck_file);
	filename = cichlid_file_get_filename(priv->ck_file);

	g_object_set(priv->text_renderer, "text", filename, "scale", 1.0, "ellipsize", PANGO_ELLIPSIZE_NONE, NULL);
	gtk_cell_renderer_get_size(priv->text_renderer, widget, NULL, NULL, NULL, &w, &h);
	name_area.width = w;
	name_area.height = h;
	g_object_set(priv->text_renderer, "text", status, "scale", 1.0, NULL);
	gtk_cell_renderer_get_size(priv->text_renderer, widget, NULL, NULL, NULL, &w, &h);
	status_area.width = w;
	status_area.height = h;

	if (width != NULL)
		*width = self->parent_instance.xpad * 2 + name_area.width + PAD + status_area.width;
	if (height != NULL)
		*height = self->parent_instance.ypad * 2 + MAX(name_area.height, status_area.height);

	if (cell_area)
	{
		if (x_offset)
			*x_offset = 0;
		if (y_offset)
			*y_offset = 0;
	}
}

//static void
//render_minimal( TorrentCellRenderer   * cell,
//                GdkDrawable           * window,
//                GtkWidget             * widget,
//                GdkRectangle          * background_area,
//                GdkRectangle          * cell_area UNUSED,
//                GdkRectangle          * expose_area UNUSED,
//                GtkCellRendererState    flags )
//{
//    int w, h;
//    GdkRectangle icon_area;
//    GdkRectangle name_area;
//    GdkRectangle stat_area;
//    GdkRectangle prog_area;
//    GdkRectangle fill_area;
//    const char * name;
//    char * status;
//    GdkPixbuf * icon;
//    GtkCellRenderer * text_renderer;
//
//    struct TorrentCellRendererPrivate * p = cell->priv;
//    const tr_torrent * tor = p->tor;
//    const tr_stat * st = tr_torrentStatCached( (tr_torrent*)tor );
//    const gboolean active = st->activity != TR_STATUS_STOPPED;
//    const double percentDone = MAX( 0.0, st->percentDone );
//    const gboolean sensitive = active || st->error;
//
//    icon = get_icon( tor, MINIMAL_ICON_SIZE, widget );
//    name = tr_torrentInfo( tor )->name;
//    status = getShortStatusString( st, p->upload_speed, p->download_speed );
//
//    /* get the cell dimensions */
//    g_object_set( p->icon_renderer, "pixbuf", icon, NULL );
//    gtk_cell_renderer_get_size( p->icon_renderer, widget, NULL, NULL, NULL, &w, &h );
//    icon_area.width = w;
//    icon_area.height = h;
//    text_renderer = get_text_renderer( st, cell );
//    g_object_set( text_renderer, "text", name, "ellipsize", PANGO_ELLIPSIZE_NONE, "scale", 1.0, NULL );
//    gtk_cell_renderer_get_size( text_renderer, widget, NULL, NULL, NULL, &w, &h );
//    name_area.width = w;
//    name_area.height = h;
//    g_object_set( text_renderer, "text", status, "scale", SMALL_SCALE, NULL );
//    gtk_cell_renderer_get_size( text_renderer, widget, NULL, NULL, NULL, &w, &h );
//    stat_area.width = w;
//    stat_area.height = h;
//
//    /**
//    *** LAYOUT
//    **/
//
//    fill_area = *background_area;
//    fill_area.x += cell->parent.xpad;
//    fill_area.y += cell->parent.ypad;
//    fill_area.width -= cell->parent.xpad * 2;
//    fill_area.height -= cell->parent.ypad * 2;
//
//    /* icon */
//    icon_area.x = fill_area.x;
//    icon_area.y = fill_area.y + ( fill_area.height - icon_area.height ) / 2;
//
//    /* short status (right justified) */
//    stat_area.x = fill_area.x + fill_area.width - stat_area.width;
//    stat_area.y = fill_area.y + ( name_area.height - stat_area.height ) / 2;
//
//    /* name */
//    name_area.x = icon_area.x + icon_area.width + GUI_PAD;
//    name_area.y = fill_area.y;
//    name_area.width = stat_area.x - GUI_PAD - name_area.x;
//
//    /* progressbar */
//    prog_area.x = name_area.x;
//    prog_area.y = name_area.y + name_area.height;
//    prog_area.width = name_area.width + GUI_PAD + stat_area.width;
//    prog_area.height = p->bar_height;
//
//    /**
//    *** RENDER
//    **/
//
//    g_object_set( p->icon_renderer, "pixbuf", icon, "sensitive", sensitive, NULL );
//    gtk_cell_renderer_render( p->icon_renderer, window, widget, &icon_area, &icon_area, &icon_area, flags );
//    g_object_set( text_renderer, "text", status, "scale", SMALL_SCALE, "sensitive", sensitive, "ellipsize", PANGO_ELLIPSIZE_END, NULL );
//    gtk_cell_renderer_render( text_renderer, window, widget, &stat_area, &stat_area, &stat_area, flags );
//    g_object_set( text_renderer, "text", name, "scale", 1.0, NULL );
//    gtk_cell_renderer_render( text_renderer, window, widget, &name_area, &name_area, &name_area, flags );
//    g_object_set( p->progress_renderer, "value", (int)(percentDone*100.0), "text", "", "sensitive", sensitive, NULL );
//    gtk_cell_renderer_render( p->progress_renderer, window, widget, &prog_area, &prog_area, &prog_area, flags );
//
//    /* cleanup */
//    g_free( status );
//    g_object_unref( icon );
//}

static void
cichlid_cell_renderer_render(GtkCellRenderer       *cell,
                             GdkDrawable           *window,
                             GtkWidget             *widget,
                             GdkRectangle          *background_area,
                             GdkRectangle          *cell_area,
                             GdkRectangle          *expose_area,
                             GtkCellRendererState   flags)
{
    CichlidCellRenderer        *self = CICHLID_CELL_RENDERER(cell);
    CichlidCellRendererPrivate *priv = self->priv;
    GdkRectangle                fill_area = *background_area;
	GdkRectangle name_area;
	GdkRectangle status_area;
	const char *status;	
	const char *filename;
	int w, h;

	status = cichlid_file_get_status_string(priv->ck_file);
	filename = cichlid_file_get_filename(priv->ck_file);

	g_object_set(priv->text_renderer, "text", filename, "scale", 1.0, "ellipsize", PANGO_ELLIPSIZE_NONE, NULL);
	gtk_cell_renderer_get_size(priv->text_renderer, widget, NULL, NULL, NULL, &w, &h);
	name_area.width = w;
	name_area.height = h;
	g_object_set(priv->text_renderer, "text", status, "scale", 1.0, NULL);
	gtk_cell_renderer_get_size(priv->text_renderer, widget, NULL, NULL, NULL, &w, &h);
	status_area.width = w;
	status_area.height = h;

	fill_area.x += self->parent_instance.xpad;
    fill_area.y += self->parent_instance.ypad;
    fill_area.width -= self->parent_instance.xpad * 2;
    fill_area.height -= self->parent_instance.ypad * 2;

    name_area.x = fill_area.x + PAD;
    name_area.y = fill_area.y + PAD_SMALL;
    name_area.width = fill_area.width - status_area.width - 3*PAD;
    status_area.x = name_area.x + name_area.width + PAD;
	status_area.y = fill_area.y + PAD_SMALL;

    g_object_set(priv->text_renderer, "text", filename, "scale", 1.0, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    gtk_cell_renderer_render(priv->text_renderer, window, widget, &name_area, &name_area, &name_area, flags);
    g_object_set(priv->text_renderer, "text", status, "scale", 1.0, NULL);
    gtk_cell_renderer_render(priv->text_renderer, window, widget, &status_area, &status_area, &status_area, flags);
}


static void
cichlid_cell_renderer_dispose(GObject *gobject)
{
    CichlidCellRenderer        *self = CICHLID_CELL_RENDERER(gobject);
    CichlidCellRendererPrivate *priv = self->priv;

    g_object_unref(priv->text_renderer);

	G_OBJECT_CLASS(cichlid_cell_renderer_parent_class)->dispose(gobject);
}

static void
cichlid_cell_renderer_finalize(GObject *gobject)
{
	CichlidCellRenderer *self = CICHLID_CELL_RENDERER(gobject);

	G_OBJECT_CLASS(cichlid_cell_renderer_parent_class)->finalize(gobject);
}


static void
cichlid_cell_renderer_class_init(CichlidCellRendererClass *klass)
{
	GObjectClass         *gobject_class = G_OBJECT_CLASS(klass);
	GtkCellRendererClass *gtkcellrenderer_class = GTK_CELL_RENDERER_CLASS(klass);

	g_type_class_add_private(klass, sizeof(CichlidCellRendererPrivate));

	gobject_class->dispose = cichlid_cell_renderer_dispose;
	gobject_class->finalize = cichlid_cell_renderer_finalize;

	gtkcellrenderer_class->render = cichlid_cell_renderer_render;
	gtkcellrenderer_class->get_size = cichlid_cell_renderer_get_size;

	gobject_class->get_property = cichlid_cell_renderer_get_property;
    gobject_class->set_property = cichlid_cell_renderer_set_property;

    g_object_class_install_property(gobject_class, P_FILE,
    		                        g_param_spec_object("file", NULL,
    		                        		             "CichlidFile",
    		                        		             CICHLID_TYPE_FILE,
    		                        		             G_PARAM_READWRITE));

}

static void
cichlid_cell_renderer_init(CichlidCellRenderer *self)
{
	CichlidCellRendererPrivate *priv;
	priv = self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self, CICHLID_TYPE_CELL_RENDERER, CichlidCellRendererPrivate);

	priv->text_renderer = gtk_cell_renderer_text_new();
	g_object_ref_sink(priv->text_renderer);
	g_object_set(priv->text_renderer, "xpad", 0, "ypad", 0, NULL);
	priv->ck_file = NULL;
}

GtkCellRenderer *
cichlid_cell_renderer_new( void )
{
	return GTK_CELL_RENDERER(g_object_new(CICHLID_TYPE_CELL_RENDERER, NULL));
}

static void
cichlid_cell_renderer_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	CichlidCellRenderer        *self = CICHLID_CELL_RENDERER(object);
	CichlidCellRendererPrivate *priv = self->priv;

	switch(property_id)
	{
	case P_FILE:
		g_value_set_object(value, priv->ck_file);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}

}

static void
cichlid_cell_renderer_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	CichlidCellRenderer        *self = CICHLID_CELL_RENDERER(object);
	CichlidCellRendererPrivate *priv = self->priv;

	switch(property_id)
	{
	case P_FILE:
		priv->ck_file = g_value_get_object(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

