/*
 * Copyright © 2010 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_file.vala
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

	public enum Status {
		BAD,
		GOOD,
		NOT_FOUND,
		NOT_VERIFIED
	}
	
	public class File : GLib.Object {
		private GLib.File? _file;
		private string?    _name;
		private string?    _hash;

		public File (GLib.File f, string hash) {
			_hash = hash;
			file = f;
		}

		public GLib.File file {
			set { set_file (file); }
			get {
				GLib.assert(_file != null);
				return _file; }
		}
				
		private unowned string get_status_string () {
			string status;

			switch (_status) {
			case Status.GOOD:
				status = "Ok";
				break;
			case Status.BAD:
				status = "Corrupt";
				break;
			case Status.NOT_VERIFIED:
				status = "Not verified";
				break;
			case Status.NOT_FOUND:
				status = "Missing";
				break;
			default:
				GLib.assert_not_reached();
			}
			
			return status;
		}

		private void set_file (GLib.File file) {
		}

		public Status status { get; set; default = Status.NOT_VERIFIED; }

		public string status_string {
			get { return get_status_string (); }
		}
	}
}