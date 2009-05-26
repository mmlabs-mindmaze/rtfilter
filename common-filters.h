#ifndef COMMON_FILTERS_H
#define COMMON_FILTERS_H

#include <filter.h>

typedef enum {
	BLACKMAN_WINDOW,
	HAMMING_WINDOW,
	RECT_WINDOW
} KernelWindow;

dfilter* create_fir_filter_mean(unsigned int nsamples, unsigned int nchann);
dfilter* create_fir_filter_lowpass(float fc, unsigned int half_length, unsigned int nchann, KernelWindow window);
dfilter* create_fir_filter_highpass(float fc, unsigned int half_length, unsigned int nchann, KernelWindow window);
dfilter* create_fir_filter_bandpass(float fc_low, float fc_high, unsigned int half_length, unsigned int nchann, KernelWindow window);
dfilter* create_butterworth_filter(float fc, unsigned int num_pole, unsigned int num_chann, int highpass);
dfilter* create_chebychev_filter(float fc, unsigned int num_pole, unsigned int nchann, int highpass, float ripple);
dfilter* create_integrate_filter(unsigned int nchann);

#endif /*COMMON_FILTERS_H*/
