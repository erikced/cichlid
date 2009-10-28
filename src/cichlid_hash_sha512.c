/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha512.c
 *
 * SHA512 algorithm implemented according to FIPS 180-2
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
#include "cichlid_hash_sha512.h"

#define SHA512_HASH_LENGTH 128

static void     cichlid_hash_interface_init(CichlidHashInterface *iface);
static gboolean cichlid_hash_sha512_equals(uint32_t *a, uint32_t *b);
static void     cichlid_hash_sha512_get_hash_properties(uint64_t *h0, uint32_t *hash_length);

G_DEFINE_TYPE_WITH_CODE(CichlidHashSha512, cichlid_hash_sha512, CICHLID_TYPE_HASH_SHA2_64,
						G_IMPLEMENT_INTERFACE(CICHLID_TYPE_HASH, cichlid_hash_interface_init));

static void
cichlid_hash_interface_init(CichlidHashInterface *iface)
{
	iface->equals = cichlid_hash_sha512_equals;
}

static void
cichlid_hash_sha512_dispose(GObject *gobject)
{
	G_OBJECT_CLASS (cichlid_hash_sha512_parent_class)->dispose (gobject);
}

static void
cichlid_hash_sha512_finalize(GObject *gobject)
{
	G_OBJECT_CLASS (cichlid_hash_sha512_parent_class)->finalize (gobject);
}

static void
cichlid_hash_sha512_class_init(CichlidHashSha512Class *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = cichlid_hash_sha512_dispose;
	gobject_class->finalize = cichlid_hash_sha512_finalize;

	CichlidHashSha2_64Class *sha2_class = CICHLID_HASH_SHA2_64_CLASS(klass);
	sha2_class->get_hash_properties = cichlid_hash_sha512_get_hash_properties;
}

static void
cichlid_hash_sha512_init(CichlidHashSha512 *self)
{ }

CichlidHash *
cichlid_hash_sha512_new()
{
	CichlidHashSha512 *_ccalc;
	_ccalc = g_object_new(CICHLID_TYPE_HASH_SHA512, NULL);

	return CICHLID_HASH(_ccalc);
}

static gboolean
cichlid_hash_sha512_equals(uint32_t *a, uint32_t *b)
{
	if (memcmp(a, b, SHA512_HASH_LENGTH/2) == 0)
		return TRUE;
	else
		return FALSE;
}

static void
cichlid_hash_sha512_get_hash_properties(uint64_t *h0, uint32_t *hash_length) 
{
	h0[0] = 0x6a09e667f3bcc908;
	h0[1] = 0xbb67ae8584caa73b;
	h0[2] = 0x3c6ef372fe94f82b;
	h0[3] = 0xa54ff53a5f1d36f1;
	h0[4] = 0x510e527fade682d1;
	h0[5] = 0x9b05688c2b3e6c1f;
	h0[6] = 0x1f83d9abfb41bd6b;
	h0[7] = 0x5be0cd19137e2179;

	*hash_length = SHA512_HASH_LENGTH;
}
