/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha224.c
 *
 * SHA224 algorithm implemented according to FIPS 180-2
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
#include <gio/gio.h>
#include <glib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cichlid_hash.h"
#include "cichlid_hash_sha224.h"

#define SHA224_HASH_LENGTH 56

static void     cichlid_hash_interface_init(CichlidHashInterface *iface);
static gboolean cichlid_hash_sha224_equals(uint32_t *a, uint32_t *b);
static void     cichlid_hash_sha224_get_hash_properties(uint32_t h0[8], uint32_t *hash_length);

G_DEFINE_TYPE_WITH_CODE(CichlidHashSha224, cichlid_hash_sha224, CICHLID_TYPE_HASH_SHA2_32,
						G_IMPLEMENT_INTERFACE(CICHLID_TYPE_HASH, cichlid_hash_interface_init));

static void
cichlid_hash_interface_init(CichlidHashInterface *iface)
{
	iface->equals = cichlid_hash_sha224_equals;
}

static void
cichlid_hash_sha224_dispose(GObject *gobject)
{
	G_OBJECT_CLASS(cichlid_hash_sha224_parent_class)->dispose (gobject);
}

static void
cichlid_hash_sha224_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(cichlid_hash_sha224_parent_class)->finalize (gobject);
}

static void
cichlid_hash_sha224_class_init(CichlidHashSha224Class *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = cichlid_hash_sha224_dispose;
	gobject_class->finalize = cichlid_hash_sha224_finalize;

	CichlidHashSha2_32Class *sha2_class = CICHLID_HASH_SHA2_32_CLASS(klass);
	sha2_class->get_hash_properties = cichlid_hash_sha224_get_hash_properties;
}


static void
cichlid_hash_sha224_init(CichlidHashSha224 *self)
{ }

CichlidHash *
cichlid_hash_sha224_new()
{
	CichlidHashSha224 *_ccalc;
	_ccalc = g_object_new(CICHLID_TYPE_HASH_SHA224, NULL);

	return CICHLID_HASH(_ccalc);
}

static gboolean
cichlid_hash_sha224_equals(uint32_t *a, uint32_t *b)
{
	if (memcmp(a, b, SHA224_HASH_LENGTH/2) == 0)
		return TRUE;
	else
		return FALSE;
}

static void
cichlid_hash_sha224_get_hash_properties(uint32_t h0[8], uint32_t *hash_length) 
{
	h0[0] = 0xc1059ed8;
	h0[1] = 0x367cd507;
	h0[2] = 0x3070dd17;
	h0[3] = 0xf70e5939;
	h0[4] = 0xffc00b31;
	h0[5] = 0x68581511;
	h0[6] = 0x64f98fa7;
	h0[7] = 0xbefa4fa4;

	*hash_length = SHA224_HASH_LENGTH;
}
