/*
 * Copyright © 2010 Erik Cederberg <erikced@gmail.com>
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

namespace Cichlid {

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
	
	public class ChecksumFile : Gtk.ListStore, Gtk.TreeModel {
		private Verifier m_verifier;
		private HashType m_cs_type;
		private GLib.File m_file;
		
		/* Signals */
		public signal void file_loaded();
		public signal void verification_progress_update( double progress, double speed );
		public signal void verification_complete();
		
		private void on_verifier_progress_update( double progress, double speed, Verifier verifier ) {
			
		}
		
		public ChecksumFile (GLib.File file) {
			GLib.Type[] types = { typeof(GLib.Object) };
			set_column_types( types );
			m_file = file;
			GLib.Idle.add (load_file, Priority.DEFAULT_IDLE);
		}

		private bool load_file () {
			stdout.printf("Load File...\n");
			return false;
		}

		public GLib.Type get_column_type (int column) {
			GLib.Type[] types = { typeof(string), typeof(int), typeof(int) };
			return types[column];
		}

		private int get_object (Gtk.TreeIter iter) {
			GLib.Value value;
			base.get_value (iter, 1, out value);
			return 1;
		}

		public void get_value (Gtk.TreeIter iter, int column, out GLib.Value value) {
			value.init (get_column_type(column));

			switch (column) {
			case 0:
				value.set_string ("Lala");
				break;
			case 1:
				value.set_int (1);
				break;
			default:
				GLib.assert_not_reached ();
			}
		}
	}
	
}