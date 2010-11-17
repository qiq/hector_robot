SUBDIRS = perl java python

AM_CXXFLAGS = @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I$(top_srcdir)/resources -I$(top_srcdir)/lib
AM_LDFLAGS = @LIBXML2_LIBS@ @LOG4CXX_LIBS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@ -module -avoid-version
AM_LIBTOOLFLAGS = --tag=disable-static

modulesdir = $(libdir)/hector/modules
modules_LTLIBRARIES = GenerateWebResource.la ParseUrl.la ComposeUrl.la Fetch.la ResolveDns.la WebSiteManager.la #Filter.la

GenerateWebResource_la_SOURCES = GenerateWebResource.h GenerateWebResource.cc
GenerateWebResource_la_LIBADD = ../resources/libWebResource.la

ParseUrl_la_SOURCES = ParseUrl.h ParseUrl.cc
ParseUrl_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/lib/googleurl
ParseUrl_la_LIBADD = ../resources/libWebResource.la ../lib/librobot.la

ComposeUrl_la_SOURCES = ComposeUrl.h ComposeUrl.cc
ComposeUrl_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/lib/googleurl
ComposeUrl_la_LIBADD = ../resources/libWebResource.la ../lib/librobot.la

#Filter_la_SOURCES = Filter.h Filter.cc
#Filter_la_CXXFLAGS = $(AM_CXXFLAGS) @LIBPCRECPP_CFLAGS@
#Filter_la_LDFLAGS = $(AM_LDFLAGS) @LIBPCRECPP_LIBS@
#Filter_la_LIBADD = ../resources/libWebResource.la ../lib/librobot.la

Fetch_la_SOURCES = Fetch.h Fetch.cc
Fetch_la_CXXFLAGS = $(AM_CXXFLAGS) -I ../lib/curl/include
Fetch_la_LDFLAGS = $(AM_LDFLAGS) -lev
Fetch_la_LIBADD = ../resources/libWebResource.la ../lib/librobot.la ../lib/curl/lib/libcurl.la

ResolveDns_la_SOURCES = ResolveDns.h ResolveDns.cc
ResolveDns_la_CXXFLAGS = $(AM_CXXFLAGS)
ResolveDns_la_LDFLAGS = $(AM_LDFLAGS) -lunbound
ResolveDns_la_LIBADD = ../resources/libWebResource.la ../lib/librobot.la ../lib/curl/lib/libcurl.la

WebSiteManager_la_SOURCES = WebSiteManager.h WebSiteManager.cc
WebSiteManager_la_CXXFLAGS = $(AM_CXXFLAGS)
WebSiteManager_la_LDFLAGS = $(AM_LDFLAGS) -lunbound
WebSiteManager_la_LIBADD = ../resources/libWebResource.la ../lib/librobot.la ../lib/curl/lib/libcurl.la
