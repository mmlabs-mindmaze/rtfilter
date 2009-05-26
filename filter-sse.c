#include "filter.h"
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#ifdef __SSE__
#include <xmmintrin.h>
#endif


void* align_alloc(size_t alignment, size_t size)
{
	void* memptr = NULL;
	if (posix_memalign(&memptr, alignment, size))
		return NULL;
	return memptr;
}

void  align_free(void* memptr)
{
	free(memptr);
}



dfilter* create_dfilter(unsigned int nchann, unsigned int a_len, const float *num, unsigned int b_len, const float *denum)
{
	dfilter *filt = NULL;
	float *a = NULL;
	float *xoff = NULL;
	float *b = NULL;
	float *yoff = NULL;
	int xoffsize, yoffsize;

	xoffsize = (a_len - 1) * nchann;
	yoffsize = b_len * nchann;

	filt = malloc(sizeof(*filt));
	a = (a_len > 0) ? malloc(a_len * sizeof(*num)) : NULL;
	b = (b_len > 0) ? malloc(b_len * sizeof(*denum)) : NULL;
	if (xoffsize > 0) 
		xoff = align_alloc(16,xoffsize * sizeof(*xoff));
	if (yoffsize > 0) 
		yoff = align_alloc(16,yoffsize * sizeof(*yoff));

	// handle memory allocation problem
	if (!filt || ((a_len > 0) && !a) || ((xoffsize > 0) && !xoff)
	    || ((b_len > 0) && !b) || ((yoffsize > 0) && !yoff)) {
		free(filt);
		free(a);
		align_free(xoff);
		free(b);
		align_free(yoff);
		return NULL;
	}

	memset(filt, 0, sizeof(*filt));

	// prepare the filt struct
	filt->a = a;
	filt->num_chann = nchann;
	filt->a_len = a_len;
	filt->xoff = xoff;
	filt->b = b;
	filt->b_len = b_len;
	filt->yoff = yoff;

	// copy the numerator and denumerator
	memcpy(a, num, a_len * sizeof(*num));
	memcpy(b, denum, b_len * sizeof(*denum));

	reset_filter(filt);

	return filt;
}

void destroy_filter(dfilter * filt)
{
	if (!filt)
		return;

	free((void *) (filt->a));
	free((void *) (filt->b));
	align_free(filt->xoff);
	align_free(filt->yoff);
	free(filt);
}

void reset_filter(dfilter * filt)
{
	memset(filt->xoff, 0,
	       (filt->a_len -
		1) * filt->num_chann * sizeof(*(filt->xoff)));
	memset(filt->yoff, 0,
	       (filt->b_len) * filt->num_chann * sizeof(*(filt->yoff)));
}


void filter(dfilter* filt, const float* in, float* out, int nsamples)
{
	int i, k, ichann, io, ii, num;
	const float* x;
	const float* y;
	int a_len = filt->a_len;
	const float* a = filt->a;
	int b_len = filt->b_len;
	const float* b = filt->b;
	int nchann = filt->num_chann;
	const float* xprev = filt->xoff + (a_len-1)*nchann;
	const float* yprev = filt->yoff + b_len*nchann;


	if (!nchann)
		return;

	// compute the product of convolution of the input with the finite
	// impulse response (fir)
	for (i=0; i<nsamples; i++) {
		io = i*nchann;
		memset(out+io, 0, nchann*sizeof(*out));

		for (k=0; k<a_len; k++) {
			ii = (i-k)*nchann;

			// If the convolution must be done with samples not
			// provided, use the stored ones
			x = (ii >= 0) ? in : xprev;
			
			for (ichann=0; ichann<nchann; ichann++)
				out[io+ichann] += a[k]*x[ii+ichann];
		}

		// compute the convolution in the denominator
		for (k=0; k<b_len; k++) {
			ii = (i-k-1)*nchann;

			// If the convolution must be done with samples not
			// provided, use the stored ones
			y = (ii>=0) ? out : yprev;
			
			for (ichann=0; ichann<nchann; ichann++)
				out[io+ichann] += b[k]*y[ii+ichann];
		}
	}

	// store the last input sample
	if (((int)a_len)-1>=0) {
		num = a_len-1 - nsamples;
		if (num > 0)
			memmove(filt->xoff, filt->xoff + nsamples*nchann, num*nchann*sizeof(*in));
		else
			num = 0;
		memcpy(filt->xoff + num*nchann, in+(nsamples-a_len+1+num)*nchann, (a_len-1-num)*nchann*sizeof(*in));
	}

	// store the last output sample
	if (b_len) {
		num = b_len - nsamples;
		if (num > 0)
			memmove(filt->yoff, filt->yoff + nsamples*nchann, num*nchann*sizeof(*out));
		else
			num = 0;
		memcpy(filt->yoff + num*nchann, out+(nsamples-b_len+num)*nchann, (b_len-num)*nchann*sizeof(*out));
	}
}


void filtera(dfilter * filt, const __m128 *in, __m128 *out, int nsamples)
{
	int i, k, ichann, ii;
	const __m128 *x, *y;
	int a_len = filt->a_len;
	const float *a = filt->a;
	int b_len = filt->b_len;
	const float *b = filt->b;
	int nchann = filt->num_chann/4;
	const __m128 *xprev = (__m128*)(filt->xoff) + (a_len - 1) * nchann;
	const __m128 *yprev = (__m128*)(filt->yoff) + b_len * nchann;
	__m128 coef, *currout, *dest;
	const __m128* src;


	if (!nchann)
		return;

	// compute the product of convolution of the input with the finite
	// impulse response (fir)
	for (i = 0; i < nsamples; i++) {
		currout = out + i*nchann;

		// Init to convolution to 0
		for (ichann=0; ichann<nchann; ichann++)
			currout[ichann] = _mm_setzero_ps();

		// Compute the convolution with numerator
		for (k = 0; k < a_len; k++) {
			ii = (i - k) * nchann;
			coef = _mm_set1_ps(a[k]);

			// If the convolution must be done with samples not
			// provided, use the stored ones
			x = (ii >= 0) ? in : xprev;

			for (ichann = 0; ichann < nchann; ichann++)
				currout[ichann] = _mm_add_ps(_mm_mul_ps(coef,x[ii+ichann]),currout[ichann]);
		}

		// compute the convolution in the denominator
		for (k = 0; k < b_len; k++) {
			ii = (i - k - 1) * nchann;
			coef = _mm_set1_ps(b[k]);

			// If the convolution must be done with samples not
			// provided, use the stored ones
			y = (ii >= 0) ? out : yprev;

			for (ichann = 0; ichann < nchann; ichann++)
				currout[ichann] = _mm_add_ps(_mm_mul_ps(coef,y[ii+ichann]),currout[ichann]);
		}
	}

	/*
	// store the last input sample
	if (((int) a_len) - 1 >= 0) {
		num = a_len - 1 - nsamples;
		if (num > 0)
			memmove(filt->xoff, filt->xoff + nsamples * nchann,
				num * nchann * sizeof(*in));
		else
			num = 0;
		memcpy(filt->xoff + num * nchann,
		       in + (nsamples - a_len + 1 + num) * nchann,
		       (a_len - 1 - num) * nchann * sizeof(*in));
	}
	// store the last output sample
	if (b_len) {
		num = b_len - nsamples;
		if (num > 0)
			memmove(filt->yoff, filt->yoff + nsamples * nchann,
				num * nchann * sizeof(*out));
		else
			num = 0;
		memcpy(filt->yoff + num * nchann,
		       out + (nsamples - b_len + num) * nchann,
		       (b_len - num) * nchann * sizeof(*out));
	}*/
	dest = (__m128*)(filt->xoff);
	src = (__m128*)(filt->xoff)+nsamples*nchann;
	for (i=0; i<(a_len-1)*nchann; i++) {
		if (i>(a_len-1-nsamples)*nchann)
			src = in+(nsamples-a_len-1)*nchann;
		dest[i] = src[i];
	}
	dest = (__m128*)(filt->yoff);
	src = (__m128*)(filt->yoff)+nsamples*nchann;
	for (i=0; i<(b_len*nchann); i++) {
		if (i>(b_len-nsamples)*nchann)
			src = out+(nsamples-b_len)*nchann;
		dest[i] = src[i];
	}
}

