SUBDIRS = perl java python

AM_CXXFLAGS = @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I$(top_srcdir)/resources -I$(top_srcdir)/lib
AM_LDFLAGS = @LIBXML2_LIBS@ @LOG4CXX_LIBS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@ -module -avoid-version
AM_LIBTOOLFLAGS = --tag=disable-static

BUILT_SOURCES =
EXTRA_DIST =

modulesdir = $(libdir)/hector/modules
modules_LTLIBRARIES = PageResourceGenerator.la Fetcher.la DnsResolver.la SiteManager.la UrlExtractor.la ResourceTypeToStatus.la UpdateSites.la UpdateUrls.la FeaturamaTagger.la Tokenizer.la Tokenizer_cs_CZ.la
#Filter.la

PageResourceGenerator_la_SOURCES = PageResourceGenerator.h PageResourceGenerator.cc
PageResourceGenerator_la_LIBADD = ../resources/libPageResource.la

#Filter_la_SOURCES = Filter.h Filter.cc FilterLexer.h FilterLexer.cc
#Filter_la_CXXFLAGS = $(AM_CXXFLAGS) @LIBPCRECPP_CFLAGS@
#Filter_la_LDFLAGS = $(AM_LDFLAGS) @LIBPCRECPP_LIBS@
#Filter_la_LIBADD = ../resources/libWebResource.la ../lib/librobot.la
#FilterLexer.cc: FilterLexer.l
#	flex -8 -f -Cf -Ca -Cr -t FilterLexer.l >FilterLexer.cc

Fetcher_la_SOURCES = Fetcher.h Fetcher.cc
Fetcher_la_CXXFLAGS = $(AM_CXXFLAGS) @LIBCURL_CPPFLAGS@
Fetcher_la_LDFLAGS = $(AM_LDFLAGS) -lev @LIBCURL@
Fetcher_la_LIBADD = ../resources/libPageResource.la $(top_builddir)/lib/librobot.la

DnsResolver_la_SOURCES = DnsResolver.h DnsResolver.cc
DnsResolver_la_CXXFLAGS = $(AM_CXXFLAGS)
DnsResolver_la_LDFLAGS = $(AM_LDFLAGS) -lunbound
DnsResolver_la_LIBADD = ../resources/libPageResource.la ../resources/libSiteResource.la $(top_builddir)/lib/librobot.la

SiteManager_la_SOURCES = SiteManager.h SiteManager.cc
SiteManager_la_LDFLAGS = $(AM_LDFLAGS) -lunbound
SiteManager_la_LIBADD = ../resources/libPageResource.la ../resources/libSiteResource.la $(top_builddir)/lib/librobot.la @HECTOR_CORE_PREFIX@/lib/hector/resources/libMarkerResource.la

UrlExtractor_la_SOURCES = UrlExtractor.h UrlExtractor.cc UrlExtractorLexer.h UrlExtractorLexer.cc
UrlExtractor_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/lib/googleurl
UrlExtractor_la_LIBADD = ../resources/libPageResource.la $(top_builddir)/lib/librobot.la $(top_builddir)/lib/libgoogleurl.la
UrlExtractorLexer.cc: UrlExtractorLexer.l
	$(LEX) -8 -f -Cf -Ca -Cr -i -t UrlExtractorLexer.l >UrlExtractorLexer.cc
BUILT_SOURCES += UrlExtractorLexer.cc
EXTRA_DIST += UrlExtractorLexer.l

ResourceTypeToStatus_la_SOURCES = ResourceTypeToStatus.h ResourceTypeToStatus.cc
ResourceTypeToStatus_la_LIBADD = $(top_builddir)/lib/librobot.la

UpdateSites_la_SOURCES = UpdateSites.h UpdateSites.cc
UpdateSites_la_LIBADD = $(top_builddir)/lib/librobot.la ../resources/libSiteResource.la @HECTOR_CORE_PREFIX@/lib/hector/resources/libMarkerResource.la

UpdateUrls_la_SOURCES = UpdateUrls.h UpdateUrls.cc
UpdateUrls_la_LIBADD = $(top_builddir)/lib/librobot.la ../resources/libUrlResource.la ../resources/libPageResource.la @HECTOR_CORE_PREFIX@/lib/hector/resources/libMarkerResource.la

FeaturamaTagger_la_CXXFLAGS = $(AM_CXXFLAGS) @FEATURAMA_CFLAGS@ @CZMORPHOLOGY_CFLAGS@
FeaturamaTagger_la_LDFLAGS = $(AM_LDFLAGS) @FEATURAMA_LIBS@ @CZMORPHOLOGY_LIBS@
FeaturamaTagger_la_SOURCES = FeaturamaTagger.h FeaturamaTagger.cc
FeaturamaTagger_la_LIBADD = $(top_builddir)/lib/librobot.la ../resources/libTextResource.la

Tokenizer_la_SOURCES = Tokenizer.h Tokenizer.cc
Tokenizer_la_LIBADD = $(top_builddir)/lib/librobot.la ../resources/libTextResource.la

Tokenizer_cs_CZ_la_SOURCES = Tokenizer_cs_CZ.cc
