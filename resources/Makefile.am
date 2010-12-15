AM_CXXFLAGS = @LOG4CXX_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I$(top_srcdir)/lib
AM_LDFLAGS = @LOG4CXX_LIBS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@
AM_LIBTOOLFLAGS = --tag=disable-static

EXTRA_DIST =
BUILT_SOURCES =
CLEANFILES =
ALIASES =

resourcesdir = $(libdir)/hector/resources
resources_LTLIBRARIES = libScheme.la libWebResource.la WebResource.la libWebSiteResource.la WebSiteResource.la

hectorincludedir = $(includedir)/hector
hectorinclude_HEADERS = WebResource.h WebResource.pb.h WebResource.i WebSiteResource.h WebSiteResource.pb.h WebSiteResource.i WebSitePath.h

BUILT_SOURCES += Scheme.pb.h Scheme.pb.cc
Scheme.pb.cc Scheme.pb.h: Scheme.proto
	$(PROTOC) Scheme.proto --cpp_out=.

libScheme_la_SOURCES = Scheme.pb.h Scheme.pb.cc
libScheme_la_LDFLAGS = $(AM_LDFLAGS)

EXTRA_DIST += Scheme.proto Scheme.i
CLEANFILES += Scheme.pb.h Scheme.pb.cc

libWebResource_la_SOURCES = WebResource.h WebResource.cc WebResource.pb.h WebResource.pb.cc
libWebResource_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/lib/googleurl
libWebResource_la_LDFLAGS = $(AM_LDFLAGS)
libWebResource_la_LIBADD = libScheme.la $(top_builddir)/lib/libgoogleurl.la

BUILT_SOURCES += WebResource.pb.h WebResource.pb.cc
WebResource.pb.cc WebResource.pb.h: WebResource.proto
	$(PROTOC) WebResource.proto --cpp_out=.
WebResource.pb.h WebResource.pb.cc: Scheme.pb.h Scheme.pb.cc

WebResource_la_SOURCES = WebResourceWrapper.h WebResourceWrapper.cc
WebResource_la_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
WebResource_la_LIBADD = libWebResource.la

EXTRA_DIST += WebResource.i WebResource.proto
CLEANFILES += WebResource.pb.h WebResource.pb.cc
ALIASES += WebResource.la

libWebSiteResource_la_SOURCES = WebSiteResource.h WebSiteResource.cc WebSiteResource.pb.h WebSiteResource.pb.cc WebSitePath.h
libWebSiteResource_la_LDFLAGS = $(AM_LDFLAGS)
libWebSiteResource_la_LIBADD = libScheme.la -lJudy

BUILT_SOURCES += WebSiteResource.pb.h WebSiteResource.pb.cc
WebSiteResource.pb.cc WebSiteResource.pb.h: WebSiteResource.proto
	$(PROTOC) WebSiteResource.proto --cpp_out=.
WebSiteResource.pb.h WebSiteResource.pb.cc: Scheme.pb.h Scheme.pb.cc

WebSiteResource_la_SOURCES = WebSiteResourceWrapper.h WebSiteResourceWrapper.cc
WebSiteResource_la_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
WebSiteResource_la_LIBADD = libWebSiteResource.la

EXTRA_DIST += WebSiteResource.i WebSiteResource.proto
CLEANFILES += WebSiteResource.pb.h WebSiteResource.pb.cc
ALIASES += WebSiteResource.la

# see comment in hector_core/lib/processing_engine/resources/Makefile.am

all-local: $(ALIASES)
	for i in $(ALIASES); do \
		h=`echo $$i|sed -e 's/\.la/\.h/'`; \
		id=`grep "int typeId" $(top_srcdir)/resources/$$h|sed -e 's/.*int typeId = \([0-9]\+\).*/\1/'`; \
		ln -fs $$i $${id}.la; \
	done

clean-local:
	rm -f {0,1,2,3,4,5,6,7,8,9}*.la

install-exec-local:
	$(INSTALL) -d $(DESTDIR)$(resourcesdir); \
	for i in $(ALIASES); do \
		h=`echo $$i|sed -e 's/\.la/\.h/'`; \
		id=`grep "int typeId" $(top_srcdir)/resources/$$h|sed -e 's/.*int typeId = \([0-9]\+\).*/\1/'`; \
		ln -fs $$i $(DESTDIR)$(resourcesdir)/$${id}.la; \
	done
