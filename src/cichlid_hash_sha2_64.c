/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha2_64.c
 *
 * Implementation of the calculation of 64-bit SHA-2 hashes (SHA384 and SHA512)
 * as defined in FIPS 180-2, with the limitation that the message can at most be
 * 2^64-1 bit (instead of the standard's 2^128-1 bit).
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
#include "cichlid_hash_common.h"
#include "cichlid_hash_sha2_64.h"

static void            cichlid_hash_interface_init(CichlidHashInterface *iface);
static void            cichlid_hash_sha2_64_calc(CichlidHashSha2_64 *self,
												 const char *buf,
												 size_t bytes_read);
static gboolean        cichlid_hash_sha2_64_equals(uint32_t* a, uint32_t* b);
static void            cichlid_hash_sha2_64_final(CichlidHashSha2_64 *self);
static uint32_t       *cichlid_hash_sha2_64_get_hash(CichlidHash *object);
static char           *cichlid_hash_sha2_64_get_hash_string(CichlidHash *object);
static void            cichlid_hash_sha2_64_reinit(CichlidHashSha2_64 *self);
static void            cichlid_hash_sha2_64_update(CichlidHash *object,
												   const char *data,
												   size_t data_size);
static inline uint64_t Ch(uint64_t x, uint64_t y, uint64_t z);
static inline uint64_t Maj(uint64_t x, uint64_t y, uint64_t z);
static inline uint64_t Sigma0(uint64_t x);
static inline uint64_t Sigma1(uint64_t x);
static inline uint64_t sigma0(uint64_t x);
static inline uint64_t sigma1(uint64_t x);

static const uint64_t k[80] = {
	0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
	0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
	0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
	0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
	0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
	0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
	0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
	0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
	0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
	0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
	0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
	0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
	0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
	0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
	0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
	0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
	0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
	0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
	0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
	0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};

G_DEFINE_TYPE_WITH_CODE(CichlidHashSha2_64, cichlid_hash_sha2_64, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(CICHLID_TYPE_HASH, cichlid_hash_interface_init))

static void
cichlid_hash_interface_init(CichlidHashInterface *iface)
{
	iface->equals = cichlid_hash_sha2_64_equals;
	iface->get_hash = cichlid_hash_sha2_64_get_hash;
	iface->get_hash_string = cichlid_hash_sha2_64_get_hash_string;
	iface->update = cichlid_hash_sha2_64_update;
}

static void
cichlid_hash_sha2_64_dispose(GObject *gobject)
{
	G_OBJECT_CLASS(cichlid_hash_sha2_64_parent_class)->dispose(gobject);
}

static void
cichlid_hash_sha2_64_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(cichlid_hash_sha2_64_parent_class)->finalize(gobject);
}

static void
cichlid_hash_sha2_64_class_init(CichlidHashSha2_64Class *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = cichlid_hash_sha2_64_dispose;
	gobject_class->finalize = cichlid_hash_sha2_64_finalize;

	klass->get_hash_properties = NULL;
}


static void
cichlid_hash_sha2_64_init(CichlidHashSha2_64 *self)
{
	self->initialized = FALSE;
	self->hash_computed = TRUE; /* To force reinit */
	self->total_size = 0;
	self->data_left_size = 0;
}

/**
 * Calculates the partial checksum for buf
 * @param buf data buffer
 * @param bytes_read size of data buffer
 */
static void
cichlid_hash_sha2_64_calc(CichlidHashSha2_64 *self, const char *data, size_t bytes_read)
{
	uint64_t a, b, c, d, e, f, g, h, t1, t2;
	uint64_t w[80];

	/* Process data in 512-bit chunks */
	for (int j = 0; j < bytes_read/128; j++)
	{
		CHANGE_ENDIANNESS_64(w, (uint64_t*)(data+j*64), 16);

		/* Extend w to contain 64 uint32_t */
		for (int i = 16; i < 80; i++)
			w[i] = w[i-16] + sigma0(w[i-15]) + w[i-7] + sigma1(w[i-2]);

		/* Initialize with the current values */
		a = self->h[0];
		b = self->h[1];
		c = self->h[2];
		d = self->h[3];
		e = self->h[4];
		f = self->h[5];
		g = self->h[6];
		h = self->h[7];

		/* Calculate */
		for (int i = 0; i < 80; i++)
		{
			t1 = h + Sigma1(e) + Ch(e, f, g) + k[i] + w[i];
			t2 = Sigma0(a) + Maj(a, b, c);

			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}

		/* Add this chunks result to the total result */
		self->h[0] += a;
		self->h[1] += b;
		self->h[2] += c;
		self->h[3] += d;
		self->h[4] += e;
		self->h[5] += f;
		self->h[6] += g;
		self->h[7] += h;
	}
}

static gboolean
cichlid_hash_sha2_64_equals(uint32_t* a, uint32_t* b)
{
	return FALSE;
}

static void
cichlid_hash_sha2_64_final(CichlidHashSha2_64 *self)
{
	char     *buf;
	goffset   size_offset;
	uint32_t  tmp;
	char     *total_size_p;
	
	if (!self->hash_computed)
	{
		if (self->data_left_size < 112) {
			buf = g_malloc0(sizeof(char) * 128);
			size_offset = 112 + 8;
		}
		else {
			buf = g_malloc0(sizeof(char) * 128 * 2);
			size_offset = 128 + 112 + 8;
		}

		/* Copy the data that is left, the ending 1 and the size to the buffer */
		memcpy(buf, self->data_left, self->data_left_size);
		buf[self->data_left_size] = -0x80;		 /* Signed char */
		self->total_size = self->total_size * 8; /* Convert size to bits */

		total_size_p = (char *)&self->total_size;
		memcpy(&tmp, total_size_p + 4, 4);
		CHANGE_ENDIANNESS(buf + size_offset, &tmp, 1);
		memcpy(&tmp, total_size_p, 4);
		CHANGE_ENDIANNESS(buf + size_offset + 4, &tmp, 1);
		
		cichlid_hash_sha2_64_calc(self, buf, size_offset + 8);

		g_free(buf);
		self->hash_computed = TRUE;
	}
}

/**
 * Returns the resulting hash as an array of integers
 * @param object
 * @return an array of for uint32_t* if successful, else NULL.
 */
static uint32_t *
cichlid_hash_sha2_64_get_hash(CichlidHash *object)
{
	CichlidHashSha2_64	*self;
	uint32_t		    *hash;

	self = CICHLID_HASH_SHA2_64(object);

	cichlid_hash_sha2_64_final(self);
	
	hash = g_malloc(sizeof(uint32_t)*self->hash_size/8);
	memcpy(hash, self->h, self->hash_size/2);

	return hash;
}

/**
 * Returns the resulting hash as a byte string
 * @param object
 * @return a NULL-terminated string with the checksum if successful, else NULL.
 */
static char *
cichlid_hash_sha2_64_get_hash_string(CichlidHash *object)
{
	CichlidHashSha2_64 *self = CICHLID_HASH_SHA2_64(object);
	uint32_t *hash;
	char     *hash_string;

	hash = cichlid_hash_sha2_64_get_hash(object);
	if (hash == NULL)
		return NULL;

	hash_string = g_malloc(sizeof(char)*(self->hash_size + 1));
	for (int i = 0; i < self->hash_size/8; ++i)
		sprintf(hash_string + 8*i, "%.16lx", hash[i]);
	hash_string[self->hash_size] = '\0';

	g_free(hash);
	return hash_string;
}

static void
cichlid_hash_sha2_64_reinit(CichlidHashSha2_64 *self)
{
	if (!self->initialized)
	{
		CICHLID_HASH_SHA2_64_GET_CLASS(self)->get_hash_properties(self->h0, &self->hash_size);
		self->initialized = TRUE;	}

	for (int i = 0; i < 8; ++i)
		self->h[i] = self->h0[i];

	self->hash_computed = FALSE;
	self->total_size = 0;
	self->data_left_size = 0;
}

static void
cichlid_hash_sha2_64_update(CichlidHash *object, const char *data, size_t data_size)
{
	CichlidHashSha2_64 *self;
	char               *buf = NULL;
	uint8_t             new_data_left_size;

	self = CICHLID_HASH_SHA2_64(object);

	if (self->hash_computed)
		cichlid_hash_sha2_64_reinit(self);

	if (!data_size)
		return;

	self->total_size += data_size;

	/* If there is no data left since the previous update */
	if (!self->data_left_size)
	{
		self->data_left_size = data_size % 128;
		memcpy(self->data_left, data + data_size - self->data_left_size, self->data_left_size);

		cichlid_hash_sha2_64_calc(self, data, data_size - self->data_left_size);
	}
	/* If there is data left since the previous update but the total data size < 64 bytes */
	else if (data_size + self->data_left_size < 128)
	{
		memcpy(self->data_left + self->data_left_size, data, data_size);
		self->data_left_size += data_size;
	}
	/* If there is data left since the previous update*/
	else
	{
		new_data_left_size = (data_size + self->data_left_size) % 128;
		buf = g_malloc(sizeof(guchar)*(data_size + self->data_left_size - new_data_left_size));
		memcpy(buf, self->data_left, self->data_left_size);
		memcpy(buf + self->data_left_size, data, data_size - new_data_left_size);
		memcpy(self->data_left, data + data_size - new_data_left_size, new_data_left_size);

		cichlid_hash_sha2_64_calc(self, buf, data_size + self->data_left_size - new_data_left_size);
		self->data_left_size = new_data_left_size;

		g_free(buf);
	}
}

/**
 * Ch as defined in FIPS 180-2
 * @param x
 * @param y
 * @param z
 * @return calculated result
 */
static inline uint64_t
Ch(uint64_t x, uint64_t y, uint64_t z)
{
	return (x & y) ^ ((~x) & z);
}

/**
 * Maj as defined in FIPS 180-2
 * @param x
 * @param y
 * @param z
 * @return calculated result
 */
static inline uint64_t
Maj(uint64_t x, uint64_t y, uint64_t z)
{
	return (x & y) ^ (x & z) ^ (y & z);
}

/**
 * Sigma0 as defined in FIPS 180-2
 * @param x
 * @return calculated result
 */
static inline uint64_t
Sigma0(uint64_t x)
{
	return ROTATE_RIGHT_64(x, 28) ^ ROTATE_RIGHT_64(x, 34) ^ ROTATE_RIGHT_64(x, 39);
}

/**
 * Sigma1 as defined in FIPS 180-2
 * @param x
 * @return calculated result
 */
static inline uint64_t
Sigma1(uint64_t x)
{
	return ROTATE_RIGHT_64(x, 14) ^ ROTATE_RIGHT_64(x, 18) ^ ROTATE_RIGHT_64(x, 41);
}

/**
 * sigma0 as defined in FIPS 180-2
 * @param x
 * @return calculated result
 */
static inline uint64_t
sigma0(uint64_t x)
{
	return ROTATE_RIGHT_64(x, 1) ^ ROTATE_RIGHT_64(x, 8) ^ (x >> 7);
}

/**
 * sigma1 as defined in FIPS 180-2
 * @param x
 * @return calculated result
 */
static inline uint64_t
sigma1(uint64_t x)
{
	return ROTATE_RIGHT_64(x, 19) ^ ROTATE_RIGHT_64(x, 61) ^ (x >> 6);
}
