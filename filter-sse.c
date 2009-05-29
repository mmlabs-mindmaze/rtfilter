#include "filter.h"
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef USE_DOUBLE

# define get_num_padded_chann(numchann)	(((int)(numchann)+1) & ~1)
# define get_num_chgrp(numchann)	(get_num_padded_chann(numchann)/2)
# define add_vec(v1,v2)			_mm_add_pd(v1,v2)
# define mul_vec(v1,v2)			_mm_mul_pd(v1,v2)
# define zero_vec()			_mm_setzero_pd()
# define set1_vec(data)			_mm_set1_pd(data)
typedef __m128d  typereal_a;

#else

# define get_num_padded_chann(numchann)	(((int)(numchann)+3) & ~3)
# define get_num_chgrp(numchann)	(get_num_padded_chann(numchann)/4)
# define add_vec(v1,v2)			_mm_add_ps(v1,v2)
# define mul_vec(v1,v2)			_mm_mul_ps(v1,v2)
#define zero_vec()			_mm_setzero_ps()
# define set1_vec(data)			_mm_set1_ps(data)
typedef __m128  typereal_a;

#endif


struct _dfilter
{
	unsigned int num_chann;
	unsigned int a_len;
	const typereal* a;
	unsigned int b_len;
	const typereal* b;
	typereal* xoff;
	typereal* yoff;
};


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


hfilter create_dfilter(unsigned int nchann, unsigned int a_len, const typereal *num, unsigned int b_len, const typereal *denum)
{
	struct _dfilter *filt = NULL;
	typereal *a = NULL;
	typereal *xoff = NULL;
	typereal *b = NULL;
	typereal *yoff = NULL;
	int xoffsize, yoffsize;
	int nvchann;

	// Compute the nearest bigger multiple of 
	nvchann = get_num_padded_chann(nchann);
	
	xoffsize = (a_len - 1) * nvchann;
	yoffsize = b_len * nvchann;

	filt = malloc(sizeof(*filt));
	a = (a_len > 0) ? malloc(a_len * sizeof(*num)) : NULL;
	b = (b_len > 0) ? malloc(b_len * sizeof(*denum)) : NULL;
	if (xoffsize > 0) 
		xoff = align_alloc(16,xoffsize * sizeof(typereal));
	if (yoffsize > 0) 
		yoff = align_alloc(16,yoffsize * sizeof(typereal));

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


void destroy_filter(hfilter filt)
{
	if (!filt)
		return;

	free((void *) (filt->a));
	free((void *) (filt->b));
	align_free(filt->xoff);
	align_free(filt->yoff);
	free((void*) filt);
}


void reset_filter(hfilter filt)
{
	memset(filt->xoff, 0,
	       (filt->a_len -
		1) * filt->num_chann * sizeof(*(filt->xoff)));
	memset(filt->yoff, 0,
	       (filt->b_len) * filt->num_chann * sizeof(*(filt->yoff)));
}


void filter(hfilter filt, const typereal* in, typereal* out, int nsamples)
{
	int i, k, ichann, io, ii, num;
	const typereal* x;
	const typereal* y;
	int a_len = filt->a_len;
	const typereal* a = filt->a;
	int b_len = filt->b_len;
	const typereal* b = filt->b;
	int nchann = filt->num_chann;
	const typereal* xprev = filt->xoff + (a_len-1)*nchann;
	const typereal* yprev = filt->yoff + b_len*nchann;


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



void filtera(hfilter filt, const typereal *xaligned, typereal *yaligned, unsigned int nsamples)
{
	int i, k, ichann, ii, len, midlen;
	const typereal_a *x, *y;

	int a_len = filt->a_len;
	const typereal *a = filt->a;
	int b_len = filt->b_len;
	const typereal *b = filt->b;
	int nchann = get_num_chgrp(filt->num_chann);
	const typereal_a *xprev = (typereal_a*)(filt->xoff) + (a_len - 1) * nchann;
	const typereal_a *yprev = (typereal_a*)(filt->yoff) + b_len * nchann;
	typereal_a coef, *currout, *dest;
	const typereal_a* src;

	const typereal_a *in = (typereal_a*)xaligned;
	typereal_a *out = (typereal_a*)yaligned;


	if (!nchann)
		return;

	// Init to convolution to 0
	//memset(out, 0, nchann*nsamples*sizeof(*out));
	for (i=0; i<nchann*nsamples; i++)
		out[i] = zero_vec();


	// compute the product of convolution of the input with the infinite
	// impulse response (iir)
	for (i = 0; i < nsamples; i++) {
		currout = out + i*nchann;


		// Compute the convolution with numerator
		for (k = 0; k < a_len; k++) {
			ii = (i - k) * nchann;
			coef = set1_vec(a[k]);

			// If the convolution must be done with samples not
			// provided, use the stored ones
			x = (ii >= 0) ? in : xprev;

			for (ichann = 0; ichann < nchann; ichann++)
				currout[ichann] = add_vec(mul_vec(coef,x[ii+ichann]),currout[ichann]);
		}

		// compute the convolution in the denominator
		for (k = 0; k < b_len; k++) {
			ii = (i - k - 1) * nchann;
			coef = set1_vec(b[k]);

			// If the convolution must be done with samples not
			// provided, use the stored ones
			y = (ii >= 0) ? out : yprev;

			for (ichann = 0; ichann < nchann; ichann++)
				currout[ichann] = add_vec(mul_vec(coef,y[ii+ichann]),currout[ichann]);
		}
	}

	// Store the latest input samples
	dest = (typereal_a*)(filt->xoff);
	len = (a_len-1)*nchann;
	midlen = (a_len-1-nsamples)*nchann;
	if (midlen > 0) {
		src = dest + nsamples*nchann;
		memmove(dest, src, midlen*sizeof(*src));
		dest += midlen;
		len -= midlen;
		src = in;
	} else
		src = in-midlen;
	memcpy(dest, src, len*sizeof(*src));
	
	
	// Store the latest output samples
	dest = (typereal_a*)(filt->yoff);
	len = b_len*nchann;
	midlen = (b_len-nsamples)*nchann;
	if (midlen > 0) {
		src = dest + nsamples*nchann;
		memmove(dest, src, midlen*sizeof(*src));
		dest += midlen;
		len -= midlen;
		src = out;
	} else
		src = out-midlen;
	memcpy(dest, src, len*sizeof(*src));

}
