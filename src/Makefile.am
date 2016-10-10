EXTRA_DIST=filter-func-template.c

lib_LTLIBRARIES = librtfilter.la
noinst_LTLIBRARIES = libcommonfilt.la
include_HEADERS = rtfilter.h rtf_common.h

libcommonfilt_la_SOURCES = rtf_common.h common-filters.c complex-arithmetic.h

librtfilter_la_SOURCES = filter-internal.h filter-funcs.h\
                         rtfilter.h filter.c \
			 probesimd.h \
			 complex-arithmetic.h \
			 filter-real-single.c filter-real-double.c\
			 filter-complex-single.c filter-complex-double.c\
			 filter-inreal-outcomplex-single.c\
			 filter-inreal-outcomplex-double.c\
			 downsampler.c

librtfilter_la_CFLAGS = $(FASTMATH_CFLAG) $(AMCFLAGS)

librtfilter_la_LIBADD = libcommonfilt.la
librtfilter_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined	\
                         -version-info $(CURRENT):$(REVISION):$(AGE) 

if BUILD_SSE_SUPPORT
noinst_LTLIBRARIES += libfiltersse.la
librtfilter_la_LIBADD += libfiltersse.la
endif

if BUILD_SSE2_SUPPORT
noinst_LTLIBRARIES += libfiltersse2.la
librtfilter_la_LIBADD += libfiltersse2.la
endif

if BUILD_SSE3_SUPPORT
noinst_LTLIBRARIES += libfiltersse3.la
librtfilter_la_LIBADD += libfiltersse3.la
endif

libfiltersse_la_SOURCES = filter-sse.c filter-internal.h filter-funcs.h
libfiltersse_la_CFLAGS = $(FASTMATH_CFLAG) $(SSE_CFLAGS) $(AMCFLAGS)

libfiltersse2_la_SOURCES = filter-sse2.c filter-internal.h filter-funcs.h
libfiltersse2_la_CFLAGS = $(FASTMATH_CFLAG) $(SSE2_CFLAGS) $(AMCFLAGS)

libfiltersse3_la_SOURCES = filter-sse3.c filter-internal.h filter-funcs.h
libfiltersse3_la_CFLAGS = $(FASTMATH_CFLAG) $(SSE3_CFLAGS) $(AMCFLAGS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = rtfilter.pc

