3/*
 * Copyright © 2010 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid.vala
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
	public class Cichlid : GLib.Object {
		public File current_file;
		
		public static int main(string[] args) {
			string filename;
			Cichlid ck;
			
			Gtk.init(ref args);
			
			if (args.length > 1) {
				filename = args[1];
				ck = new Cichlid.with_file(filename);
			} else {
				ck = new Cichlid();
			}
			MainWindow m = new MainWindow();
			m.show_all();
			Gtk.main();
			
			return 0;
		}
		
		public Cichlid() {
			stdout.printf("Ät en fisk\n");
		}
		
		public Cichlid.with_file(string filename) {
//		this.current_file = filename;
//		Cichlid.ChecksumFile.load_from_cmd(filename);
			this();
		}
	}
}