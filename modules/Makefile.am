SUBDIRS = perl java python

AM_CXXFLAGS = @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I$(top_srcdir)/resources -I$(top_srcdir)/lib
AM_LDFLAGS = @LIBXML2_LIBS@ @LOG4CXX_LIBS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@ -module -avoid-version
AM_LIBTOOLFLAGS = --tag=disable-static

modulesdir = $(libdir)/hector/modules
modules_LTLIBRARIES = WebResourceGenerator.la UrlParser.la UrlComposer.la Fetcher.la DnsResolver.la WebSiteManager.la #Filter.la

WebResourceGenerator_la_SOURCES = WebResourceGenerator.h WebResourceGenerator.cc
WebResourceGenerator_la_LIBADD = ../resources/libWebResource.la

UrlParser_la_SOURCES = UrlParser.h UrlParser.cc
UrlParser_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/lib/googleurl
UrlParser_la_LIBADD = ../resources/libWebResource.la ../lib/librobot.la

UrlComposer_la_SOURCES = UrlComposer.h UrlComposer.cc
UrlComposer_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/lib/googleurl
UrlComposer_la_LIBADD = ../resources/libWebResource.la ../lib/librobot.la

#Filter_la_SOURCES = Filter.h Filter.cc
#Filter_la_CXXFLAGS = $(AM_CXXFLAGS) @LIBPCRECPP_CFLAGS@
#Filter_la_LDFLAGS = $(AM_LDFLAGS) @LIBPCRECPP_LIBS@
#Filter_la_LIBADD = ../resources/libWebResource.la ../lib/librobot.la

Fetcher_la_SOURCES = Fetcher.h Fetcher.cc
Fetcher_la_CXXFLAGS = $(AM_CXXFLAGS) -I ../lib/curl/include
Fetcher_la_LDFLAGS = $(AM_LDFLAGS) -lev
Fetcher_la_LIBADD = ../resources/libWebResource.la ../lib/librobot.la ../lib/curl/lib/libcurl.la

DnsResolver_la_SOURCES = DnsResolver.h DnsResolver.cc
DnsResolver_la_CXXFLAGS = $(AM_CXXFLAGS)
DnsResolver_la_LDFLAGS = $(AM_LDFLAGS) -lunbound
DnsResolver_la_LIBADD = ../resources/libWebResource.la ../lib/librobot.la ../lib/curl/lib/libcurl.la

WebSiteManager_la_SOURCES = WebSiteManager.h WebSiteManager.cc
WebSiteManager_la_CXXFLAGS = $(AM_CXXFLAGS)
WebSiteManager_la_LDFLAGS = $(AM_LDFLAGS) -lunbound
WebSiteManager_la_LIBADD = ../resources/libWebResource.la ../resources/libWebSiteResource.la ../lib/librobot.la ../lib/curl/lib/libcurl.la
