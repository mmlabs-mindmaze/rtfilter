eol=

ACLOCAL_AMFLAGS = -I m4 --install
EXTRA_DIST = autogen.sh
CLEANFILES =

lib_LTLIBRARIES = librtfilter.la

AM_CPPFLAGS = \
	-I$(srcdir)/src \
	-DSRCDIR=\"$(srcdir)\" \
	-DBUILDDIR=\"$(builddir)\" \
	$(CHECK_CPPFLAGS) \
	$(eol)

AM_CFLAGS = \
	$(WARNFLAGS) \
	$(eol)

librtfilter_la_SOURCES = \
	src/common-filters.c \
	src/downsampler.c \
	src/filter.c \
	src/filter-complex-double.c \
	src/filter-complex-single.c \
	src/filter-funcs.h \
	src/filter-inreal-outcomplex-double.c \
	src/filter-inreal-outcomplex-single.c \
	src/filter-internal.h \
	src/filter-real-double.c \
	src/filter-real-single.c \
	src/probesimd.h \
	src/rtf_common.h \
	src/rtfilter.h \
	$(eol)

filter_templates =  \
	src/filter-func-template.c \
	src/init-filter-func-template.c \
	$(eol)
EXTRA_DIST += $(filter_templates)

include_HEADERS = \
	src/rtfilter.h \
	src/rtf_common.h \
	$(eol)

librtfilter_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-no-undefined \
	-version-info $(CURRENT):$(REVISION):$(AGE)  \
	$(eol)

librtfilter_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(FASTMATH_CFLAG) \
	$(eol)

noinst_LTLIBRARIES =
librtfilter_la_LIBADD =
if BUILD_SSE_SUPPORT
libfiltersse_la_SOURCES = \
	src/filter-funcs.h \
	src/filter-internal.h \
	src/filter-sse.c \
	$(eol)
libfiltersse_la_CFLAGS = $(AM_CFLAGS) $(FASTMATH_CFLAG) $(SSE_CFLAGS)
noinst_LTLIBRARIES += libfiltersse.la
librtfilter_la_LIBADD += libfiltersse.la
endif

if BUILD_SSE2_SUPPORT
libfiltersse2_la_SOURCES = \
	src/filter-funcs.h \
	src/filter-internal.h \
	src/filter-sse2.c \
	$(eol)
libfiltersse2_la_CFLAGS = $(AM_CFLAGS) $(FASTMATH_CFLAG) $(SSE2_CFLAGS)
noinst_LTLIBRARIES += libfiltersse2.la
librtfilter_la_LIBADD += libfiltersse2.la
endif

if BUILD_SSE3_SUPPORT
libfiltersse3_la_SOURCES = \
	src/filter-funcs.h \
	src/filter-internal.h \
	src/filter-sse3.c \
	$(eol)
libfiltersse3_la_CFLAGS = $(AM_CFLAGS) $(FASTMATH_CFLAG) $(SSE3_CFLAGS)
noinst_LTLIBRARIES += libfiltersse3.la
librtfilter_la_LIBADD += libfiltersse3.la
endif


if HAVE_LD_OUTPUT_DEF
librtfilter_la_LDFLAGS += -Wl,--output-def,librtfilter-$(CURRENT).def
defexecdir = $(bindir)
defexec_DATA = librtfilter-$(CURRENT).def
DISTCLEANFILES = $(defexec_DATA)
$(defexec_DATA): librtfilter.la
endif  # HAVE_LD_OUTPUT_DEF

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = src/rtfilter.pc

#
# DOC RULES
#

doc_DATA = AUTHORS NEWS README COPYING

dist_man_MANS = \
	doc/rtf_create_bandpass_analytic.3 \
	doc/rtf_create_butterworth.3 \
	doc/rtf_create_chebychev.3 \
	doc/rtf_create_downsampler.3 \
	doc/rtf_create_filter.3 \
	doc/rtf_create_filter_coeffs.3 \
	doc/rtf_create_fir_bandpass.3 \
	doc/rtf_create_fir_highpass.3 \
	doc/rtf_create_fir_lowpass.3 \
	doc/rtf_destroy_filter.3 \
	doc/rtf_filter.3 \
	doc/rtf_filter_set_lazy_init.3 \
	doc/rtf_get_type.3 \
	doc/rtf_get_version.3 \
	doc/rtf_init_filter.3 \
	$(eol)

nobase_dist_doc_DATA = \
	doc/examples/butterworth.c \
	doc/examples/Makefile \
	$(eol)

# Verify at least that the example compiles
check_PROGRAMS = butterworth
butterworth_SOURCES = \
	doc/examples/butterworth.c \
	$(eol)

butterworth_LDADD = \
	librtfilter.la \
	$(eol)


#
# TESTS RULES
#

# alias "test" and "check" targets targets
.PHONY: test
test: check

TEST_EXTENSIONS = .test .tap

# if check support TAP output, make use of it
if TAP_SUPPORT_IN_CHECK
AM_CPPFLAGS += -DCHECK_SUPPORT_TAP
TAP_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(srcdir)/build-aux/tap-driver.sh
else
TAP_LOG_DRIVER = $(LOG_DRIVER)
endif  # TAP_SUPPORT_IN_CHECK

TESTS = \
	unittests.tap \
	$(eol)

check_PROGRAMS += \
	unittests.tap \
	$(eol)

unittests_tap_SOURCES = \
	test/rtfilter_test.c \
	test/testcases.h \
	test/test-common.c \
	test/test-common.h \
	test/unittests.c \
	$(eol)

unittests_tap_CFLAGS = \
	$(AM_CFLAGS) \
	$(CHECK_CFLAGS) \
	$(eol)

unittests_tap_LDADD = \
	$(CHECK_LIBS) \
	.libs/librtfilter.la \
	$(MATH_LIB) \
	$(eol)

TESTS_ENVIRONMENT = PATH="$(PATH):$(buildir)" TESTSSRCDIR="$(srcdir)/tests"

#
# DEV TARGETS
#

# rewrite in-place all the source files with correct coding style
EXTRA_DIST += devtools/uncrustify.cfg
.PHONY: fixstyle
fixstyle: devtools/uncrustify.cfg
	cd $(srcdir) && uncrustify -c devtools/uncrustify.cfg -l C --replace \
		$(librtfilter_la_SOURCES) $(filter_templates) $(unittests_tap_SOURCES)

# check all the source files for correct coding style
.PHONY: checkstyle
checkstyle: devtools/uncrustify.cfg
	cd $(srcdir) && uncrustify -c devtools/uncrustify.cfg -l C --check \
		$(librtfilter_la_SOURCES) $(filter_templates) $(unittests_tap_SOURCES)

# primitive syntax check on all the source files
.PHONY: spelling
spelling: $(librtfilter_la_SOURCES) $(filter_templates) $(dist_man_MANS) $(unittests_tap_SOURCES)
	codespell --ignore-words-list=sinc $^
