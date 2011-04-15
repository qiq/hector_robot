AM_CXXFLAGS = @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I$(top_srcdir)/resources
AM_LDFLAGS = @LIBXML2_LIBS@ @LOG4CXX_LIBS@ @HECTOR_CORE_LIBS@

lib_LTLIBRARIES = librobot.la
noinst_LTLIBRARIES = libgoogleurl.la

hectorincludedir = $(includedir)/hector
hectorinclude_HEADERS = robot_common.h SuperFastHash.h \
	robot_common.i

librobot_la_SOURCES = \
	robot_common.h robot_common.cc SuperFastHash.h
librobot_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/lib/googleurl
librobot_la_LIBADD = libgoogleurl.la
librobot_la_LDFLAGS = -version-info 0:0:0 $(AM_LDFLAGS) -lltdl

libgoogleurl_la_SOURCES = googleurl/src/gurl.h googleurl/src/url_canon.h googleurl/src/url_canon_icu.h googleurl/src/url_canon_internal.h googleurl/src/url_canon_internal_file.h googleurl/src/url_canon_ip.h googleurl/src/url_canon_stdstring.h googleurl/src/url_common.h googleurl/src/url_file.h googleurl/src/url_parse.h googleurl/src/url_parse_internal.h googleurl/src/url_util.h googleurl/src/gurl.cc googleurl/src/url_canon_etc.cc googleurl/src/url_canon_fileurl.cc googleurl/src/url_canon_host.cc googleurl/src/url_canon_icu.cc googleurl/src/url_canon_internal.cc googleurl/src/url_canon_ip.cc googleurl/src/url_canon_mailtourl.cc googleurl/src/url_canon_path.cc googleurl/src/url_canon_pathurl.cc googleurl/src/url_canon_query.cc googleurl/src/url_canon_relative.cc googleurl/src/url_canon_stdurl.cc googleurl/src/url_parse.cc googleurl/src/url_parse_file.cc googleurl/src/url_util.cc googleurl/base/basictypes.h googleurl/base/logging.h googleurl/base/scoped_ptr.h googleurl/base/string16.h googleurl/base/string16.cc
libgoogleurl_la_CXXFLAGS = @ICU_CXXFLAGS@ -I$(top_srcdir)/lib/googleurl -Wno-deprecated -DNDEBUG -fPIC
libgoogleurl_la_LDFLAGS = @ICU_LIBS@
libgoogleurl_la_LIBTOOLFLAGS = --tag=disable-shared

#bin_PROGRAMS = test
#test_SOURCES = \
#	test.cc
#test_CXXFLAGS = -I$(top_srcdir)/lib/googleurl
#test_LDFLAGS = -lpthread
#test_LDADD = libgoogleurl.la
