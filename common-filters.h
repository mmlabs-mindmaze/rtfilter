#ifndef COMMON_FILTERS_H
#define COMMON_FILTERS_H

#include <filter.h>

typedef enum {
	BLACKMAN_WINDOW,
	HAMMING_WINDOW,
	RECT_WINDOW
} KernelWindow;

const dfilter* create_fir_filter_mean(unsigned int nsamples, unsigned int nchann);
const dfilter* create_fir_filter_lowpass(typereal fc, unsigned int half_length, unsigned int nchann, KernelWindow window);
const dfilter* create_fir_filter_highpass(typereal fc, unsigned int half_length, unsigned int nchann, KernelWindow window);
const dfilter* create_fir_filter_bandpass(typereal fc_low, typereal fc_high, unsigned int half_length, unsigned int nchann, KernelWindow window);
const dfilter* create_butterworth_filter(typereal fc, unsigned int num_pole, unsigned int num_chann, int highpass);
const dfilter* create_chebychev_filter(typereal fc, unsigned int num_pole, unsigned int nchann, int highpass, typereal ripple);
const dfilter* create_integrate_filter(unsigned int nchann);

#endif /*COMMON_FILTERS_H*/
