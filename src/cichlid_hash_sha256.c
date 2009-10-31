/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha256.c
 *
 * SHA256 algorithm implemented according to FIPS 180-2
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
#include "cichlid_hash_sha256.h"

#define SHA256_HASH_LENGTH 64

static void     cichlid_hash_interface_init(CichlidHashInterface *iface);
static gboolean cichlid_hash_sha256_equals(uint32_t *a, uint32_t *b);
static void     cichlid_hash_sha256_get_hash_properties(uint32_t *h0, uint32_t *hash_length);

G_DEFINE_TYPE_WITH_CODE(CichlidHashSha256, cichlid_hash_sha256, CICHLID_TYPE_HASH_SHA2_32,
						G_IMPLEMENT_INTERFACE(CICHLID_TYPE_HASH, cichlid_hash_interface_init));

static void
cichlid_hash_interface_init(CichlidHashInterface *iface)
{
	iface->equals = cichlid_hash_sha256_equals;
}

static void
cichlid_hash_sha256_dispose(GObject *gobject)
{
	G_OBJECT_CLASS(cichlid_hash_sha256_parent_class)->dispose (gobject);
}

static void
cichlid_hash_sha256_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(cichlid_hash_sha256_parent_class)->finalize (gobject);
}

static void
cichlid_hash_sha256_class_init(CichlidHashSha256Class *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = cichlid_hash_sha256_dispose;
	gobject_class->finalize = cichlid_hash_sha256_finalize;

	CichlidHashSha2_32Class *sha2_class = CICHLID_HASH_SHA2_32_CLASS(klass);
	sha2_class->get_hash_properties = cichlid_hash_sha256_get_hash_properties;
}

static void
cichlid_hash_sha256_init(CichlidHashSha256 *self)
{ }

CichlidHash *
cichlid_hash_sha256_new()
{
	CichlidHashSha256 *_ccalc;
	_ccalc = g_object_new(CICHLID_TYPE_HASH_SHA256, NULL);

	return CICHLID_HASH(_ccalc);
}

static gboolean
cichlid_hash_sha256_equals(uint32_t *a, uint32_t *b)
{
	if (memcmp(a, b, SHA256_HASH_LENGTH/2) == 0)
		return TRUE;
	else
		return FALSE;
}

static void
cichlid_hash_sha256_get_hash_properties(uint32_t *h0, uint32_t *hash_length) 
{
	h0[0] = 0x6a09e667;
	h0[1] = 0xbb67ae85;
	h0[2] = 0x3c6ef372;
	h0[3] = 0xa54ff53a;
	h0[4] = 0x510e527f;
	h0[5] = 0x9b05688c;
	h0[6] = 0x1f83d9ab;
	h0[7] = 0x5be0cd19;

	*hash_length = SHA256_HASH_LENGTH;
}
