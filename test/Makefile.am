TESTS_ENVIRONMENT = PATH='$(HECTOR_CORE_PREFIX)/bin:$(PATH)' LD_LIBRARY_PATH=$(HECTOR_CORE_PREFIX)/lib:$(HECTOR_CORE_PREFIX)/lib/hector:$(HECTOR_CORE_PREFIX)/lib/hector/resources:$(HECTOR_CORE_PREFIX)/lib/hector/modules:$(top_builddir)/resources:$(top_builddir)/modules:$(top_builddir)/servers:$(top_builddir)/perl/.libs:$(LD_LIBRARY_PATH) PERL5LIB=$(HECTOR_CORE_PREFIX)/lib/hector/modules/perl:$(top_builddir)/perl:$(top_builddir)/modules/perl $(SHELL)

TESTS =
EXTRA_DIST = test_common.sh test.log.props
CLEANFILES = test.log test*.log.valgrind

TESTS += test1.sh
EXTRA_DIST += test1.sh test1-config.xml test1.log.correct test1.data.url
CLEANFILES += test1.log.test

TESTS += test2.sh
EXTRA_DIST += test2.sh test2-config.xml test2.log.correct
CLEANFILES += test2.log.test

TESTS += test3.sh
EXTRA_DIST += test3.sh test3-config.xml test3.log.correct
CLEANFILES += test3.log.test

TESTS += test4.sh
EXTRA_DIST += test4.sh test4-config.xml test4.log.correct
CLEANFILES += test4.log.test

TESTS += test5.sh
EXTRA_DIST += test5.sh test5-config.xml test5.log.correct
CLEANFILES += test5.log.test
