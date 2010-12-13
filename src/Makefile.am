EXTRA_DIST=templates.c filter-func-template.c exports.sym

lib_LTLIBRARIES = librtfilter.la
noinst_LTLIBRARIES = libcommonfilt.la
include_HEADERS = rtfilter.h rtf_common.h

libcommonfilt_la_SOURCES = rtf_common.h common-filters.c

librtfilter_la_SOURCES = filter-internal.h\
                         rtfilter.h filter.c \
			 filter-real-single.c filter-real-double.c\
			 complex-simd.h\
			 filter-complex-single.c filter-complex-double.c\
			 filter-inreal-outcomplex-single.c filter-inreal-outcomplex-double.c\
			 downsampler.c

librtfilter_la_CFLAGS = $(FASTMATH_CFLAG) $(AMCFLAGS)

librtfilter_la_LIBADD = libcommonfilt.la

librtfilter_la_LDFLAGS = $(AM_LDFLAGS) \
                         -no-undefined	\
			 -export-symbols $(srcdir)/exports.sym  \
                         -version-info $(CURRENT):$(REVISION):$(AGE) 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = rtfilter.pc
