EXTRA_DIST = cmp-aligned-float.sh cmp-unaligned-float.sh\
             cmp-aligned-double.sh cmp-unaligned-double.sh\
	     checkfiltres.m
AM_CFLAGS = -I$(top_srcdir)/src

check_PROGRAMS = cmpwmatlab

cmpwmatlab_SOURCES = cmpwmatlab.c
cmpwmatlab_LDADD = $(top_builddir)/src/librtfilter.la

TESTS_ENVIRONMENT = PATH=$(PATH):$(buildir)
TESTS =	cmp-aligned-float.sh cmp-unaligned-float.sh\
	cmp-aligned-double.sh cmp-unaligned-double.sh

