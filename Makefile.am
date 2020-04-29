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

doc_DATA = AUTHORS NEWS README.md COPYING

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


DOC_SRCS = \
	doc/index.rst \
	doc/rtfilter.rst \
	$(eol)

export srctree=$(SRCTREE)

#
# Sphinx documentation build targets
# Skipped if --enable-sphinxdoc=no
#
if BUILD_SPHINXDOC
SPHINXBUILD   = sphinx-build
SRCTREE       = $(top_srcdir)

sphinx_verbose = $(sphinx_verbose_@AM_V@)
sphinx_verbose_ = $(sphinx_verbose_@AM_DEFAULT_V@)
sphinx_verbose_0 = -q

ALLSPHINXOPTS = -d $(builddir)/.doctrees $(sphinx_verbose) $(srcdir)/doc -c $(srcdir)/doc

# Due to a dependency bug in sphinx-build, we use the -M option instead of
# the usual -b one. This hidden option sums up to the same as -b with makefile
# dependency support ... except that it does not support -W option which turns
# warnings into errors !
# => do this with a simple grep line on the output
# Since html target depends on man target, this is only needed once
sphinx-build.log: $(wildcard $(SRCTREE)/src/*.[ch]) $(DOC_SRCS)
	@$(RM) -f manpages-buildstamp
	$(AM_V_GEN) $(SPHINXBUILD) -M kernel-doc-man $(ALLSPHINXOPTS) $(builddir)/man &> sphinx-build.log

# sphinx-build.log target does the doc generation
# This target checks that it was done right
manpages-buildstamp: sphinx-build.log
	@if [ "$(shell grep -c WARNING: sphinx-build.log)" != "0" ] ; then \
		grep WARNING: sphinx-build.log; \
		false; \
	fi
	@echo
	@echo "Build finished. The man pages are in $(builddir)/man."
	@touch $@

.PHONY: man-pages
man-pages: manpages-buildstamp
html-local: html-buildstamp

# always generate html after man: parallel sphinx invocation is not upported
html-buildstamp: $(wildcard $(SRCTREE)/src/*.[ch]) $(DOC_SRCS) manpages-buildstamp
	@$(RM) -f $@
	$(AM_V_GEN) $(SPHINXBUILD) -M html $(ALLSPHINXOPTS) $(builddir)/html
	@echo
	@echo "Build finished. The HTML pages are in $(builddir)/html."
	@touch $@

install-html: html-buildstamp
	@mkdir -p $(DESTDIR)$(docdir)/html/
	cd $(builddir)/html && \
		find . -type f -exec $(install_sh_DATA) '{}' $(DESTDIR)$(docdir)/html/'{}' \;

uninstall-html:
	$(RM) -r $(DESTDIR)$(docdir)/html/

install-man3: manpages-buildstamp
	@mkdir -p $(DESTDIR)$(mandir)/man3
	$(foreach manpage, $(notdir $(wildcard $(builddir)/man/*.3)), \
		$(install_sh_DATA) $(builddir)/man/$(manpage) $(DESTDIR)$(mandir)/man3/$(manpage);)

uninstall-man3: manpages-buildstamp
	$(foreach manpage, $(notdir $(wildcard $(builddir)/man/*.3)), \
		$(RM) $(DESTDIR)$(mandir)/man3/$(manpage);)
	@-rmdir $(mandir)/man3

all-local: manpages-buildstamp html-buildstamp
install-data-local: install-man3 install-html
uninstall-local: uninstall-man3 uninstall-html

endif # BUILD_SPHINXDOC

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

if HAVE_CLOCK_FN
check_PROGRAMS += perf-test
endif

unittests_tap_SOURCES = \
	test/float-comparison.c \
	test/float-comparison.h \
	test/lowpass_test.c \
	test/rtfilter_combine_test.c \
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

perf_test_SOURCES = \
	test/perf-test.c \
	$(eol)

perf_test_CFLAGS = \
	$(AM_CFLAGS) \
	$(eol)

perf_test_LDADD = \
	.libs/librtfilter.la \
	$(MATH_LIB) \
	$(eol)

TESTS_ENVIRONMENT = PATH="$(PATH):$(buildir)" TESTSSRCDIR="$(srcdir)/tests"

# only test rtfilter.h and not all exported headers: $(include_HEADERS)
# "rtf_common.h" contains a #warning which would always make the test fail
# otherwise
.PHONY: api-compat-test
api-compat-test: src/rtfilter.h
	$(srcdir)/devtools/api-compat-test.sh $(srcdir)/src $^

#
# DEV TARGETS
#

# rewrite in-place all the source files with correct coding style
EXTRA_DIST += devtools/uncrustify.cfg
.PHONY: fixstyle
fixstyle: devtools/uncrustify.cfg
	cd $(srcdir) && uncrustify -c devtools/uncrustify.cfg -l C --replace \
		$(librtfilter_la_SOURCES) $(filter_templates) $(unittests_tap_SOURCES) \
		$(perf_test_SOURCES) \
		doc/examples/butterworth.c

# check all the source files for correct coding style
.PHONY: checkstyle
checkstyle: devtools/uncrustify.cfg
	cd $(srcdir) && uncrustify -c devtools/uncrustify.cfg -l C --check \
		$(librtfilter_la_SOURCES) $(filter_templates) $(unittests_tap_SOURCES) \
		$(perf_test_SOURCES) \
		doc/examples/butterworth.c

# primitive syntax check on all the source files
.PHONY: spelling
spelling: $(librtfilter_la_SOURCES) $(filter_templates) $(dist_man_MANS) \
		$(perf_test_SOURCES) \
		$(unittests_tap_SOURCES) doc/examples/butterworth.c
	codespell --ignore-words-list=sinc $^

.PHONY: coverage
coverage:
	$(srcdir)/devtools/coverage.sh run

clean-local:
	$(RM) -rf html .doctrees man manpages-buildstamp html-buildstamp sphinx-build.log
	cd test && $(RM) *.gcno *.gcda
