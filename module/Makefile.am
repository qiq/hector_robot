SUBDIRS = perl java python

AM_CXXFLAGS = @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I$(top_srcdir)/resource -I$(top_srcdir)/lib
AM_LDFLAGS = @LIBXML2_LIBS@ @LOG4CXX_LIBS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@ -module -avoid-version
AM_LIBTOOLFLAGS = --tag=disable-static

BUILT_SOURCES =
EXTRA_DIST =

moduledir = $(libdir)/hector/module
module_LTLIBRARIES = 

module_LTLIBRARIES += PageResourceGenerator.la
PageResourceGenerator_la_SOURCES = PageResourceGenerator.h PageResourceGenerator.cc
PageResourceGenerator_la_LIBADD = ../resource/libPageResource.la

#Filter_la_SOURCES = Filter.h Filter.cc FilterLexer.h FilterLexer.cc
#Filter_la_CXXFLAGS = $(AM_CXXFLAGS) @LIBPCRECPP_CFLAGS@
#Filter_la_LDFLAGS = $(AM_LDFLAGS) @LIBPCRECPP_LIBS@
#Filter_la_LIBADD = ../resource/libWebResource.la ../lib/librobot.la
#FilterLexer.cc: FilterLexer.l
#	flex -8 -f -Cf -Ca -Cr -t FilterLexer.l >FilterLexer.cc

if FETCH_MODULE
module_LTLIBRARIES += Fetcher.la
Fetcher_la_SOURCES = Fetcher.h Fetcher.cc
Fetcher_la_CXXFLAGS = $(AM_CXXFLAGS) @LIBCURL_CPPFLAGS@
Fetcher_la_LDFLAGS = $(AM_LDFLAGS) -lev @LIBCURL@
Fetcher_la_LIBADD = ../resource/libPageResource.la $(top_builddir)/lib/librobot.la
endif

module_LTLIBRARIES += DnsResolver.la
DnsResolver_la_SOURCES = DnsResolver.h DnsResolver.cc
DnsResolver_la_CXXFLAGS = $(AM_CXXFLAGS)
DnsResolver_la_LDFLAGS = $(AM_LDFLAGS) -lunbound
DnsResolver_la_LIBADD = ../resource/libPageResource.la ../resource/libSiteResource.la $(top_builddir)/lib/librobot.la

#SiteManager_la_SOURCES = SiteManager.h SiteManager.cc
#SiteManager_la_LDFLAGS = $(AM_LDFLAGS) -lunbound
#SiteManager_la_LIBADD = ../resource/libPageResource.la ../resource/libSiteResource.la $(top_builddir)/lib/librobot.la @HECTOR_CORE_PREFIX@/lib/hector/resource/libMarkerResource.la

module_LTLIBRARIES += UrlExtractor.la
UrlExtractor_la_SOURCES = UrlExtractor.h UrlExtractor.cc UrlExtractorLexer.h UrlExtractorLexer.cc
UrlExtractor_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/lib/googleurl
UrlExtractor_la_LIBADD = ../resource/libPageResource.la ../resource/libUrlResource.la $(top_builddir)/lib/librobot.la $(top_builddir)/lib/libgoogleurl.la
UrlExtractorLexer.cc: UrlExtractorLexer.l
	$(LEX) -8 -f -Cf -Ca -Cr -i -t UrlExtractorLexer.l >UrlExtractorLexer.cc
BUILT_SOURCES += UrlExtractorLexer.cc
EXTRA_DIST += UrlExtractorLexer.l

module_LTLIBRARIES += ResourceTypeToStatus.la
ResourceTypeToStatus_la_SOURCES = ResourceTypeToStatus.h ResourceTypeToStatus.cc
ResourceTypeToStatus_la_LIBADD = $(top_builddir)/lib/librobot.la

module_LTLIBRARIES += UpdateSites.la
UpdateSites_la_SOURCES = UpdateSites.h UpdateSites.cc
UpdateSites_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libSiteResource.la @HECTOR_CORE_PREFIX@/lib/hector/resource/libMarkerResource.la

module_LTLIBRARIES += UpdateUrls.la
UpdateUrls_la_SOURCES = UpdateUrls.h UpdateUrls.cc
UpdateUrls_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libUrlResource.la ../resource/libPageResource.la @HECTOR_CORE_PREFIX@/lib/hector/resource/libMarkerResource.la

module_LTLIBRARIES += UpdateIndex.la
UpdateIndex_la_SOURCES = UpdateIndex.h UpdateIndex.cc
UpdateIndex_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libUrlResource.la ../resource/libPageResource.la ../resource/libIndexResource.la @HECTOR_CORE_PREFIX@/lib/hector/resource/libMarkerResource.la

if FEATURAMA_TAGGER_MODULE
module_LTLIBRARIES += FeaturamaTagger.la
FeaturamaTagger_la_CXXFLAGS = $(AM_CXXFLAGS) @FEATURAMA_CFLAGS@ @CZMORPHOLOGY_CFLAGS@
FeaturamaTagger_la_LDFLAGS = $(AM_LDFLAGS) @FEATURAMA_LIBS@ @CZMORPHOLOGY_LIBS@
FeaturamaTagger_la_SOURCES = FeaturamaTagger.h FeaturamaTagger.cc
FeaturamaTagger_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la -lunistring
endif

if TOKENIZER_MODULE
module_LTLIBRARIES += Tokenizer.la
Tokenizer_la_SOURCES = Tokenizer.h Tokenizer.cc
Tokenizer_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la -lunistring

module_LTLIBRARIES += Tokenizer_cs_CZ.la
Tokenizer_cs_CZ_la_SOURCES = Tokenizer_cs_CZ.cc
Tokenizer_cs_CZ_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la
endif

module_LTLIBRARIES += RawTextRead.la
RawTextRead_la_SOURCES = RawTextRead.h RawTextRead.cc
RawTextRead_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la

module_LTLIBRARIES += TextResourcePrint.la
TextResourcePrint_la_SOURCES = TextResourcePrint.h TextResourcePrint.cc
TextResourcePrint_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la

module_LTLIBRARIES += TextResourceRead.la
TextResourceRead_la_SOURCES = TextResourceRead.h TextResourceRead.cc
TextResourceRead_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la

module_LTLIBRARIES += Deduplicator.la
Deduplicator_la_SOURCES = Deduplicator.h Deduplicator.cc
Deduplicator_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la

module_LTLIBRARIES += WordCount.la
WordCount_la_SOURCES = WordCount.h WordCount.cc
WordCount_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la

module_LTLIBRARIES += MstParser.la
MstParser_la_SOURCES = MstParser.h MstParser.cc
MstParser_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la
