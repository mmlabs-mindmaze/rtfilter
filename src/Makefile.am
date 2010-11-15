EXTRA_DIST=templates.c filter-func-template.c exports.sym

lib_LTLIBRARIES = librtfilter.la
include_HEADERS = rtfilter.h rtf_common.h

librtfilter_la_SOURCES = filter-internal.h\
                         rtfilter.h filter.c \
			 filter-single.c filter-double.c\
			 filter-complex-single.c filter-complex-double.c\
			 downsampler.c	\
			 rtf_common.h common-filters.c

librtfilter_la_LDFLAGS = $(AM_LDFLAGS) \
                         -no-undefined	\
			 -export-symbols $(srcdir)/exports.sym  \
                         -version-info $(CURRENT):$(REVISION):$(AGE) 

