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

	enum FilterButton {
	ALL = 0,
	OK,
	CORRUPT,
	MISSING,
	NUM
}
	public class MainWindow : GLib.Object {
/*	const string MAIN_UI_SYSTEM =PKGDATADIR"/cichlid.ui"; */
		public static const  string MAIN_UI_WIN32 = "cichlid.ui";
		public static const string MAIN_UI_SOURCE = "data/cichlid.ui";
		
		Gtk.Window    main_window;
		Gtk.TreeView  filelist;
		Gtk.Button    btn_cancel;
		Gtk.Button    btn_verify;
		Gtk.Button[]  btn_filter;
		Gtk.Action    file_menu_open;
		Gtk.Action    file_menu_quit;
		Gtk.Action    about;
		Gtk.HBox      progress_hbox;
		Gtk.ProgressBar    progress;
		Gtk.Widget    progress_lbl;
		Gtk.Entry     search_box;
		
		public MainWindow () {
			Gtk.Builder builder;
			try {
				builder = new Gtk.Builder ();
				builder.add_from_file (MAIN_UI_SOURCE);
				this.main_window = builder.get_object ("main_window") as Gtk.Window;
			} catch (Error e) {
				stderr.printf ("Could not load UI: %s\n", e.message);
			}

			/* Main window */
			main_window = builder.get_object ("main_window") as Gtk.Window;
			main_window.destroy.connect (Gtk.main_quit);

			/* Progress hbox */
			progress_hbox = builder.get_object ("hbox_progess") as Gtk.HBox;
			
			/* Progress bar */
			progress = builder.get_object ("pb_progress") as Gtk.ProgressBar;

			/* Progress label */
			progress_lbl = builder.get_object ("lbl_progess") as Gtk.Widget;

			/* File list */
			filelist = builder.get_object ("tv_files") as Gtk.TreeView;

			/* Filter buttons */
			btn_filter = new Gtk.Button[FilterButton.NUM];
			btn_filter[FilterButton.ALL] = builder.get_object ("btn_all") as Gtk.Button;

			btn_filter[FilterButton.OK] = builder.get_object ("btn_ok") as Gtk.Button;

			btn_filter[FilterButton.CORRUPT] = builder.get_object ("btn_corrupt") as Gtk.Button;

			btn_filter[FilterButton.MISSING] = builder.get_object ("btn_missing") as Gtk.Button;

			/* Search box */
			search_box = builder.get_object ("search_box") as Gtk.Entry;
			search_box.set_icon_activatable(Gtk.EntryIconPosition.PRIMARY, false);

			/* Verify button */
			btn_verify = builder.get_object ("btn_verify") as Gtk.Button;
			btn_verify.clicked.connect (on_verify_clicked);

			/* Cancel button */
			btn_cancel = builder.get_object ("btn_cancel") as Gtk.Button;

			/* Menu */
			file_menu_open = builder.get_object ("file_menu_open") as Gtk.Action;			
			file_menu_quit = builder.get_object ("file_menu_quit") as Gtk.Action;
			file_menu_quit.activate.connect (() => {
					Gtk.main_quit();
				});
			about = builder.get_object ("about") as Gtk.Action;
		}

		public void show_all () {
			this.main_window.show_all ();
		}

		private void on_verification_complete () {
			progress_hbox.hide ();			
			file_menu_open.set_sensitive (true);
			btn_cancel.hide ();
			btn_verify.show ();
		}
		
		private void on_verify_clicked () {
			btn_verify.hide ();
			btn_cancel.show ();
			file_menu_open.set_sensitive (false);
			progress_hbox.show ();
		}
		
	}
}