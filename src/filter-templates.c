/*
      Copyright (C) 2008-2009 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    This file is part of the rtfilter library

    The eegpanel library is free software: you can redistribute it and/or
    modify it under the terms of the version 3 of the GNU General Public
    License as published by the Free Software Foundation.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#undef NELEM_VEC
#define NELEM_VEC	(sizeof(TYPEREAL_V)/sizeof(TYPEREAL))


hfilter CREATE_FILTER_FUNC(unsigned int nchann, unsigned int a_len, const TYPEREAL *num, unsigned int b_len, const TYPEREAL *denum, unsigned int type)
{
	unsigned int i;
	TYPEREAL normfactor;
	struct _dfilter *filt = NULL;
	void *a = NULL;
	void *xoff = NULL;
	void *b = NULL;
	void *yoff = NULL;
	int xoffsize, yoffsize;

	// Check if a denominator exists
	if ((b_len==0) || (denum==NULL)) {
		b_len = 0;
		normfactor = 1.0;
	}
	else {
		b_len--;
		normfactor = denum[0];
	}

	xoffsize = (a_len - 1) * nchann;
	yoffsize = b_len * nchann;

	filt = malloc(sizeof(*filt));
	a = (a_len > 0) ? malloc(a_len * sizeof_data(type)) : NULL;
	b = (b_len > 0) ? malloc(b_len * sizeof_data(type)) : NULL;
	if (xoffsize > 0) 
		xoff = align_alloc(16, xoffsize * sizeof_data(type));
	if (yoffsize > 0) 
		yoff = align_alloc(16, yoffsize * sizeof_data(type));

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
	filt->num_chann = nchann;
	filt->type = type;
	filt->a = a;
	filt->a_len = a_len;
	filt->xoff = xoff;
	filt->b = b;
	filt->b_len = b_len;
	filt->yoff = yoff;

	// copy the numerator and denumerator 
	// (and normalize and convert to recursive rule)
	if (type == DATATYPE_FLOAT) {
		float *af = a, *bf = b;
		for (i=0; i<a_len; i++)
			af[i] = num[i] / normfactor;
		for (i=0; i<b_len; i++)
			bf[i] = -denum[i+1] / normfactor;
	}
	else {
		float *ad = a, *bd = b;
		for (i=0; i<a_len; i++)
			ad[i] = num[i] / normfactor;
		for (i=0; i<b_len; i++)
			bd[i] = -denum[i+1] / normfactor;
	}

	reset_filter(filt);

	return filt;
}


static void FILTER_UNALIGNED_FUNC(hfilter filt, const TYPEREAL* in, TYPEREAL* out, unsigned int nsamples)
{
	unsigned int i;
	int k, ichann, ii, num;
	const TYPEREAL* x;
	const TYPEREAL* y;
	int a_len = filt->a_len;
	const TYPEREAL* a = filt->a;
	int b_len = filt->b_len;
	const TYPEREAL* b = filt->b;
	int nchann = filt->num_chann;
	const TYPEREAL* xprev = (TYPEREAL*)(filt->xoff) + (a_len-1)*nchann;
	const TYPEREAL* yprev = (TYPEREAL*)(filt->yoff) + b_len*nchann;
	TYPEREAL* currout;


	if (!nchann)
		return;

	// Init to convolution to 0
	memset(out, 0, nchann*nsamples*sizeof(*out));

	// compute the product of convolution of the input with the finite
	// impulse response (fir)
	for (i=0; i<nsamples; i++) {
		currout = out + i*nchann;

		for (k=0; k<a_len; k++) {
			ii = (i-k)*nchann;

			// If the convolution must be done with samples not
			// provided, use the stored ones
			x = (ii >= 0) ? in : xprev;
			
			for (ichann=0; ichann<nchann; ichann++)
				currout[ichann] += a[k]*x[ii+ichann];
		}

		// compute the convolution in the denominator
		for (k=0; k<b_len; k++) {
			ii = (i-k-1)*nchann;

			// If the convolution must be done with samples not
			// provided, use the stored ones
			y = (ii>=0) ? out : yprev;
			
			for (ichann=0; ichann<nchann; ichann++)
				currout[ichann] += b[k]*y[ii+ichann];
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



#ifdef USE_SIMD
static void FILTER_ALIGNED_FUNC(hfilter filt, const TYPEREAL *xaligned, TYPEREAL *yaligned, unsigned int nsamples)
{
	unsigned int i;
	int k, ichann, ii, len, midlen;
	const TYPEREAL_V *x, *y;

	int a_len = filt->a_len;
	const TYPEREAL *a = filt->a;
	int b_len = filt->b_len;
	const TYPEREAL *b = filt->b;
	int nchann = filt->num_chann / NELEM_VEC;
	const TYPEREAL_V *xprev = (TYPEREAL_V*)(filt->xoff) + (a_len - 1) * nchann;
	const TYPEREAL_V *yprev = (TYPEREAL_V*)(filt->yoff) + b_len * nchann;
	TYPEREAL_V coef, *currout, *dest;
	const TYPEREAL_V* src;

	const TYPEREAL_V *in = (TYPEREAL_V*)xaligned;
	TYPEREAL_V *out = (TYPEREAL_V*)yaligned;


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
	dest = (TYPEREAL_V*)(filt->xoff);
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
	dest = (TYPEREAL_V*)(filt->yoff);
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
#endif //USE_SIMD

void FILTER_FUNC(hfilter filt, const TYPEREAL* in, TYPEREAL* out, unsigned int nsamples)
{
#ifdef USE_SIMD
	// Check that data is aligned on 16 byte boundaries
	if ( !((filt->num_chann%NELEM_VEC) 
		|| ((uintptr_t)in % sizeof(TYPEREAL_V)) 
		|| ((uintptr_t)out % sizeof(TYPEREAL_V))) )
		FILTER_ALIGNED_FUNC(filt, in, out, nsamples);
	else
#endif //USE_SIMD
		FILTER_UNALIGNED_FUNC(filt, in, out, nsamples);
}

