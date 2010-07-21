EXTRA_DIST = cmp-aligned-float.sh cmp-unaligned-float.sh\
             cmp-aligned-double.sh cmp-unaligned-double.sh\
	     checkfiltres.m
AM_CFLAGS = -I$(top_srcdir)/src

#check_PROGRAMS = cmpwmatlab
#check_PROGRAMS = testcomplex
check_PROGRAMS = testcomplex cmpwmatlab

testcomplex_SOURCES = testcomplex.c
testcomplex_LDADD = $(top_builddir)/src/librtfilter.la
cmpwmatlab_SOURCES = cmpwmatlab.c
cmpwmatlab_LDADD = $(top_builddir)/src/librtfilter.la

TESTS_ENVIRONMENT = PATH=$(PATH):$(buildir)

#TESTS = testcomplex

TESTS = testcomplex\
	test-float.sh test-complex-float.sh 	\
	test-double.sh test-complex-double.sh
