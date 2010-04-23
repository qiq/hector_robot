AM_CXXFLAGS = @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@
AM_LDFLAGS = @LIBXML2_LIBS@ @LOG4CXX_LIBS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@
AM_LIBTOOLFLAGS = --tag=disable-static

BUILT_SOURCES =
EXTRA_DIST =

resourcesdir = $(libdir)/hector/resources
resources_LTLIBRARIES = libWebResource.la WebResource.la

libWebResource_la_SOURCES = WebResource.h WebResource.cc WebResource.pb.h WebResource.pb.cc
libWebResource_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version

BUILT_SOURCES += WebResource.pb.h WebResource.pb.cc
WebResource.pb.cc WebResource.pb.h: WebResource.proto
	$(PROTOC) WebResource.proto --cpp_out=.

WebResource_la_SOURCES = WebResourceWrapper.h WebResourceWrapper.cc
WebResource_la_LDFLAGS = $(AM_LDFLAGS) -module -avoid-version
WebResource_la_LIBADD = libWebResource.la

EXTRA_DIST += WebResource.i WebResource.proto

CLEANFILES = $(BUILT_SOURCES)
