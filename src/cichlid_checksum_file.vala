/*
 * Copyright © 2008-2010 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_checksum_file.vala
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

enum HashType {
	UNKNOWN,
	CRC32,
	MD5,
	SHA1,
	SHA224,
	SHA256,
	SHA384,
	SHA512
}

public Cichlid.ChecksumFile : Gtk.ListStore {
	private Cichlid.Verifier *verifier;
	private HashType cs_type;

	/* Signals */
	public signal void file_loaded();
	public signal void verification_progress_update(double progress, double speed);
	public signal void verification_complete();
	
	private void on_verifier_progress_update(double progress, double speed, Cichlid.Verifier verifier) {
		
	}

	public ChecksumFile() {
	}
}

