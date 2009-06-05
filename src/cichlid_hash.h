/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash.h
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

#ifndef CICHLID_HASH_H
#define CICHLID_HASH_H

#include <glib-object.h>

#define CICHLID_TYPE_HASH       		(cichlid_hash_get_type ())
#define CICHLID_HASH(obj)       		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CICHLID_TYPE_HASH, CichlidHash))
#define CICHLID_IS_HASH(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CICHLID_TYPE_HASH))
#define CICHLID_HASH_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE ((inst), CICHLID_TYPE_HASH, CichlidHashInterface))

typedef struct _CichlidHash			CichlidHash;
typedef struct _CichlidHashInterface	CichlidHashInterface;

struct _CichlidHashInterface
{
  GObjectClass parent_iface;

  gboolean (* equals) (guint32* a, guint32* b);
  guint32 *(* get_hash) (CichlidHash *self);
  gchar   *(* get_hash_string) (CichlidHash *self);
  void	   (* update) (CichlidHash *self, const gchar *data, gsize data_size);
};

GType cichlid_hash_get_type (void);

gboolean cichlid_hash_equals (CichlidHash *self, guint32* a, guint32* b);
guint32 *cichlid_hash_get_hash (CichlidHash *self);
gchar *cichlid_hash_get_hash_string (CichlidHash *self);
void cichlid_hash_update (CichlidHash *self, const gchar *data, gsize data_size);

#endif /* CICHLID_HASH_H */
