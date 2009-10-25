/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_sha224.c
 *
 * Class for computing checksums according to the
 * RSA Data Security, Inc. SHA256 Message-Digest Algorithm
 * as defined in RFC 1321 from April 1992.
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

static void            cichlid_hash_sha224_calc(CichlidHashSha224 *self, const char *buf, size_t bytes_read);
static gboolean        cichlid_hash_sha224_equals(uint32_t* a, uint32_t* b);
static uint32_t       *cichlid_hash_sha224_get_hash(CichlidHash *object);
static char           *cichlid_hash_sha224_get_hash_string(CichlidHash *object);
static void            cichlid_hash_sha224_update(CichlidHash *object, const char *data, size_t data_size);
static void            cichlid_hash_interface_init(CichlidHashInterface *iface);


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

G_DEFINE_TYPE_WITH_CODE(CichlidHashSha224, cichlid_hash_sha224, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(CICHLID_TYPE_HASH, cichlid_hash_interface_init));

/*
 * Initialize the interface
 */
static void
cichlid_hash_interface_init(CichlidHashInterface *iface)
{
	iface->equals = cichlid_hash_sha224_equals;
	iface->get_hash = cichlid_hash_sha224_get_hash;
	iface->get_hash_string = cichlid_hash_sha224_get_hash_string;
	iface->update = cichlid_hash_sha224_update;

}

static void
cichlid_hash_sha224_dispose(GObject *gobject)
{
	CichlidHashSha224 *self = CICHLID_HASH_SHA224 (gobject);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(cichlid_hash_sha224_parent_class)->dispose (gobject);
}

static void
cichlid_hash_sha224_finalize(GObject *gobject)
{
	CichlidHashSha224 *self = CICHLID_HASH_SHA224 (gobject);

	G_OBJECT_CLASS(cichlid_hash_sha224_parent_class)->finalize (gobject);
}


static void
cichlid_hash_sha224_class_init(CichlidHashSha224Class *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->dispose = cichlid_hash_sha224_dispose;
	gobject_class->finalize = cichlid_hash_sha224_finalize;

	/* Initiera funktioner */
}


static void
cichlid_hash_sha224_init(CichlidHashSha224 *self)
{
	/* Partial result variables */
	self->p[0] = 0x6a09e667;
	self->p[1] = 0xbb67ae85;
	self->p[2] = 0x3c6ef372;
	self->p[3] = 0xa54ff53a;
	self->p[4] = 0x510e527f;
	self->p[5] = 0x9b05688c;
	self->p[6] = 0x1f83d9ab;
	self->p[7] = 0x5be0cd19;

	self->hash_computed = FALSE;
	self->total_size = 0;
	self->data_left_size = 0;
}

CichlidHash *
cichlid_hash_sha224_new()
{
	CichlidHashSha224 *_ccalc;
	_ccalc = g_object_new(CICHLID_TYPE_HASH_SHA224,NULL);

	return CICHLID_HASH(_ccalc);
}

static gboolean
cichlid_hash_sha224_equals(uint32_t* a, uint32_t* b)
{
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static void
cichlid_hash_sha224_update(CichlidHash *object, const char *data, size_t data_size)
{
	CichlidHashSha224 *self;
	char *buf = NULL;
	uint8_t new_data_left_size;

	self = CICHLID_HASH_SHA224(object);

	if (self->hash_computed)
		cichlid_hash_sha224_init(self);

	if (!data_size)
		return;

	self->total_size += data_size;

	/* If there is no data left since the previous update */
	if (!self->data_left_size) {
		self->data_left_size = data_size % 64;
		memcpy(self->data_left, data + data_size - self->data_left_size, self->data_left_size);

		cichlid_hash_sha224_calc(self, data, data_size - self->data_left_size);
	}
	/* If there is data left since the previous update but the total data size < 64 bytes */
	else if (data_size + self->data_left_size < 64)	{
		memcpy(self->data_left + self->data_left_size, data, data_size);
		self->data_left_size += data_size;
	}
	/* If there is data left since the previous update*/
	else {
		new_data_left_size = (data_size + self->data_left_size) % 64;
		buf = g_malloc(sizeof(guchar)*(data_size + self->data_left_size - new_data_left_size));
		memcpy(buf, self->data_left, self->data_left_size);
		memcpy(buf + self->data_left_size, data, data_size - new_data_left_size);
		memcpy(self->data_left, data + data_size - new_data_left_size, new_data_left_size);

		cichlid_hash_sha224_calc(self, buf, data_size + self->data_left_size - new_data_left_size);
		self->data_left_size = new_data_left_size;

		g_free(buf);
	}
}

/**
 * Returns the resulting hash as an array of integers
 * @param object
 * @return an array of for uint32_t* if successful, else NULL.
 */
static uint32_t *
cichlid_hash_sha224_get_hash(CichlidHash *object)
{
	CichlidHashSha224	*self;
	char 			*buf;
	goffset 		size_offset;
	uint32_t			*hash;

	self = CICHLID_HASH_SHA224(object);
	if (!self->hash_computed)
	{
		if (self->data_left_size < 56)
		{
			buf = g_malloc0(sizeof(char) * 64);
			size_offset = 56;
		}
		else
		{
			buf = g_malloc0(sizeof(char) * 64 * 2);
			size_offset = 64 + 56;
		}
		fprintf(stderr, "%lu\n", self->total_size);
		
		/* Copy the data that is left, the ending 1 and the size to the buffer */
		memcpy(buf, self->data_left, self->data_left_size);
		buf[self->data_left_size] = -0x80; /* Signed char */
		self->total_size = self->total_size * 8; /* Convert size to bits */
		//		for (int i = 0; i < 8; i++)
		//	memcpy(buf + size_offset + i,(char *)&self->total_size + 7 - i, 1);
		flip_endianness(buf + size_offset, (char *)&self->total_size + 4, 1);
		flip_endianness(buf + size_offset + 4, (char *)&self->total_size, 1);
		cichlid_hash_sha224_calc(self, buf, size_offset + 8);

		g_free(buf);
		self->hash_computed = TRUE;
	}

	/* Copy the hash and switch endianness */
	hash = g_malloc(sizeof(uint32_t)*8);
	memcpy(hash, self->p, 32);
	//	flip_endianness(hash, self->p, 8);

	return hash;
}

/**
 * Returns the resulting hash as a byte string
 * @param object
 * @return a NULL-terminated string with the checksum if successful, else NULL.
 */
static char *
cichlid_hash_sha224_get_hash_string(CichlidHash *object)
{
	char *hash_string;
	uint32_t *hash;

	hash = cichlid_hash_sha224_get_hash(object);
	if (hash == NULL)
		return NULL;

	hash_string = g_malloc(sizeof(char)*65);
	sprintf(hash_string, "%.8x%.8x%.8x%.8x%.8x%.8x%.8x%.8x",
			hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8]);

	g_free(hash);
	return hash_string;
}

/**
 * Calculates the partial checksum for buf
 * @param buf data buffer
 * @param bytes_read size of data buffer
 */
static void
cichlid_hash_sha224_calc(CichlidHashSha224 *self, const char *buf, size_t bytes_read)
{
	uint32_t a, b, c, d, e, f, g, h, s0, s1, t1, t2, ch, maj;
	uint32_t w[64];

	/* Process buf in 512-bit (64-byte) chunks */
	for (int j = 0; j < bytes_read/64; j++)
	{
		flip_endianness(w, buf+j*64, 16);
		
		//		for (int i = 0; i < 16; i++)
		//	fprintf(stderr, "w[%i] = %x\n", i, w[i]);
		
		/* Extend w to contain 64 uint32_t */
		for (int i = 16; i < 64; i++)
		{
			s0 = rotate_right(w[i-15], 7) ^ rotate_right(w[i-15], 18) ^ (w[i-15] >> 3);
			s1 = rotate_right(w[i-2], 17) ^ rotate_right(w[i-2], 19) ^ (w[i-2] >> 10);
			w[i] = w[i-16] + s0 + w[i-7] + s1;
		}
		
		/* Initialize with the current values */
		a = self->p[0];
		b = self->p[1];
		c = self->p[2];
		d = self->p[3];
		e = self->p[4];
		f = self->p[5];
		g = self->p[6];
		h = self->p[7];
		
		/* Calculate */
		for (int i = 0; i < 64; i++)
		{
			s0 = rotate_right(a, 2) ^ rotate_right(a, 13) ^ rotate_right(a, 22);
			maj = (a & b) ^ (a & c) ^ (b & c);
			t2 = s0 + maj;
			s1 = rotate_right(e, 6) ^ rotate_right(e, 11) ^ rotate_right(e, 25);
			ch = (e & f) ^ ((~e) & g);
			t1 = h + s1 + ch + k[i] + w[i];
			
			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
			
			//			fprintf(stderr, "t = %i : %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x\n", i,
			//					a, b, c, d, e, f, g, h);
		}
		
		/* Add this chunks result to the total result */
		self->p[0] += a;
		self->p[1] += b;
		self->p[2] += c;
		self->p[3] += d;
		self->p[4] += e;
		self->p[5] += f;
		self->p[6] += g;
		self->p[7] += h;
	}							
}

/**
 * Flips the endianness of n consecutive 32-bit integers in source and saves it in destination
 * @param destination
 * @param source
 */
static inline void
flip_endianness(void *_destination, const void *_source, uint8_t n)
{
	uint32_t *source = _source;
	uint32_t *destination = _destination;
	for (int i = 0; i < n; i++)
		destination[i] = ((source[i] & 0xFF) << 24) |
		((source[i] & 0xFF00) << 8) |
		((source[i] & 0xFF0000) >> 8) |
		((source[i] & 0xFF000000) >> 24);
}

/**
 * Sigma0 as defined in FIPS 180-2
 * @param x
 * @return calculated result
 */
static inline uint32_t
Sigma0(uint32_t x)
{
	return rotate_right(x, 2) ^ rotate_right(x, 13) ^ rotate_right(x, 22);
}

/**
 * Sigma1 as defined in FIPS 180-2
 * @param x
 * @return calculated result
 */
static inline uint32_t
Sigma1(uint32_t x)
{
	return rotate_right(x, 6) ^ rotate_right(x, 11) ^ rotate_right(x, 25);
}

/**
 * sigma0 as defined in FIPS 180-2
 * @param x
 * @return calculated result
 */
static inline uint32_t
sigma0(uint32_t x)
{
	return rotate_right(x, 7) ^ rotate_right(x, 18) ^ (x >> 3);
}

/**
 * sigma1 as defined in FIPS 180-2
 * @param x
 * @return calculated result
 */
static inline uint32_t
sigma1(uint32_t x)
{
	rotate_right(x, 17) ^ rotate_right(x, 19) ^ (x >> 10);
}
/**
 * Performs a right rotation of x with y steps
 * @param x the variable to be rotated
 * @param y the number of steps it should rotate
 */
static inline uint32_t
rotate_right(uint32_t x, uint32_t y)
{
	return ((x) >> (y)) | ((x) << (32-y));
}
