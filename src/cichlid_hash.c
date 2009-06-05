/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash.c
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
#include "cichlid_hash.h"

static void
cichlid_hash_init (gpointer g_class)
{
	static gboolean is_initialized = FALSE;
	if (!is_initialized)
	{
		/* add properties and signals to the interface here */
		is_initialized = TRUE;
	}
}

GType
cichlid_hash_get_type (void)
{
  static GType iface_type = 0;
  if (iface_type == 0)
    {
      static const GTypeInfo info = {
        sizeof (CichlidHashInterface),
        cichlid_hash_init,   /* base_init */
        NULL,   /* base_finalize */
      };

      iface_type = g_type_register_static (G_TYPE_INTERFACE, "CichlidHash",
                                           &info, 0);
    }

  return iface_type;
}

gboolean
cichlid_hash_equals (CichlidHash *self, guint32 *a, guint32 *b)
{
	gboolean retval;
	g_return_val_if_fail (CICHLID_IS_HASH (self), FALSE);
	if (a == NULL || b == NULL)
		return FALSE;

	retval = CICHLID_HASH_GET_INTERFACE (self)->equals (a, b);
	return retval;
}

guint32 *
cichlid_hash_get_hash (CichlidHash *self)
{
	guint32 *retval;
	g_return_val_if_fail (CICHLID_IS_HASH (self), NULL);

	retval = CICHLID_HASH_GET_INTERFACE (self)->get_hash (self);
	return retval;
}

gchar *
cichlid_hash_get_hash_string (CichlidHash *self)
{
	gchar *retval;
	g_return_val_if_fail (CICHLID_IS_HASH (self), NULL);

	retval = CICHLID_HASH_GET_INTERFACE (self)->get_hash_string (self);
	return retval;
}

void
cichlid_hash_update (CichlidHash *self, const gchar *data, gsize data_size)
{
	g_return_if_fail (CICHLID_IS_HASH(self));

	if (data_size == 0)
		return;

	CICHLID_HASH_GET_INTERFACE (self)->update (self, data, data_size);
}

