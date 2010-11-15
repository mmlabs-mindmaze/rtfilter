EXTRA_DIST = test-float.sh test-complex-float.sh\
             test-double.sh test-complex-double.sh\
             test-downsample.sh\
	     checkfiltres.m checkdown.m
AM_CFLAGS = -I$(top_srcdir)/src

check_PROGRAMS = cmpwmatlab downsample testchebyshev

cmpwmatlab_SOURCES = cmpwmatlab.c
cmpwmatlab_LDADD = $(top_builddir)/src/librtfilter.la

downsample_SOURCES = downsample.c
downsample_LDADD = $(top_builddir)/src/librtfilter.la

testchebyshev_SOURCES = testchebyshev.c cheby-table.h
testchebyshev_LDADD = $(top_builddir)/src/librtfilter.la

TESTS_ENVIRONMENT = PATH=$(PATH):$(buildir)

TESTS = test-float.sh test-complex-float.sh 	\
	test-double.sh test-complex-double.sh   \
	test-downsample.sh			\
	testchebyshev
