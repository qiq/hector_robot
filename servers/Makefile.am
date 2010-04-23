AM_CXXFLAGS = @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@ @HECTOR_CORE_CFLAGS@
AM_LDFLAGS = @LIBXML2_LIBS@ @LOG4CXX_LIBS@ @HECTOR_CORE_LIBS@ -module -avoid-version
AM_LIBTOOLFLAGS = --tag=disable-static


serverdir = $(libdir)/hector
server_LTLIBRARIES = RobotServer.la

RobotServer_la_SOURCES = RobotServer.h RobotServer.cc
RobotServer_la_CXXFLAGS = $(AM_CXXFLAGS)
RobotServer_la_LDFLAGS = $(AM_LDFLAGS)
