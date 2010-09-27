dist_man_MANS = rtf_create_filter.3 rtf_init_filter.3 rtf_destroy_filter.3 \
                rtf_filter.3 \
		rtf_create_fir_lowpass.3 rtf_create_fir_highpass.3  \
		rtf_create_fir_bandpass.3 \
		rtf_create_chebychev.3 rtf_create_butterworth.3 \
		rtf_create_bandpass_analytic.3

nobase_dist_doc_DATA = examples/Makefile		\
                       examples/butterworth.c

# Verify at least that the example compiles
check_PROGRAMS = butterworth
butterworth_SOURCES = examples/butterworth.c
butterworth_LDADD = $(top_builddir)/src/librtfilter.la

