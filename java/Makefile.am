AM_CXXFLAGS = @LOG4CXX_CFLAGS@ @JAVA_CFLAGS@ @PROTOBUF_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I$(top_srcdir)/lib -I$(top_srcdir)/resource
AM_LDFLAGS = @LOG4CXX_LIBS@ @JAVA_LDFLAGS@ @PROTOBUF_LIBS@ @HECTOR_CORE_LIBS@

BUILT_SOURCES =

if WITH_SWIG
if WITH_JAVA

javadir = $(libdir)/hector/module/java
java_LTLIBRARIES = HectorRobot.la

BUILT_SOURCES += HectorRobot_wrap.cc
HectorRobot_wrap.cc: $(top_srcdir)/HectorRobot.i
	$(SWIG) -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/resource -DSWIG_TYPE_TABLE=hector @HECTOR_CORE_CFLAGS@ -c++ -java -o $@ $<

HectorRobot_la_SOURCES = \
	HectorRobot_wrap.cc
HectorRobot_la_LIBADD = $(top_builddir)/resource/libIndexResource.la $(top_builddir)/resource/libUrlResource.la $(top_builddir)/resource/libSiteResource.la $(top_builddir)/resource/libPageResource.la $(top_builddir)/resource/libTextResource.la $(top_builddir)/lib/librobot.la

HectorRobot_la_LDFLAGS = -module -avoid-version

endif
endif

clean-local:
	rm -f HectorRobot_wrap.cc *.java
