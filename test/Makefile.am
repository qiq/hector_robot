TESTS_ENVIRONMENT = PATH='$(HECTOR_CORE_PREFIX)/bin:$(top_srcdir)/test:$(PATH)' LD_LIBRARY_PATH=$(HECTOR_CORE_PREFIX)/lib:$(HECTOR_CORE_PREFIX)/lib/hector:$(HECTOR_CORE_PREFIX)/lib/hector/resource:$(HECTOR_CORE_PREFIX)/lib/hector/module:$(top_builddir)/resource:$(top_builddir)/module:$(top_builddir)/server:$(top_builddir)/perl/.libs:$(LD_LIBRARY_PATH) PERL5LIB=$(HECTOR_CORE_PREFIX)/lib/hector/module/perl:$(top_builddir)/perl:$(top_builddir)/module/perl $(SHELL)

TESTS =
EXTRA_DIST = test_common.sh dns_server.pl http_server.pl
CLEANFILES = test*.log.valgrind

TESTS += deduplicate.sh
EXTRA_DIST += deduplicate.xml deduplicate.in
CLEANFILES += deduplicate.out deduplicate.log deduplicate.log.result

TESTS += deduplicate_para.sh
EXTRA_DIST += deduplicate_para.xml deduplicate_para.in
CLEANFILES += deduplicate_para.out deduplicate_para.log deduplicate_para.log.result

TESTS += detect_language.sh
EXTRA_DIST += detect_language.xml detect_language.in
CLEANFILES += detect_language.out detect_language.log detect_language.log.result

#TESTS += test1.sh
#EXTRA_DIST += test1.sh test1_config.xml test1.log.correct test1.data.url
#CLEANFILES += test1.log test1.log.result
#
#TESTS += test2.sh
#EXTRA_DIST += test2.sh test2_config.xml test2.log.correct
#CLEANFILES += test2.log test2.log.result
#
#TESTS += test3.sh
#EXTRA_DIST += test3.sh test3_config.xml test3.log.correct
#CLEANFILES += test3.log test3.log.result
#
#TESTS += test4.sh
#EXTRA_DIST += test4.sh test4_config.xml test4.log.correct
#CLEANFILES += test4.log test4.log.result
#
#TESTS += test5.sh
#EXTRA_DIST += test5.sh test5_config.xml test5.log.correct
#CLEANFILES += test5.log test5.log.result
#
#TESTS += test6.sh
#EXTRA_DIST += test6.sh test6_config.xml test6.log.correct
#CLEANFILES += test6.log test6.log.result test6.data.out
#
#TESTS += test7.sh
#EXTRA_DIST += test7.sh test7_config.xml test7.log.correct
#CLEANFILES += test7.log test7.log.result test7.data.tmp
#
#TESTS += test8.sh
#EXTRA_DIST += test8.sh test8_config.xml test8.log.correct test8.data.in
#CLEANFILES += test8.log test8.log.result test8.data.out
#
#TESTS += test9.sh
#EXTRA_DIST += test9.sh test9_config.xml test9.log.correct
#CLEANFILES += test9.log test9.log.result
#
#TESTS += test10.sh
#EXTRA_DIST += test10.sh test10_config.xml test10.log.correct
#CLEANFILES += test10.log test10.log.result
#
#TESTS += test11.sh
#EXTRA_DIST += test11.sh test11_config.xml test11.log.correct
#CLEANFILES += test11.log test11.log.result
#
#TESTS += test12.sh
#EXTRA_DIST += test12.sh test12_config.xml test12.log.correct test12.filter
#CLEANFILES += test12.log test12.log.result
