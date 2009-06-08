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

#include <stdint.h>
#include <glib.h>
#include <gio/gio.h>

#include "cichlid_hash.h"

static void
cichlid_hash_init (gpointer g_class)
{
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
cichlid_hash_equals (CichlidHash *self, uint32_t *a, uint32_t *b)
{
	gboolean retval;
	g_return_val_if_fail (CICHLID_IS_HASH (self), FALSE);
	if (a == NULL || b == NULL)
		return FALSE;

	retval = CICHLID_HASH_GET_INTERFACE (self)->equals (a, b);
	return retval;
}

uint32_t *
cichlid_hash_get_hash (CichlidHash *self)
{
	uint32_t *retval;
	g_return_val_if_fail (CICHLID_IS_HASH (self), NULL);

	retval = CICHLID_HASH_GET_INTERFACE (self)->get_hash (self);
	return retval;
}

char *
cichlid_hash_get_hash_string (CichlidHash *self)
{
	char *retval;
	g_return_val_if_fail (CICHLID_IS_HASH (self), NULL);

	retval = CICHLID_HASH_GET_INTERFACE (self)->get_hash_string (self);
	return retval;
}

void
cichlid_hash_update (CichlidHash *self, const char *data, size_t data_size)
{
	g_return_if_fail (CICHLID_IS_HASH(self));

	if (data_size == 0)
		return;

	CICHLID_HASH_GET_INTERFACE (self)->update (self, data, data_size);
}

