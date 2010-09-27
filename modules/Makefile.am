SUBDIRS = perl java python

AM_CXXFLAGS = @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I$(top_srcdir)/resources -I$(top_srcdir)/lib
AM_LDFLAGS = @LIBXML2_LIBS@ @LOG4CXX_LIBS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@ -module -avoid-version
AM_LIBTOOLFLAGS = --tag=disable-static

modulesdir = $(libdir)/hector/modules
modules_LTLIBRARIES = GenerateWebResource.la ParseUrl.la Filter.la

GenerateWebResource_la_SOURCES = GenerateWebResource.h GenerateWebResource.cc
GenerateWebResource_la_LIBADD = ../resources/libWebResource.la

ParseUrl_la_SOURCES = ParseUrl.h ParseUrl.cc
ParseUrl_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/lib/googleurl
ParseUrl_la_LIBADD = ../resources/libWebResource.la ../lib/librobot.la

Filter_la_SOURCES = Filter.h Filter.cc
Filter_la_CXXFLAGS = $(AM_CXXFLAGS) @LIBPCRECPP_CFLAGS@
Filter_la_LDFLAGS = $(AM_LDFLAGS) @LIBPCRECPP_LIBS@
Filter_la_LIBADD = ../resources/libWebResource.la
