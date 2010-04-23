SUBDIRS = perl java python

AM_CXXFLAGS = @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@
AM_LDFLAGS = @LIBXML2_LIBS@ @LOG4CXX_LIBS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@ -module -avoid-version
AM_LIBTOOLFLAGS = --tag=disable-static

modulesdir = $(libdir)/hector/modules
modules_LTLIBRARIES = TestInput.la TestOutput.la TestSimple.la TestMulti.la

TestInput_la_SOURCES = \
		TestInput.h TestInput.cc

TestOutput_la_SOURCES = \
	TestOutput.h TestOutput.cc

TestSimple_la_SOURCES = \
		TestSimple.h TestSimple.cc

TestMulti_la_SOURCES = \
		TestMulti.h TestMulti.cc
