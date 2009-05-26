#ifndef FILTER_H
#define FILTER_H

#include <xmmintrin.h>

typedef struct _dfilter
{
	unsigned int num_chann;
	unsigned int a_len;
	const float* a;
	unsigned int b_len;
	const float* b;
	float* xoff;
	float* yoff;
} dfilter;

dfilter* create_dfilter(unsigned int nchann, unsigned int alen, const float *a, unsigned int blen, const float *b);
void filter(dfilter* filt, const float* x, float* y, int num_samples);
void filtera(dfilter* filt, const __m128* x, __m128* y, int num_samples);
void reset_filter(dfilter* filt);
void destroy_filter(dfilter* filt);

#endif //FILTER_H
