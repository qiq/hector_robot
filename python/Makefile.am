AM_CXXFLAGS = @LOG4CXX_CFLAGS@ @PYTHON_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I$(top_srcdir)/lib -I$(top_srcdir)/resource
AM_LDFLAGS = @LOG4CXX_LIBS@ @PYTHON_LDFLAGS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@ -module -avoid-version
AM_LIBTOOLFLAGS = --tag=disable-static

BUILT_SOURCES =

if WITH_SWIG
if WITH_PYTHON

pythondir = $(libdir)/hector/module/python
python_SCRIPTS = HectorRobot.py
python_LTLIBRARIES = _HectorRobot.la

BUILT_SOURCES += HectorRobot_wrap.cc HectorRobot.py
HectorRobot_wrap.cc HectorRobot.py: $(top_srcdir)/HectorRobot.i
	$(SWIG) -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/resource -DSWIG_TYPE_TABLE=hector @HECTOR_CORE_CFLAGS@ -c++ -python -o $@ $<

_HectorRobot_la_SOURCES = \
	HectorRobot_wrap.cc
_HectorRobot_la_LIBADD = $(top_builddir)/resource/libIndexResource.la $(top_builddir)/resource/libUrlResource.la $(top_builddir)/resource/libSiteResource.la $(top_builddir)/resource/libPageResource.la $(top_builddir)/resource/libTextResource.la $(top_builddir)/lib/librobot.la

endif
endif

CLEANFILES = $(BUILT_SOURCES) *.pyc
