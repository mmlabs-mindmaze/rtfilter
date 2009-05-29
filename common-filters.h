#ifndef COMMON_FILTERS_H
#define COMMON_FILTERS_H

#include <filter.h>

typedef enum {
	BLACKMAN_WINDOW,
	HAMMING_WINDOW,
	RECT_WINDOW
} KernelWindow;

hfilter create_fir_filter_mean(unsigned int nsamples, unsigned int nchann);
hfilter create_fir_filter_lowpass(typereal fc, unsigned int half_length, unsigned int nchann, KernelWindow window);
hfilter create_fir_filter_highpass(typereal fc, unsigned int half_length, unsigned int nchann, KernelWindow window);
hfilter create_fir_filter_bandpass(typereal fc_low, typereal fc_high, unsigned int half_length, unsigned int nchann, KernelWindow window);
hfilter create_butterworth_filter(typereal fc, unsigned int num_pole, unsigned int num_chann, int highpass);
hfilter create_chebychev_filter(typereal fc, unsigned int num_pole, unsigned int nchann, int highpass, typereal ripple);
hfilter create_integrate_filter(unsigned int nchann);

#endif /*COMMON_FILTERS_H*/
