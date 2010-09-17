SUBDIRS = perl java python

AM_CXXFLAGS = @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I$(top_srcdir)/resources
AM_LDFLAGS = @LIBXML2_LIBS@ @LOG4CXX_LIBS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@ -module -avoid-version
AM_LIBTOOLFLAGS = --tag=disable-static

modulesdir = $(libdir)/hector/modules
modules_LTLIBRARIES = GenerateWebResource.la

GenerateWebResource_la_SOURCES = GenerateWebResource.h GenerateWebResource.cc
GenerateWebResource_la_LIBADD = ../resources/libWebResource.la

lib_LTLIBRARIES = libgoogleurl.la

libgoogleurl_la_SOURCES = googleurl/src/gurl.h googleurl/src/url_canon.h googleurl/src/url_canon_icu.h googleurl/src/url_canon_internal.h googleurl/src/url_canon_internal_file.h googleurl/src/url_canon_ip.h googleurl/src/url_canon_stdstring.h googleurl/src/url_common.h googleurl/src/url_file.h googleurl/src/url_parse.h googleurl/src/url_parse_internal.h googleurl/src/url_util.h googleurl/src/gurl.cc googleurl/src/url_canon_etc.cc googleurl/src/url_canon_fileurl.cc googleurl/src/url_canon_host.cc googleurl/src/url_canon_icu.cc googleurl/src/url_canon_internal.cc googleurl/src/url_canon_ip.cc googleurl/src/url_canon_mailtourl.cc googleurl/src/url_canon_path.cc googleurl/src/url_canon_pathurl.cc googleurl/src/url_canon_query.cc googleurl/src/url_canon_relative.cc googleurl/src/url_canon_stdurl.cc googleurl/src/url_parse.cc googleurl/src/url_parse_file.cc googleurl/src/url_util.cc googleurl/base/string16.cc
libgoogleurl_la_CXXFLAGS = $(AM_CXXFLAGS) -I googleurl
libgoogleurl_la_LDFLAGS = -version-info 0:0:0 $(AM_LDFLAGS)
