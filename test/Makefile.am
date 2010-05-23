TESTS_ENVIRONMENT = PATH='$(top_srcdir)/test:$(PATH)' LD_LIBRARY_PATH=$(top_builddir)/lib/processing_engine/modules:$(top_builddir)/lib/processing_engine/resources:$(top_builddir)/lib/processing_engine/servers:$(LD_LIBRARY_PATH) $(SHELL)

TESTS =
EXTRA_DIST = common.sh test.log.props
CLEANFILES = test.log test*.log.valgrind

TESTS += test1.sh
EXTRA_DIST += test1.sh test1-config.xml test1.log.correct
CLEANFILES += test1.log.test

