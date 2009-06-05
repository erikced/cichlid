AM_CFLAGS = -g -pedantic -Wall -std=c99

bin_PROGRAMS = cichlid

cichlid_SOURCES = \
	cichlid.c \
	cichlid.h \
	checksum_file.c \
	checksum_file.h \
	gui.c \
	gui.h \
	verification.c \
	verification.h \
	cichlid_hash.c \
	cichlid_hash.h \
	cichlid_hash_crc32.c \
	cichlid_hash_crc32.h \
	cichlid_hash_md5.c \
	cichlid_hash_md5.h \
	CichlidFile.c \
	CichlidFile.h

cichlid_CFLAGS = \
	-DPKGDATADIR=\""$(pkgdatadir)"\" \
	$(GLIB_CFLAGS) \
	$(UI_CFLAGS) \
	$(AM_CFLAGS)

cichlid_LDADD = \
	$(GLIB_LIBS) \
	$(UI_LIBS)
