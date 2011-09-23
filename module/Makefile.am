SUBDIRS = perl java python

AM_CXXFLAGS = @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I$(top_srcdir)/resource -I$(top_srcdir)/lib
AM_LDFLAGS = @LIBXML2_LIBS@ @LOG4CXX_LIBS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@ -module -avoid-version
AM_LIBTOOLFLAGS = --tag=disable-static

BUILT_SOURCES =
EXTRA_DIST =

moduledir = $(libdir)/hector/module
module_LTLIBRARIES = 

module_LTLIBRARIES += GeneratePageResource.la
GeneratePageResource_la_SOURCES = GeneratePageResource.h GeneratePageResource.cc
GeneratePageResource_la_LIBADD = ../resource/libPageResource.la

#Filter_la_SOURCES = Filter.h Filter.cc FilterLexer.h FilterLexer.cc
#Filter_la_CXXFLAGS = $(AM_CXXFLAGS) @LIBPCRECPP_CFLAGS@
#Filter_la_LDFLAGS = $(AM_LDFLAGS) @LIBPCRECPP_LIBS@
#Filter_la_LIBADD = ../resource/libWebResource.la ../lib/librobot.la
#FilterLexer.cc: FilterLexer.l
#	flex -8 -f -Cf -Ca -Cr -t FilterLexer.l >FilterLexer.cc

if FETCH_MODULE
module_LTLIBRARIES += Fetch.la
Fetch_la_SOURCES = Fetch.h Fetch.cc
Fetch_la_CXXFLAGS = $(AM_CXXFLAGS) @LIBCURL_CPPFLAGS@
Fetch_la_LDFLAGS = $(AM_LDFLAGS) -lev @LIBCURL@
Fetch_la_LIBADD = ../resource/libPageResource.la $(top_builddir)/lib/librobot.la
endif

module_LTLIBRARIES += ResolveDns.la
ResolveDns_la_SOURCES = ResolveDns.h ResolveDns.cc
ResolveDns_la_CXXFLAGS = $(AM_CXXFLAGS)
ResolveDns_la_LDFLAGS = $(AM_LDFLAGS) -lunbound
ResolveDns_la_LIBADD = ../resource/libPageResource.la ../resource/libSiteResource.la $(top_builddir)/lib/librobot.la

#SiteManager_la_SOURCES = SiteManager.h SiteManager.cc
#SiteManager_la_LDFLAGS = $(AM_LDFLAGS) -lunbound
#SiteManager_la_LIBADD = ../resource/libPageResource.la ../resource/libSiteResource.la $(top_builddir)/lib/librobot.la @HECTOR_CORE_PREFIX@/lib/hector/resource/libMarkerResource.la

module_LTLIBRARIES += ExtractUrl.la
ExtractUrl_la_SOURCES = ExtractUrl.h ExtractUrl.cc ExtractUrlLexer.h ExtractUrlLexer.cc
ExtractUrl_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/lib/googleurl
ExtractUrl_la_LIBADD = ../resource/libPageResource.la ../resource/libUrlResource.la $(top_builddir)/lib/librobot.la $(top_builddir)/lib/libgoogleurl.la
ExtractUrlLexer.cc: ExtractUrlLexer.l
	$(LEX) -8 -f -Cf -Ca -Cr -i -t ExtractUrlLexer.l >ExtractUrlLexer.cc
BUILT_SOURCES += ExtractUrlLexer.cc
EXTRA_DIST += ExtractUrlLexer.l

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
module_LTLIBRARIES += TagFeaturama.la
TagFeaturama_la_CXXFLAGS = $(AM_CXXFLAGS) @FEATURAMA_CFLAGS@ @CZMORPHOLOGY_CFLAGS@
TagFeaturama_la_LDFLAGS = $(AM_LDFLAGS) @FEATURAMA_LIBS@ @CZMORPHOLOGY_LIBS@
TagFeaturama_la_SOURCES = TagFeaturama.h TagFeaturama.cc
TagFeaturama_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la -lunistring
endif

if TOKENIZER_MODULE
module_LTLIBRARIES += Tokenize.la
Tokenize_la_SOURCES = Tokenize.h Tokenize.cc
Tokenize_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la -lunistring

module_LTLIBRARIES += Tokenize_cs.la
Tokenize_cs_la_SOURCES = Tokenize_cs.cc
Tokenize_cs_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la
endif

module_LTLIBRARIES += ReadRawText.la
ReadRawText_la_SOURCES = ReadRawText.h ReadRawText.cc
ReadRawText_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la

module_LTLIBRARIES += PrintTextResource.la
PrintTextResource_la_SOURCES = PrintTextResource.h PrintTextResource.cc
PrintTextResource_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la

module_LTLIBRARIES += ReadTextResource.la
ReadTextResource_la_SOURCES = ReadTextResource.h ReadTextResource.cc
ReadTextResource_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la

module_LTLIBRARIES += Deduplicate.la
Deduplicate_la_SOURCES = Deduplicate.h Deduplicate.cc
Deduplicate_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la

module_LTLIBRARIES += WordCount.la
WordCount_la_SOURCES = WordCount.h WordCount.cc
WordCount_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la

module_LTLIBRARIES += ParseMst.la
ParseMst_la_SOURCES = ParseMst.h ParseMst.cc
ParseMst_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la

module_LTLIBRARIES += Normalize.la
Normalize_la_SOURCES = Normalize.h Normalize.cc
Normalize_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la

module_LTLIBRARIES += DetectLanguage.la
DetectLanguage_la_SOURCES = DetectLanguage.h DetectLanguage.cc
DetectLanguage_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la

module_LTLIBRARIES += FilterLanguage.la
FilterLanguage_la_SOURCES = FilterLanguage.h FilterLanguage.cc
FilterLanguage_la_LIBADD = $(top_builddir)/lib/librobot.la ../resource/libTextResource.la
