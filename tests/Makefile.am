AM_CFLAGS = -I$(top_srcdir)/src
EXTRA_DIST = filein.bin fileout.bin

check_PROGRAMS = testfilter

testfilter_SOURCES = test_filter.c
testfilter_LDADD = $(top_builddir)/src/librtfilter.la

TESTS = testfilter
