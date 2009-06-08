#ifndef COMMON_FILTERS_H
#define COMMON_FILTERS_H

#include <filter.h>

typedef enum {
	BLACKMAN_WINDOW,
	HAMMING_WINDOW,
	RECT_WINDOW
} KernelWindow;

hfilter create_fir_filter_mean(unsigned int nsamples, unsigned int nchann, unsigned int type);
hfilter create_fir_filter_lowpass(double fc, unsigned int half_length, unsigned int nchann, KernelWindow window, unsigned int type);
hfilter create_fir_filter_highpass(double fc, unsigned int half_length, unsigned int nchann, KernelWindow window, unsigned int type);
hfilter create_fir_filter_bandpass(double fc_low, double fc_high, unsigned int half_length, unsigned int nchann, KernelWindow window, unsigned int type);
hfilter create_butterworth_filter(double fc, unsigned int num_pole, unsigned int num_chann, int highpass, unsigned int type);
hfilter create_chebychev_filter(double fc, unsigned int num_pole, unsigned int nchann, int highpass, double ripple, unsigned int type);
hfilter create_integrate_filter(unsigned int nchann, unsigned int type);

#endif /*COMMON_FILTERS_H*/
