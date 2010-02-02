/*
 * Copyright © 2010 Erik Cederberg <erikced@gmail.com>
 *
 * cichlid - cichlid_main_window.vala
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
	enum Buttons {
		B_ALL = 0,
		B_OK,
		B_CORRUPT,
		B_MISSING,
		N_BTNS
	}
	
	public class MainWindow : GLib.Object {
/*	const string MAIN_UI_SYSTEM =PKGDATADIR"/cichlid.ui"; */
		public static const  string MAIN_UI_WIN32 = "cichlid.ui";
		public static const string MAIN_UI_SOURCE = "data/cichlid.ui";
		
		Gtk.Widget main_window;
		Gtk.Widget filelist;
		Gtk.Widget tn_cancel;
		Gtk.Widget btn_verify;
/*	Gtk.Widget btn_filter[Buttons.N_BTNS];*/
		Gtk.Widget file_menu_open;
		Gtk.Widget file_menu_quit;
		Gtk.Widget about;
		Gtk.Widget rogress_hbox;
		Gtk.Widget progress;
		Gtk.Widget progress_lbl;
		Gtk.Widget search_box;
		
		public MainWindow() {
			try {
				var builder = new Gtk.Builder();
				builder.add_from_file(MAIN_UI_SOURCE);
				this.main_window = builder.get_object("main_window") as Gtk.Window;
			} catch (Error e) {
				stderr.printf ("Could not load UI: %s\n", e.message);
			} 
		}

		public void show_all() {
			this.main_window.show_all();
		}
	}
}