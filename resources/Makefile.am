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
hectorinclude_HEADERS = WebResource.h WebResource.pb.h WebResource.i WebSiteResource.h WebSiteResource.pb.h WebSiteResource.i WebSitePath.h IndexResource.h IndexResource.pb.h IndexResource.i

resources_LTLIBRARIES += libScheme.la
EXTRA_DIST += Scheme.proto Scheme.i
CLEANFILES += Scheme.pb.h Scheme.pb.cc

BUILT_SOURCES += Scheme.pb.h Scheme.pb.cc
Scheme.pb.cc Scheme.pb.h: Scheme.proto
	$(PROTOC) Scheme.proto --cpp_out=.

libScheme_la_SOURCES = Scheme.pb.h Scheme.pb.cc
libScheme_la_LDFLAGS = $(AM_LDFLAGS)


resources_LTLIBRARIES += libWebResource.la WebResource.la
hectorinclude_HEADERS += WebResource.h WebResource.pb.h WebResource.i
EXTRA_DIST += WebResource.i WebResource.proto
CLEANFILES += WebResource.pb.h WebResource.pb.cc
ALIASES += WebResource.la

libWebResource_la_SOURCES = WebResource.h WebResource.cc WebResource.pb.h WebResource.pb.cc
libWebResource_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/lib/googleurl
libWebResource_la_LDFLAGS = $(AM_LDFLAGS)
libWebResource_la_LIBADD = libScheme.la $(top_builddir)/lib/libgoogleurl.la

BUILT_SOURCES += WebResource.pb.h WebResource.pb.cc
WebResource.pb.cc WebResource.pb.h: WebResource.proto
	$(PROTOC) WebResource.proto --cpp_out=.
WebResource.pb.h WebResource.pb.cc: Scheme.pb.h Scheme.pb.cc

WebResource_la_SOURCES = WebResource.cc
WebResource_la_CXXFLAGS = $(AM_CXXFLAGS) -DWRAPPER -I$(top_srcdir)/lib/googleurl
WebResource_la_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
WebResource_la_LIBADD = libWebResource.la

resources_LTLIBRARIES += libWebSiteResource.la WebSiteResource.la
hectorinclude_HEADERS += WebSiteResource.h WebSiteResource.pb.h WebSiteResource.i WebSitePath.h
EXTRA_DIST += WebSiteResource.i WebSiteResource.proto
CLEANFILES += WebSiteResource.pb.h WebSiteResource.pb.cc
ALIASES += WebSiteResource.la

libWebSiteResource_la_SOURCES = WebSiteResource.h WebSiteResource.cc WebSiteResource.pb.h WebSiteResource.pb.cc WebSitePath.h
libWebSiteResource_la_LDFLAGS = $(AM_LDFLAGS)
libWebSiteResource_la_LIBADD = libScheme.la -lJudy

BUILT_SOURCES += WebSiteResource.pb.h WebSiteResource.pb.cc
WebSiteResource.pb.cc WebSiteResource.pb.h: WebSiteResource.proto
	$(PROTOC) WebSiteResource.proto --cpp_out=.
WebSiteResource.pb.h WebSiteResource.pb.cc: Scheme.pb.h Scheme.pb.cc

WebSiteResource_la_SOURCES = WebSiteResource.cc
WebSiteResource_la_CXXFLAGS = $(AM_CXXFLAGS) -DWRAPPER
WebSiteResource_la_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
WebSiteResource_la_LIBADD = libWebSiteResource.la

resources_LTLIBRARIES += libIndexResource.la IndexResource.la
hectorinclude_HEADERS += IndexResource.h IndexResource.pb.h IndexResource.i
EXTRA_DIST += IndexResource.i IndexResource.proto
CLEANFILES += IndexResource.pb.h IndexResource.pb.cc
ALIASES += IndexResource.la

libIndexResource_la_SOURCES = IndexResource.h IndexResource.cc IndexResource.pb.h IndexResource.pb.cc
libIndexResource_la_LDFLAGS = $(AM_LDFLAGS)

BUILT_SOURCES += IndexResource.pb.h IndexResource.pb.cc
IndexResource.pb.cc IndexResource.pb.h: IndexResource.proto
	$(PROTOC) IndexResource.proto --cpp_out=.

IndexResource_la_SOURCES = IndexResource.cc
IndexResource_la_CXXFLAGS = $(AM_CXXFLAGS) -DWRAPPER
IndexResource_la_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
IndexResource_la_LIBADD = libIndexResource.la


resources_LTLIBRARIES += libUrlResource.la UrlResource.la
hectorinclude_HEADERS += UrlResource.h UrlResource.pb.h UrlResource.i
EXTRA_DIST += UrlResource.i UrlResource.proto
CLEANFILES += UrlResource.pb.h UrlResource.pb.cc
ALIASES += UrlResource.la

libUrlResource_la_SOURCES = UrlResource.h UrlResource.cc UrlResource.pb.h UrlResource.pb.cc
libUrlResource_la_LDFLAGS = $(AM_LDFLAGS)

BUILT_SOURCES += UrlResource.pb.h UrlResource.pb.cc
UrlResource.pb.cc UrlResource.pb.h: UrlResource.proto
	$(PROTOC) UrlResource.proto --cpp_out=.

UrlResource_la_SOURCES = UrlResource.cc
UrlResource_la_CXXFLAGS = $(AM_CXXFLAGS) -DWRAPPER
UrlResource_la_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
UrlResource_la_LIBADD = libUrlResource.la

