/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha2_32.c
 *
 * Implementation of the calculation of 32-bit SHA-2 hashes (SHA224 and SHA256)
 * as defined in FIPS 180-2
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
#include "cichlid_hash_sha2_32.h"

static void            cichlid_hash_interface_init(CichlidHashInterface *iface);
static void            cichlid_hash_sha2_32_calc(CichlidHashSha2_32 *self,
												 const char *buf,
												 size_t bytes_read);
static gboolean        cichlid_hash_sha2_32_equals(uint32_t* a, uint32_t* b);
static void            cichlid_hash_sha2_32_final(CichlidHashSha2_32 *self);
static uint32_t       *cichlid_hash_sha2_32_get_hash(CichlidHash *object);
static char           *cichlid_hash_sha2_32_get_hash_string(CichlidHash *object);
static void            cichlid_hash_sha2_32_reinit(CichlidHashSha2_32 *self);
static void            cichlid_hash_sha2_32_update(CichlidHash *object,
												   const char *data,
												   size_t data_size);
static inline uint32_t Ch(uint32_t x, uint32_t y, uint32_t z);
static inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z);
static inline uint32_t Sigma0(uint32_t x);
static inline uint32_t Sigma1(uint32_t x);
static inline uint32_t sigma0(uint32_t x);
static inline uint32_t sigma1(uint32_t x);

static const uint32_t k[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

G_DEFINE_TYPE_WITH_CODE(CichlidHashSha2_32, cichlid_hash_sha2_32, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(CICHLID_TYPE_HASH, cichlid_hash_interface_init));

static void
cichlid_hash_interface_init(CichlidHashInterface *iface)
{
	iface->equals = cichlid_hash_sha2_32_equals;
	iface->get_hash = cichlid_hash_sha2_32_get_hash;
	iface->get_hash_string = cichlid_hash_sha2_32_get_hash_string;
	iface->update = cichlid_hash_sha2_32_update;
}

static void
cichlid_hash_sha2_32_dispose(GObject *gobject)
{
	G_OBJECT_CLASS(cichlid_hash_sha2_32_parent_class)->dispose(gobject);
}

static void
cichlid_hash_sha2_32_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(cichlid_hash_sha2_32_parent_class)->finalize(gobject);
}

static void
cichlid_hash_sha2_32_class_init(CichlidHashSha2_32Class *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	gobject_class->dispose = cichlid_hash_sha2_32_dispose;
	gobject_class->finalize = cichlid_hash_sha2_32_finalize;

	klass->get_hash_properties = NULL;
}


static void
cichlid_hash_sha2_32_init(CichlidHashSha2_32 *self)
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
cichlid_hash_sha2_32_calc(CichlidHashSha2_32 *self, const char *data, size_t bytes_read)
{
	uint32_t a, b, c, d, e, f, g, h, t1, t2;
	uint32_t w[64];

	/* Process data in 512-bit chunks */
	for (int j = 0; j < bytes_read/64; j++)
	{
		CHANGE_ENDIANNESS(w, (uint32_t*)(data+j*64), 16);

		/* Extend w to contain 64 uint32_t */
		for (int i = 16; i < 64; i++)
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
		for (int i = 0; i < 64; i++)
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
cichlid_hash_sha2_32_equals(uint32_t* a, uint32_t* b)
{
	return FALSE;
}

static void
cichlid_hash_sha2_32_final(CichlidHashSha2_32 *self)
{
	char     *buf;
	goffset   size_offset;
	uint32_t  tmp;
	char     *total_size_p;
	
	if (!self->hash_computed)
	{
		if (self->data_left_size < 56) {
			buf = g_malloc0(sizeof(char) * 64);
			size_offset = 56;
		}
		else {
			buf = g_malloc0(sizeof(char) * 64 * 2);
			size_offset = 64 + 56;
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
		
		cichlid_hash_sha2_32_calc(self, buf, size_offset + 8);

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
cichlid_hash_sha2_32_get_hash(CichlidHash *object)
{
	CichlidHashSha2_32	*self;
	uint32_t		    *hash;

	self = CICHLID_HASH_SHA2_32(object);

	cichlid_hash_sha2_32_final(self);
	
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
cichlid_hash_sha2_32_get_hash_string(CichlidHash *object)
{
	CichlidHashSha2_32 *self = CICHLID_HASH_SHA2_32(object);
	uint32_t *hash;
	char     *hash_string;

	hash = cichlid_hash_sha2_32_get_hash(object);
	if (hash == NULL)
		return NULL;

	hash_string = g_malloc(sizeof(char)*(self->hash_size + 1));
	for (int i = 0; i < self->hash_size/8; ++i)
		sprintf(hash_string + 8*i, "%.8x", hash[i]);
	hash_string[self->hash_size] = '\0';

	g_free(hash);
	return hash_string;
}

static void
cichlid_hash_sha2_32_reinit(CichlidHashSha2_32 *self)
{
	if (!self->initialized)
	{
		CICHLID_HASH_SHA2_32_GET_CLASS(self)->get_hash_properties(self->h0, &self->hash_size);
		self->initialized = TRUE;
	}

	for (int i = 0; i < 8; ++i)
		self->h[i] = self->h0[i];

	self->hash_computed = FALSE;
	self->total_size = 0;
	self->data_left_size = 0;
}

static void
cichlid_hash_sha2_32_update(CichlidHash *object, const char *data, size_t data_size)
{
	CichlidHashSha2_32 *self;
	char               *buf = NULL;
	uint8_t             new_data_left_size;

	self = CICHLID_HASH_SHA2_32(object);

	if (self->hash_computed)
		cichlid_hash_sha2_32_reinit(self);

	if (!data_size)
		return;

	self->total_size += data_size;

	/* If there is no data left since the previous update */
	if (!self->data_left_size)
	{
		self->data_left_size = data_size % 64;
		memcpy(self->data_left, data + data_size - self->data_left_size, self->data_left_size);

		cichlid_hash_sha2_32_calc(self, data, data_size - self->data_left_size);
	}
	/* If there is data left since the previous update but the total data size < 64 bytes */
	else if (data_size + self->data_left_size < 64)
	{
		memcpy(self->data_left + self->data_left_size, data, data_size);
		self->data_left_size += data_size;
	}
	/* If there is data left since the previous update*/
	else
	{
		new_data_left_size = (data_size + self->data_left_size) % 64;
		buf = g_malloc(sizeof(guchar)*(data_size + self->data_left_size - new_data_left_size));
		memcpy(buf, self->data_left, self->data_left_size);
		memcpy(buf + self->data_left_size, data, data_size - new_data_left_size);
		memcpy(self->data_left, data + data_size - new_data_left_size, new_data_left_size);

		cichlid_hash_sha2_32_calc(self, buf, data_size + self->data_left_size - new_data_left_size);
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
static inline uint32_t
Ch(uint32_t x, uint32_t y, uint32_t z)
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
static inline uint32_t
Maj(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) ^ (x & z) ^ (y & z);
}

/**
 * Sigma0 as defined in FIPS 180-2
 * @param x
 * @return calculated result
 */
static inline uint32_t
Sigma0(uint32_t x)
{
	return ROTATE_RIGHT(x, 2) ^ ROTATE_RIGHT(x, 13) ^ ROTATE_RIGHT(x, 22);
}

/**
 * Sigma1 as defined in FIPS 180-2
 * @param x
 * @return calculated result
 */
static inline uint32_t
Sigma1(uint32_t x)
{
	return ROTATE_RIGHT(x, 6) ^ ROTATE_RIGHT(x, 11) ^ ROTATE_RIGHT(x, 25);
}

/**
 * sigma0 as defined in FIPS 180-2
 * @param x
 * @return calculated result
 */
static inline uint32_t
sigma0(uint32_t x)
{
	return ROTATE_RIGHT(x, 7) ^ ROTATE_RIGHT(x, 18) ^ (x >> 3);
}

/**
 * sigma1 as defined in FIPS 180-2
 * @param x
 * @return calculated result
 */
static inline uint32_t
sigma1(uint32_t x)
{
	return ROTATE_RIGHT(x, 17) ^ ROTATE_RIGHT(x, 19) ^ (x >> 10);
}
