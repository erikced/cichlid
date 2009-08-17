/*
 * Copyright © 2008 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - checksum_file.h
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

#ifndef CHECKSUM_FILE_H
#define CHECKSUM_FILE_H


typedef enum
{
	first = 1,
	last
} checksum_position;



typedef struct
{
	GFile       *file;
	GList       *files;

	/* Checksum Options */
	const uint8_t     cs_length;
	const uint8_t     cs_order;
	checksum_position cs_position
	const char        cs_separator;
} CichlidChecksumFile;

void checksum_file_load (GFile* file);
gboolean checksum_file_load_init (char* filename);

#endif /* CHECKSUM_FILE_H */
