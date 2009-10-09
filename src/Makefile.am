AM_CFLAGS = -g -pedantic -Wall -std=c99

bin_PROGRAMS = cichlid

cichlid_SOURCES = \
	cichlid.c \
	cichlid.h \
	cichlid_file.h \
	cichlid_file.c \
	cichlid_cell_renderer.c \
	cichlid_cell_renderer.h \
	cichlid_checksum_file.c \
	cichlid_checksum_file.h \
	cichlid_checksum_file_v.c \
	cichlid_checksum_file_v.h \
	cichlid_hash.c \
	cichlid_hash.h \
	cichlid_hash_common.h \
	cichlid_hash_crc32.c \
	cichlid_hash_crc32.h \
	cichlid_hash_md5.c \
	cichlid_hash_md5.h \
	cichlid_hash_sha256.c \
	cichlid_hash_sha256.h \
	cichlid_main_window.c \
	cichlid_main_window.h	

cichlid_CFLAGS = \
	-DPKGDATADIR=\""$(pkgdatadir)"\" \
	$(GLIB_CFLAGS) \
	$(GTHREAD_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GIO_CFLAGS) \
	$(AM_CFLAGS)

cichlid_LDADD = \
	$(GLIB_LIBS) \
	$(GTHREAD_LIBS) \
	$(GTK_LIBS) \
	$(GIO_LIBS)
