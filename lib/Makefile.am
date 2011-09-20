AM_CXXFLAGS = @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@ @HECTOR_CORE_CFLAGS@ -I$(top_srcdir)/resources
AM_LDFLAGS = @LIBXML2_LIBS@ @LOG4CXX_LIBS@ @HECTOR_CORE_LIBS@

lib_LTLIBRARIES = librobot.la
noinst_LTLIBRARIES = libgoogleurl.la libgooglecld.la

hectorincludedir = $(includedir)/hector
hectorinclude_HEADERS = robot_common.h SuperFastHash.h SitePathMD5.h NgramBloomFilter.h \
	robot_common.i

librobot_la_SOURCES = \
	robot_common.h robot_common.cc SuperFastHash.h SuperFastHash.cc ParsedUrl.h ParsedUrl.cc
librobot_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/lib/googleurl
librobot_la_LIBADD = libgoogleurl.la libgooglecld.la
librobot_la_LDFLAGS = -version-info 0:0:0 $(AM_LDFLAGS) -lltdl

libgoogleurl_la_SOURCES = googleurl/src/gurl.h googleurl/src/url_canon.h googleurl/src/url_canon_icu.h googleurl/src/url_canon_internal.h googleurl/src/url_canon_internal_file.h googleurl/src/url_canon_ip.h googleurl/src/url_canon_stdstring.h googleurl/src/url_common.h googleurl/src/url_file.h googleurl/src/url_parse.h googleurl/src/url_parse_internal.h googleurl/src/url_util.h googleurl/src/gurl.cc googleurl/src/url_canon_etc.cc googleurl/src/url_canon_fileurl.cc googleurl/src/url_canon_host.cc googleurl/src/url_canon_icu.cc googleurl/src/url_canon_internal.cc googleurl/src/url_canon_ip.cc googleurl/src/url_canon_mailtourl.cc googleurl/src/url_canon_path.cc googleurl/src/url_canon_pathurl.cc googleurl/src/url_canon_query.cc googleurl/src/url_canon_relative.cc googleurl/src/url_canon_stdurl.cc googleurl/src/url_parse.cc googleurl/src/url_parse_file.cc googleurl/src/url_util.cc googleurl/base/basictypes.h googleurl/base/logging.h googleurl/base/scoped_ptr.h googleurl/base/string16.h googleurl/base/string16.cc
libgoogleurl_la_CXXFLAGS = @ICU_CXXFLAGS@ -I$(top_srcdir)/lib/googleurl -Wno-deprecated -Wno-unused-parameter -DNDEBUG -fPIC
libgoogleurl_la_LDFLAGS = @ICU_LIBS@
libgoogleurl_la_LIBTOOLFLAGS = --tag=disable-shared

libgooglecld_la_SOURCES = googlecld/encodings/compact_lang_det/cldutil.cc googlecld/encodings/compact_lang_det/cldutil_dbg_empty.cc googlecld/encodings/compact_lang_det/compact_lang_det.cc googlecld/encodings/compact_lang_det/compact_lang_det_impl.cc googlecld/encodings/compact_lang_det/ext_lang_enc.cc googlecld/encodings/compact_lang_det/getonescriptspan.cc googlecld/encodings/compact_lang_det/letterscript_enum.cc googlecld/encodings/compact_lang_det/subsetsequence.cc googlecld/encodings/compact_lang_det/tote.cc googlecld/encodings/compact_lang_det/generated/cld_generated_score_quadchrome_0406.cc googlecld/encodings/compact_lang_det/generated/compact_lang_det_generated_cjkbis_0.cc googlecld/encodings/compact_lang_det/generated/compact_lang_det_generated_ctjkvz.cc googlecld/encodings/compact_lang_det/generated/compact_lang_det_generated_deltaoctachrome.cc googlecld/encodings/compact_lang_det/generated/compact_lang_det_generated_quadschrome.cc googlecld/encodings/compact_lang_det/win/cld_htmlutils_windows.cc googlecld/encodings/compact_lang_det/win/cld_unicodetext.cc googlecld/encodings/compact_lang_det/win/cld_unilib_windows.cc googlecld/encodings/compact_lang_det/win/cld_utf8statetable.cc googlecld/encodings/compact_lang_det/win/cld_utf8utils_windows.cc googlecld/encodings/internal/encodings.cc googlecld/languages/internal/languages.cc googlecld/languages/proto/languages.pb.h googlecld/languages/public/languages.h googlecld/encodings/lang_enc.h googlecld/encodings/proto/encodings.pb.h googlecld/encodings/compact_lang_det/cldutil_dbg.h googlecld/encodings/compact_lang_det/ext_lang_enc.h googlecld/encodings/compact_lang_det/compact_lang_det.h googlecld/encodings/compact_lang_det/cldutil.h googlecld/encodings/compact_lang_det/unittest_data.h googlecld/encodings/compact_lang_det/subsetsequence.h googlecld/encodings/compact_lang_det/utf8propjustletter.h googlecld/encodings/compact_lang_det/win/cld_macros.h googlecld/encodings/compact_lang_det/win/cld_unilib.h googlecld/encodings/compact_lang_det/win/cld_strtoint.h googlecld/encodings/compact_lang_det/win/cld_google.h googlecld/encodings/compact_lang_det/win/cld_unicodetext.h googlecld/encodings/compact_lang_det/win/cld_htmlutils.h googlecld/encodings/compact_lang_det/win/cld_basictypes.h googlecld/encodings/compact_lang_det/win/cld_commandlineflags.h googlecld/encodings/compact_lang_det/win/normalizedunicodetext.h googlecld/encodings/compact_lang_det/win/cld_utf8statetable.h googlecld/encodings/compact_lang_det/win/cld_utf8utils.h googlecld/encodings/compact_lang_det/win/cld_utf.h googlecld/encodings/compact_lang_det/win/cld_logging.h googlecld/encodings/compact_lang_det/letterscript_enum.h googlecld/encodings/compact_lang_det/tote.h googlecld/encodings/compact_lang_det/getonescriptspan.h googlecld/encodings/compact_lang_det/compact_lang_det_impl.h googlecld/encodings/compact_lang_det/utf8scannotjustletterspecial.h googlecld/encodings/compact_lang_det/generated/compact_lang_det_generated_meanscore.h googlecld/encodings/compact_lang_det/utf8propletterscriptnum.h googlecld/encodings/public/encodings.h googlecld/base/basictypes.h googlecld/base/commandlineflags.h googlecld/base/string_util.h googlecld/base/scoped_ptr.h googlecld/base/crash.h googlecld/base/global_strip_options.h googlecld/base/strtoint.h googlecld/base/log_severity.h googlecld/base/casts.h googlecld/base/template_util.h googlecld/base/macros.h googlecld/base/stl_decl.h googlecld/base/dynamic_annotations.h googlecld/base/stl_decl_msvc.h googlecld/base/build_config.h googlecld/base/port.h googlecld/base/logging.h googlecld/base/vlog_is_on.h googlecld/base/type_traits.h
libgooglecld_la_CXXFLAGS = -I$(top_srcdir)/lib/googlecld -Wno-deprecated -DNDEBUG -fPIC -DCLD_WINDOWS -I$(top_srcdir)/lib/googlecld -Wno-sign-compare
libgooglecld_la_LDFLAGS = 
libgooglecld_la_LIBTOOLFLAGS = --tag=disable-shared

#bin_PROGRAMS = test
#test_SOURCES = \
#	test.cc
#test_CXXFLAGS = -I$(top_srcdir)/lib/googleurl
#test_LDFLAGS = -lpthread
#test_LDADD = libgoogleurl.la

#bin_PROGRAMS = test2
#test2_SOURCES = \
#	test2.cc
#test2_CXXFLAGS = @ICU_CXXFLAGS@ -I$(top_srcdir)/lib/googlecld
#test2_LDFLAGS = @ICU_LIBS@
#test2_LDADD = libgooglecld.la
