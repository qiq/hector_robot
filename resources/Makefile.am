AM_CXXFLAGS = @LOG4CXX_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I$(top_srcdir)/lib
AM_LDFLAGS = @LOG4CXX_LIBS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@
AM_LIBTOOLFLAGS = --tag=disable-static

EXTRA_DIST =
BUILT_SOURCES =
CLEANFILES =
ALIASES =

resourcesdir = $(libdir)/hector/resources
resources_LTLIBRARIES =

hectorincludedir = $(includedir)/hector
hectorinclude_HEADERS = 

resources_LTLIBRARIES += libScheme.la
EXTRA_DIST += Scheme.proto Scheme.i
CLEANFILES += Scheme.pb.h Scheme.pb.cc

BUILT_SOURCES += Scheme.pb.h Scheme.pb.cc
Scheme.pb.cc Scheme.pb.h: Scheme.proto
	$(PROTOC) Scheme.proto --cpp_out=.

libScheme_la_SOURCES = Scheme.pb.h Scheme.pb.cc
libScheme_la_LDFLAGS = $(AM_LDFLAGS)

# IndexResource

resources_LTLIBRARIES += libIndexResource.la IndexResource.la
hectorinclude_HEADERS += IndexResource.h IndexResource.i
EXTRA_DIST += IndexResource.i
CLEANFILES += IndexResource.pb.h IndexResource.pb.cc
ALIASES += IndexResource.la

libIndexResource_la_SOURCES = IndexResource.h IndexResource.cc
libIndexResource_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version

IndexResource_la_SOURCES = IndexResource.cc
IndexResource_la_CXXFLAGS = $(AM_CXXFLAGS) -DWRAPPER
IndexResource_la_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
IndexResource_la_LIBADD = libIndexResource.la

# UrlResource

resources_LTLIBRARIES += libUrlResource.la UrlResource.la
hectorinclude_HEADERS += UrlResource.h UrlResource.pb.h UrlResource.i
EXTRA_DIST += UrlResource.i UrlResource.proto
CLEANFILES += UrlResource.pb.h UrlResource.pb.cc
ALIASES += UrlResource.la

libUrlResource_la_SOURCES = UrlResource.h UrlResource.cc UrlResource.pb.h UrlResource.pb.cc
libUrlResource_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version

BUILT_SOURCES += UrlResource.pb.h UrlResource.pb.cc
UrlResource.pb.cc UrlResource.pb.h: UrlResource.proto
	$(PROTOC) UrlResource.proto --cpp_out=.

UrlResource_la_SOURCES = UrlResource.cc
UrlResource_la_CXXFLAGS = $(AM_CXXFLAGS) -DWRAPPER
UrlResource_la_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
UrlResource_la_LIBADD = libUrlResource.la

# SiteResource

resources_LTLIBRARIES += libSiteResource.la SiteResource.la
hectorinclude_HEADERS += SiteResource.h SiteResource.pb.h SiteResource.i
EXTRA_DIST += SiteResource.i SiteResource.proto
CLEANFILES += SiteResource.pb.h SiteResource.pb.cc
ALIASES += SiteResource.la

libSiteResource_la_SOURCES = SiteResource.h SiteResource.cc SiteResource.pb.h SiteResource.pb.cc
libSiteResource_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version

BUILT_SOURCES += SiteResource.pb.h SiteResource.pb.cc
SiteResource.pb.cc SiteResource.pb.h: SiteResource.proto
	$(PROTOC) SiteResource.proto --cpp_out=.

SiteResource_la_SOURCES = SiteResource.cc
SiteResource_la_CXXFLAGS = $(AM_CXXFLAGS) -DWRAPPER
SiteResource_la_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
SiteResource_la_LIBADD = libSiteResource.la

# PageResource

resources_LTLIBRARIES += libPageResource.la PageResource.la
hectorinclude_HEADERS += PageResource.h PageResource.pb.h PageResource.i
EXTRA_DIST += PageResource.i PageResource.proto
CLEANFILES += PageResource.pb.h PageResource.pb.cc
ALIASES += PageResource.la

libPageResource_la_SOURCES = PageResource.h PageResource.cc PageResource.pb.h PageResource.pb.cc
libPageResource_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/lib/googleurl
libPageResource_la_LDFLAGS = $(AM_LDFLAGS) libScheme.la $(top_builddir)/lib/libgoogleurl.la -avoid-version

BUILT_SOURCES += PageResource.pb.h PageResource.pb.cc
PageResource.pb.cc PageResource.pb.h: PageResource.proto
	$(PROTOC) PageResource.proto --cpp_out=.

PageResource_la_SOURCES = PageResource.cc
PageResource_la_CXXFLAGS = $(AM_CXXFLAGS) -DWRAPPER -I$(top_srcdir)/lib/googleurl
PageResource_la_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
PageResource_la_LIBADD = libPageResource.la

# NewUrlResource
#
#resources_LTLIBRARIES += libNewUrlResource.la NewUrlResource.la
#hectorinclude_HEADERS += NewUrlResource.h NewUrlResource.pb.h NewUrlResource.i
#EXTRA_DIST += NewUrlResource.i NewUrlResource.proto
#CLEANFILES += NewUrlResource.pb.h NewUrlResource.pb.cc
#ALIASES += NewUrlResource.la
#
#libNewUrlResource_la_SOURCES = NewUrlResource.h NewUrlResource.cc NewUrlResource.pb.h NewUrlResource.pb.cc
#libNewUrlResource_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version
#
#BUILT_SOURCES += NewUrlResource.pb.h NewUrlResource.pb.cc
#NewUrlResource.pb.cc NewUrlResource.pb.h: NewUrlResource.proto
#	$(PROTOC) NewUrlResource.proto --cpp_out=.
#
#NewUrlResource_la_SOURCES = NewUrlResource.cc
#NewUrlResource_la_CXXFLAGS = $(AM_CXXFLAGS) -DWRAPPER
#NewUrlResource_la_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
#NewUrlResource_la_LIBADD = libNewUrlResource.la

# TextResource

resources_LTLIBRARIES += libTextResource.la TextResource.la
hectorinclude_HEADERS += TextResource.h TextResource.pb.h TextResource.i
EXTRA_DIST += TextResource.i TextResource.proto
CLEANFILES += TextResource.pb.h TextResource.pb.cc
ALIASES += TextResource.la

libTextResource_la_SOURCES = TextResource.h TextResource.cc TextResource.pb.h TextResource.pb.cc
libTextResource_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version

BUILT_SOURCES += TextResource.pb.h TextResource.pb.cc
TextResource.pb.cc TextResource.pb.h: TextResource.proto
	$(PROTOC) TextResource.proto --cpp_out=.

TextResource_la_SOURCES = TextResource.cc
TextResource_la_CXXFLAGS = $(AM_CXXFLAGS) -DWRAPPER
TextResource_la_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
TextResource_la_LIBADD = libTextResource.la

