AM_CXXFLAGS = @LOG4CXX_CFLAGS@ @PERL_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I $(top_srcdir)/lib -I$(top_srcdir)/resources
AM_LDFLAGS = @LOG4CXX_LIBS@ @PERL_LDFLAGS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@ -module -avoid-version
AM_LIBTOOLFLAGS = --tag=disable-static

BUILT_SOURCES =

if WITH_SWIG
if WITH_PERL

perldir = $(libdir)/hector/modules/perl
perl_SCRIPTS = HectorRobot.pm
perl_LTLIBRARIES = HectorRobot.la

BUILT_SOURCES += HectorRobot_wrap.cc HectorRobot.pm
HectorRobot_wrap.cc HectorRobot.pm: $(top_srcdir)/HectorRobot.i
	$(SWIG) -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/resources -DSWIG_TYPE_TABLE=hector @HECTOR_CORE_CFLAGS@ -c++ -perl -o HectorRobot_wrap.cc $<

HectorRobot_la_SOURCES = \
	HectorRobot_wrap.cc
HectorRobot_la_CXXFLAGS = $(AM_CXXFLAGS) -DSWIGRUNTIME_DEBUG
HectorRobot_la_LIBADD = $(top_builddir)/resources/libWebResource.la $(top_builddir)/resources/libWebSiteResource.la $(top_builddir)/lib/librobot.la
#HectorRobot_la_LIBADD = $(top_builddir)/libhectorrobot.la $(top_builddir)/resources/libWebResource.la


endif
endif

CLEANFILES = $(BUILT_SOURCES)
