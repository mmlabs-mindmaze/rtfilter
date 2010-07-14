EXTRA_DIST=templates.c filter-func-template.c

lib_LTLIBRARIES = librtfilter.la
include_HEADERS = filter.h common-filters.h

librtfilter_la_SOURCES = filter-internal.h\
                         filter.h filter.c \
			 filter-single.c filter-double.c\
			  common-filters.h common-filters.c

librtfilter_la_LDFLAGS = $(AM_CFLAGS) #-version-info $(CURRENT):$(REVISION):$(AGE) -release $(RELEASE) 
librtfilter_la_LIBADD = -lm

