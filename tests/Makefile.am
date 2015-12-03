EXTRA_DIST = test-float.sh test-complex-float.sh\
             test-double.sh test-complex-double.sh\
             test-downsample.sh\
	     checkfiltres.m checkdown.m
AM_CFLAGS = -I$(top_srcdir)/src

if HAVE_CLOCK_FN
noinst_PROGRAMS = measure-rtfilter
measure_rtfilter_SOURCES = measure-rtfilter.c
measure_rtfilter_LDADD = $(top_builddir)/src/librtfilter.la $(OPTION_LIBS)
endif

check_PROGRAMS = cmpwmatlab downsample testchebyshev

cmpwmatlab_SOURCES = cmpwmatlab.c
cmpwmatlab_LDADD = $(top_builddir)/src/librtfilter.la

downsample_SOURCES = downsample.c
downsample_LDADD = $(top_builddir)/src/librtfilter.la

testchebyshev_SOURCES = testchebyshev.c cheby-table.h
testchebyshev_LDADD = $(top_builddir)/src/librtfilter.la

TESTS_ENVIRONMENT = PATH="$(PATH):$(buildir)"

TESTS = testchebyshev \
	 test-float.sh test-complex-float.sh 	\
	 test-double.sh test-complex-double.sh   \
	 test-downsample.sh
