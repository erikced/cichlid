/*
 * Copyright © 2009 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_hash_md5.c
 *
 * Class for computing checksums according to the
 * RSA Data Security, Inc. MD5 Message-Digest Algorithm
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
#include "cichlid_hash_md5.h"

#define BUFFER_SIZE 1024*512

enum
{
	READING = 0,
	BIT_APPENDED,
	SIZE_APPENDED
};

static void            cichlid_hash_md5_calc (CichlidHashMd5 *self, const char *buf, size_t bytes_read);
static gboolean        cichlid_hash_md5_equals (uint32_t* a, uint32_t* b);
static inline void     cichlid_hash_md5_flip_endianness (uint32_t *destination, uint32_t *source);
static uint32_t       *cichlid_hash_md5_get_hash (CichlidHash *object);
static char           *cichlid_hash_md5_get_hash_string (CichlidHash *object);
static inline uint32_t cichlid_hash_md5_rotate_left (uint32_t x, uint32_t y);
static void            cichlid_hash_md5_update (CichlidHash *object, const char *data, size_t data_size);
static void            cichlid_hash_interface_init (CichlidHashInterface *iface);

static const uint32_t shift_lookup_table[64] = {
		0x07, 0x0C, 0x11, 0x16,
		0x07, 0x0C, 0x11, 0x16,
		0x07, 0x0C, 0x11, 0x16,
		0x07, 0x0C, 0x11, 0x16,
		0x05, 0x09, 0x0E, 0x14,
		0x05, 0x09, 0x0E, 0x14,
		0x05, 0x09, 0x0E, 0x14,
		0x05, 0x09, 0x0E, 0x14,
		0x04, 0x0B, 0x10, 0x17,
		0x04, 0x0B, 0x10, 0x17,
		0x04, 0x0B, 0x10, 0x17,
		0x04, 0x0B, 0x10, 0x17,
		0x06, 0x0A, 0x0F, 0x15,
		0x06, 0x0A, 0x0F, 0x15,
		0x06, 0x0A, 0x0F, 0x15,
		0x06, 0x0A, 0x0F, 0x15
};

static const uint32_t shift_angle_table[64] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x4881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

G_DEFINE_TYPE_WITH_CODE(CichlidHashMd5, cichlid_hash_md5, G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(CICHLID_TYPE_HASH, cichlid_hash_interface_init));

/*
 * Initialize the interface
 */
static void
cichlid_hash_interface_init(CichlidHashInterface *iface)
{
	iface->equals = cichlid_hash_md5_equals;
	iface->get_hash = cichlid_hash_md5_get_hash;
	iface->get_hash_string = cichlid_hash_md5_get_hash_string;
	iface->update = cichlid_hash_md5_update;

}

static void
cichlid_hash_md5_dispose(GObject *gobject)
{
	CichlidHashMd5 *self = CICHLID_HASH_MD5 (gobject);

	/* Chain up to the parent class */
	G_OBJECT_CLASS (cichlid_hash_md5_parent_class)->dispose (gobject);
}

static void
cichlid_hash_md5_finalize(GObject *gobject)
{
	CichlidHashMd5 *self = CICHLID_HASH_MD5 (gobject);

	G_OBJECT_CLASS (cichlid_hash_md5_parent_class)->finalize (gobject);
}


static void
cichlid_hash_md5_class_init(CichlidHashMd5Class *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->dispose = cichlid_hash_md5_dispose;
	gobject_class->finalize = cichlid_hash_md5_finalize;

	/* Initiera funktioner */
}


static void
cichlid_hash_md5_init(CichlidHashMd5 *self)
{
	/* Partial result variables */
	self->p[0] = 0x67452301;
	self->p[1] = 0xEFCDAB89;
	self->p[2] = 0x98BADCFE;
	self->p[3] = 0x10325476;

	self->hash_computed = FALSE;
	self->total_size = 0;
	self->data_left_size = 0;
}

CichlidHash *
cichlid_hash_md5_new()
{
	CichlidHashMd5 *_ccalc;
	_ccalc = g_object_new(CICHLID_TYPE_HASH_MD5,NULL);

	return CICHLID_HASH(_ccalc);
}

static gboolean
cichlid_hash_md5_equals(uint32_t* a, uint32_t* b)
{
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static void
cichlid_hash_md5_update(CichlidHash *object, const char *data, size_t data_size)
{
	CichlidHashMd5 *self;
	char *buf = NULL;
	uint8_t new_data_left_size;

	self = CICHLID_HASH_MD5(object);

	if (self->hash_computed)
		cichlid_hash_md5_init(self);

	if (!data_size)
		return;

	self->total_size += data_size;

	/* If there is no data left since the previous update */
	if (!self->data_left_size) {
		self->data_left_size = data_size % 64;
		memcpy(self->data_left, data + data_size - self->data_left_size, self->data_left_size);

		cichlid_hash_md5_calc(self, data, data_size - self->data_left_size);
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

		cichlid_hash_md5_calc(self, buf, data_size + self->data_left_size - new_data_left_size);
		self->data_left_size = new_data_left_size;

		g_free(buf);
	}
}

/*
 * Returns the resulting hash as an array of integers
 * @param object
 * @return an array of for uint32_t* if successful, else NULL.
 */
static uint32_t *
cichlid_hash_md5_get_hash(CichlidHash *object)
{
	CichlidHashMd5	*self;
	char 			*buf;
	goffset 		size_offset;
	uint32_t			*hash;

	self = CICHLID_HASH_MD5(object);
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

		/* Copy the data that is left, the ending 1 and the size to the buffer */
		memcpy(buf, self->data_left, self->data_left_size);
		buf[self->data_left_size] = -0x80; /* Signed char */
		self->total_size = self->total_size * 8; /* Convert size to bits */
		memcpy(buf + size_offset, &self->total_size, 8);

		cichlid_hash_md5_calc(self, buf, size_offset + 8);

		g_free(buf);
		self->hash_computed = TRUE;
	}

	/* Copy the hash and switch endianness */
	hash = g_malloc(sizeof(uint32_t)*4);
	cichlid_hash_md5_flip_endianness(hash, self->p);

	return hash;
}

/*
 * Returns the resulting hash as a byte string
 * @param object
 * @return a NULL-terminated string with the checksum if successful, else NULL.
 */
static char *
cichlid_hash_md5_get_hash_string(CichlidHash *object)
{
	char *hash_string;
	uint32_t *hash;

	hash = cichlid_hash_md5_get_hash(object);
	if (hash == NULL)
		return NULL;

	hash_string = g_malloc(sizeof(char)*33);
	sprintf(hash_string, "%.8x%.8x%.8x%.8x", hash[0], hash[1], hash[2], hash[3]);

	g_free(hash);
	return hash_string;
}

/*
 * Calculates the partial checksum for buf
 * @param buf data buffer
 * @param bytes_read size of data buffer
 */
static void
cichlid_hash_md5_calc(CichlidHashMd5 *self, const char *buf, size_t bytes_read)
{
	uint32_t a, b, c, d, f, g, tmp, *w;

	/* Split buf into 512-bit chunks and process them */
	for (int j = 0; j < bytes_read/64; j++)
	{
		/* Set w to start at the proper place */
		w = (uint32_t*)(buf + j*64);
		/* Initialize with the current values */
		a = self->p[0];
		b = self->p[1];
		c = self->p[2];
		d = self->p[3];

		/* Calculate */
		for (int i = 0; i < 64; i++)
		{
			if (i < 16)
			{
				f = (b & c) | ((~b) & d);
				g = i;
			}
			else if (i < 32)
			{
				f = (b & d) | (c & (~d));
				g = (5*i + 1) % 16;
			}
			else if (i < 48)
			{
				f = b ^ c ^ d;
				g = (3*i + 5) % 16;
			}
			else
			{
				f = c ^ (b | (~d));
				g = (7*i) % 16;
			}

			tmp = d;
			d = c;
			c = b;
			b = b + cichlid_hash_md5_rotate_left (a + f + shift_angle_table[i] + w[g], shift_lookup_table[i]);
			a = tmp;
		}

		/* Add this chunks result to the total result */
		self->p[0] += a;
		self->p[1] += b;
		self->p[2] += c;
		self->p[3] += d;
	}
}

/**
 * Performs a left rotation of x with y steps
 * @param x the variable to be rotated
 * @param y the number of steps it should rotate
 */
static inline uint32_t
cichlid_hash_md5_rotate_left(uint32_t x, uint32_t y)
{
	return ((x) << (y)) | ((x) >> (32-y));
}

/**
 * Flips the endianness of 4 32-bit integers in source and saves it in destination
 * @param destination
 * @param source
 */
static inline void
cichlid_hash_md5_flip_endianness(uint32_t *destination, uint32_t *source)
{
	for (int i = 0; i < 4; i++)
		destination[i] = ((source[i] & 0xFF) << 24) |
		((source[i] & 0xFF00) << 8) |
		((source[i] & 0xFF0000) >> 8) |
		((source[i] & 0xFF000000) >> 24);
}
