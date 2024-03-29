#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(hector_robot, 0.0.1-dev, qiq@ucw.cz)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE(subdir-objects -Wall -Werror)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])

AC_ARG_ENABLE(debug, [  --enable-debug          turn on debugging support], use_debug=yes, use_debug=no)
AC_ARG_ENABLE(featurama-tagger, [  --enable-featurama-tagger          Enable FeaturamaTagger module], enable_featurama_tagger=yes, enable_featurama_tagger=no)
AM_CONDITIONAL([FEATURAMA_TAGGER_MODULE], [test x$enable_featurama_tagger = xyes])
AC_ARG_ENABLE(fetch, [  --enable-fetch          Enable Fetch module], enable_fetch=yes, enable_fetch=no)
AM_CONDITIONAL([FETCH_MODULE], [test x$enable_fetch = xyes])
AC_ARG_ENABLE(dnsresolve, [  --enable-dnsresolve     Enable DnsResolve module], enable_dnsresolve=yes, enable_dnsresolve=no)
AM_CONDITIONAL([DNSRESOLVE_MODULE], [test x$enable_dnsresolve = xyes])
AC_ARG_ENABLE(tokenizer, [  --enable-tokenizer     Enable Tokenizer module], enable_tokenizer=yes, enable_tokenizer=no)
AM_CONDITIONAL([TOKENIZER_MODULE], [test x$enable_tokenizer = xyes])


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_PROG_LEX
AC_CHECK_PROG(PROTOC, protoc, protoc)
AC_CHECK_PROG(PERL, perl, perl)

# Checks for libraries.
PKG_CHECK_MODULES([LIBXML2], [libxml-2.0 >= 2.5.0])
PKG_CHECK_MODULES([LOG4CXX], [liblog4cxx >= 0.10.0])
PKG_CHECK_MODULES([PROTOBUF], [protobuf >= 0.22])
PKG_CHECK_MODULES([HECTOR_CORE], [hector_core >= 0.0.1])
PKG_CHECK_MODULES([LIBPCRE], [libpcre >= 7.0])
PKG_CHECK_MODULES([LIBPCRECPP], [libpcrecpp >= 7.0])
if test "x$enable_featurama_tagger" = "xyes"; then
	PKG_CHECK_MODULES([CZMORPHOLOGY], [czmorphology >= 1.1.0])
	PKG_CHECK_MODULES([FEATURAMA], [featurama >= 1.0])
fi
AX_CHECK_ICU([4])
HECTOR_CORE_CFLAGS=`echo $HECTOR_CORE_CFLAGS|sed -e 's/ *$/\/hector/'`
HECTOR_CORE_PREFIX=`pkg-config --variable=prefix hector_core`
AC_SUBST(HECTOR_CORE_PREFIX)
#AC_CHECK_LIB(Judy, JudySLIns, [], [ echo "Please install libJudy"; exit -1 ])
if test "x$enable_dnsresolve" = "xyes" -o "x$enable_fetch" = "xyes"; then
	AC_CHECK_LIB(ev, ev_loop, [], [ echo "Please install libev"; exit -1 ])
fi
if test "x$enable_dnsresolve" = "xyes"; then
	AC_CHECK_LIB(ldns, ldns_rr_free, [], [ echo "Please install libldns"; exit -1 ])
	AC_CHECK_LIB(unbound, ub_process, [], [ echo "Please install libunbound"; exit -1 ])
fi
if test "x$enable_fetch" = "xyes"; then
	LIBCURL_CHECK_CONFIG([yes], [7.21.5-DEV], [], [ echo "Please install patched libcurl version"; exit -1])
fi
if test "x$enable_tokenizer" = "xyes" -o "x$enable_featurama_tagger" = "xyes"; then
	AC_CHECK_HEADERS([unistr.h unictype.h], [], [ echo "Please install libunistring"; exit -1])
fi

# hack: protocol buffers need to be linked statically
# http://groups.google.com/group/protobuf/msg/d4e3b2a967018a10
#xxx1=`echo $PROTOBUF_LIBS|sed -e 's,.*-L\([[^ ]]*\).*,\1,'`'/libprotobuf.a'
#xxx2=`echo $PROTOBUF_LIBS|sed -e "s,-lprotobuf,$xxx1,"`
#PROTOBUF_LIBS=`echo $xxx2|sed -e 's,\(.*\)\(-L[[^ ]]*\)\(.*\),\1\3,'`

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h limits.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h ev.h unbound.h], [], [
	echo "Some headers not found"
	exit -1
])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_TYPE_INT8_T
AC_TYPE_UINT8_T
AC_TYPE_INT32_T
AC_TYPE_UINT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([dup2 gethostbyname inet_ntoa memset select setlocale socket sqrt strcasecmp strdup strerror strchr gettimeofday])

# Misc
dnl Check for debug options
if test "x$use_debug" = "xyes"; then
	CXXFLAGS="$CXXFLAGS -ggdb -O0 -Wall"
else
	CXXFLAGS="$CXXFLAGS -DNDEBUG"
fi

dnl set PREFIX in config.h
if test "x${prefix}" = "xNONE"; then
	AC_DEFINE_UNQUOTED(PREFIX, "${ac_default_prefix}", [Package prefix])
else
	AC_DEFINE_UNQUOTED(PREFIX, "${prefix}", [Package prefix] )
fi

# SWIG + Java/Python/Perl and other stuff borrowed from libkml
# Which platform are we building on?
PLATFORM=
case "`uname -s`" in
        Linux)  PLATFORM=linux ;;
        *)      PLATFORM=unknown ;;
esac
AC_SUBST(PLATFORM)

# Java headers and libs
AC_ARG_WITH(java-include-dir,
            [  --with-java-include-dir=DIR look in DIR for Java headers],
            [JAVA_INCLUDE_DIR=$withval],)
AC_SUBST(JAVA_INCLUDE_DIR)
AC_ARG_WITH(java-lib-dir,
            [  --with-java-lib-dir=DIR look in DIR for Java libraries],
            [JAVA_LIB_DIR=$withval],)
AC_SUBST(JAVA_LIB_DIR)

# Python headers and libs
AC_ARG_WITH(python-include-dir,
            [  --with-python-include-dir=DIR look in DIR for Python headers],
            [PYTHON_INCLUDE_DIR=$withval],)
AC_SUBST(PYTHON_INCLUDE_DIR)
AC_ARG_WITH(python-lib-dir,
            [  --with-python-lib-dir=DIR look in DIR for Python libraries],
            [PYTHON_LIB_DIR=$withval],)
AC_SUBST(PYTHON_LIB_DIR)

# If SWIG is available we will try to generate bindings.
AC_ARG_ENABLE(swig,
	[AS_HELP_STRING([--disable-swig],
			[do not generate any SWIG language bindings])])
if test "x$enable_swig" != xno; then
	AC_CHECK_PROG(SWIG, swig, swig)
	if test "x$SWIG_PATH" != x; then
		SWIG_VERSION=`$SWIG -version 2>&1|grep Version|cut -d" " -f3`
		if test ${SWIG_VERSION:4} -lt 35; then
			AC_MSG_WARN([You have SWIG $SWIG_VERSION installed, but helen requires at least SWIG 1.3.35. Non-C++ modules will not be built.])
			SWIG=
		fi
	fi
fi
AM_CONDITIONAL(WITH_SWIG, [test "x$SWIG" != x])

# Shall we build the Perl SWIG bindings?
if test "x$SWIG" != x; then
	AC_ARG_ENABLE(perl,
		      [AS_HELP_STRING([--disable-perl],
				      [do not generate Perl SWIG bindings])])
	if test "x$enable_perl" != xno; then
		PERLB="x"
		AX_PROG_PERL_VERSION([5.8.0])
		PERL_CFLAGS=`perl -MExtUtils::Embed -e ccopts`
		AC_SUBST(PERL_CFLAGS)
		PERL_LDFLAGS=`perl -MExtUtils::Embed -e ldopts`
		AC_SUBST(PERL_LDFLAGS)
		current_CPPFLAGS="$CPPFLAGS"
		CPPFLAGS="$PERL_CFLAGS"
		AC_CHECK_HEADERS([EXTERN.h perl.h],,
			[AC_MSG_WARN([could not find perl.h. The Perl bindings will not be built.])
			PERLB=
			],
			[[#include <EXTERN.h>]]
		)
		LIBS=''
		CPPFLAGS="$current_CPPFLAGS"
	fi
fi
AM_CONDITIONAL(WITH_PERL, [test "x$PERLB" != x])

# Shall we build the Java SWIG bindings?
if test "x$SWIG" != x; then
	AC_ARG_ENABLE(java,
		      [AS_HELP_STRING([--disable-java],
				      [do not generate Java SWIG bindings])])
	if test "x$enable_java" != xno; then
		AC_CHECK_PROG(JAVA, java, java, nojava)
		if test "$JAVA" = "nojava"; then
		  AC_MSG_ERROR([no java binary in PATH])
		fi
		AC_CHECK_PROG(JAVAC, javac, javac, nojavac)
		if test "$JAVAC" = "nojavac"; then
		  AC_MSG_ERROR([no javac binary in PATH, bindings will not be built])
		fi
		AC_CHECK_PROG(JAR, jar, jar, nojar)
		if test "$JAR" = "nojar"; then
		  AC_MSG_ERROR([no jar binary in PATH, bindings will not be built])
		fi
		if test "x$JAVA" != x; then
			if test "$CPPFLAGS" != ""; then
				JAVA_CFLAGS="-I$current_CPPFLAGS -I$JAVA_INCLUDE_DIR -I$JAVA_INCLUDE_DIR/$PLATFORM"
			else
				JAVA_CFLAGS="-I$JAVA_INCLUDE_DIR -I$JAVA_INCLUDE_DIR/$PLATFORM"
			fi
			AC_SUBST(JAVA_CFLAGS)
			current_CPPFLAGS="$CPPFLAGS"
			CPPFLAGS="$JAVA_CFLAGS"
			AC_CHECK_HEADER(jni.h,,
				[AC_MSG_WARN([could not find jni.h. The Java bindings will not be built.])
				JAVA=
				]
			)
			CPPFLAGS="$current_CPPFLAGS"
		fi
	fi
fi
AM_CONDITIONAL(WITH_JAVA, [test "x$JAVA" != x])

# Shall we build the Python SWIG bindings?
if test "x$SWIG" != x; then
	AC_ARG_ENABLE(python,
		      [AS_HELP_STRING([--disable-python],
				      [do not generate Python SWIG bindings])])
	if test "x$enable_python" != xno; then
		AC_CHECK_PROG(PYTHON, python, python)
		if test "x$PYTHON" != x; then
			PYTHON_VERSION=`$PYTHON -c "import sys; print sys.version[[:3]]"`
			if test "$USE_DEFAULT_PYTHON_LIBDIR" = true; then
				PYTHON_SITE_PACKAGES_DIR=`$PYTHON -c 'import distutils.sysconfig; print distutils.sysconfig.get_python_lib(1,0)'`
			else
				PYTHON_SITE_PACKAGES_DIR=`$PYTHON -c 'import distutils.sysconfig; print distutils.sysconfig.get_python_lib(1,0,"${exec_prefix}")'`
			fi
			AC_SUBST(PYTHON_SITE_PACKAGES_DIR)
			if test "$PYTHON_INCLUDE_DIR" = ""; then
				PYTHON_INCLUDE_DIR=`$PYTHON -c "import distutils.sysconfig; print distutils.sysconfig.get_python_inc()"`
			fi
			if test "$PYTHON_LIB_DIR" = ""; then
				PYTHON_LIB_DIR=`$PYTHON -c "import distutils.sysconfig; print distutils.sysconfig.get_config_var('LIBPL')"`
			fi
			PYTHON_CFLAGS="-I$PYTHON_INCLUDE_DIR"
			AC_SUBST(PYTHON_CFLAGS)
			PYTHON_LDFLAGS="-L$PYTHON_LIB_DIR"
			AC_SUBST(PYTHON_LDFLAGS)
			current_CPPFLAGS="$CPPFLAGS"
			CPPFLAGS="-I$PYTHON_INCLUDE_DIR"
			AC_CHECK_HEADER(Python.h,,
				[AC_MSG_WARN([could not find Python.h. The Python bindings will not be built.])
				PYTHON=
				]
			)
			PYTHON_LIBNAME="python"
			if test "$PLATFORM" = "linux"; then
				PYTHON_LIBNAME="python$PYTHON_VERSION"
			fi
			AC_CHECK_LIB($PYTHON_LIBNAME, Py_Initialize,,
				[AC_MSG_WARN([could not find the Python library. The Python bindings will not be built.])
				PYTHON=
				]
			)
			LIBS=''  # don't append -lpython here.
			CPPFLAGS="$current_CPPFLAGS"
		fi
	fi
fi
AM_CONDITIONAL(WITH_PYTHON, [test "x$PYTHON" != x])

AC_CONFIG_FILES([Makefile
		lib/Makefile
		resource/Makefile
                module/Makefile
		module/perl/Makefile
		module/java/Makefile
		module/python/Makefile
		server/Makefile
		perl/Makefile
		python/Makefile
		java/Makefile
		test/Makefile
])
AC_OUTPUT
