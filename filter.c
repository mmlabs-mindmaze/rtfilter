#include "filter.h"
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#ifdef __SSE__
#include <xmmintrin.h>
#endif

#define PId	3.1415926535897932384626433832795L
#define PIf	3.1415926535897932384626433832795f

// TODO change the data type used for filter function (use double) to improve stability of recursive filters

void apply_window(float* fir, unsigned int length, KernelWindow window)
{
	unsigned int i;
	float M = length-1;

	switch (window) {
	case HAMMING_WINDOW:
		for (i=0; i<length; i++)
			fir[i] *= 0.54 + 0.46*cos(2.0*PIf*((float)i/M - 0.5));
		break;

	case BLACKMAN_WINDOW:
		for (i=0; i<length; i++)
			fir[i] *= 0.42 + 0.5*cos(2.0*PIf*((float)i/M - 0.5)) + 0.08*cos(4.0*PIf*((float)i/M - 0.5));
		break;

	case RECT_WINDOW:
		break;
	}
}




void normalize_fir(float* fir, unsigned int length)
{
	unsigned int i;
	double sum = 0.0;

	for (i=0; i<length; i++)
		sum += fir[i];

	for (i=0; i<length; i++)
		fir[i] /= sum;
}

void compute_convolution(float* product, float* sig1, unsigned int len1, float* sig2, unsigned int len2)
{
	unsigned int i,j;

	memset(product, 0, (len1+len2-1)*sizeof(*product));
	
	for (i=0; i<len1; i++)
		for (j=0; j<len2; j++)
			product[i+j] += sig1[i]*sig2[j];
}


void compute_fir_lowpass(float* fir, unsigned int length, float fc)
{
	unsigned int i;
	float half_len = (float)((unsigned int)(length/2));

	for (i=0; i<length; i++)
		if (i != length/2)
			fir[i] = sin(2.0*PIf*(float)fc*((float)i-half_len)) / ((float)i-half_len);
		else
			fir[i] = 2.0*PIf*fc;
}

void reverse_fir(float* fir, unsigned int length)
{
	unsigned int i;

	// compute delay minus lowpass fir
	for (i=0; i<length; i++)
		fir[i] = -1.0 * fir[i];
	fir[length-1] += 1.0;
}

// inspired by DSP guide ch33
int compute_cheby_iir(float* num, float* den, unsigned int num_pole, int highpass, float ripple, float cutoff_freq)
{
	double *a, *b, *ta, *tb;
	double a0, a1, a2, b1, b2;
	double rp, ip, es, vx, kx, t, w, m, d, x0, x1, x2, y1, y2, k;
	double sa, sb, gain;
	double np = num_pole;
	unsigned int i, p;
	double fc = cutoff_freq, r = ripple;
	int retval = 1;
	
	// Allocate temporary arrays
	a = malloc((num_pole+3)*sizeof(*a));
	b = malloc((num_pole+3)*sizeof(*b));
	ta = malloc((num_pole+3)*sizeof(*ta));
	tb = malloc((num_pole+3)*sizeof(*tb));
	if (!a || !b || !ta || !tb) {
		retval = 0;
		goto exit;
	}
	memset(a, 0, (num_pole+3)*sizeof(*a));
	memset(b, 0, (num_pole+3)*sizeof(*b));

	a[2] = 1.0;
	b[2] = 1.0;

	for (p=1; p<=num_pole/2; p++) {
		// calculate pole locate on the unit circle
		rp = -cos(PId/(np*2.0) + ((double)(p-1))*PId/np);
		ip = sin(PId/(np*2.0) + ((double)(p-1))*PId/np);

		// Warp from a circle to an ellipse
		if (r != 0.0) {
			es = sqrt(pow(1.0/(1.0-r),2)-1.0);
			vx = (1.0/np)*log( (1.0/es) + sqrt((1.0/(es*es))+1.0) );
			kx = (1.0/np)*log( (1.0/es) + sqrt((1.0/(es*es))-1.0) );
			kx = (exp(kx)+exp(-kx))/2.0;
			rp = rp * ((exp(vx)-exp(-vx))/2.0)/kx;
			ip = ip * ((exp(vx)+exp(-vx))/2.0)/kx;
		}

		// s to z domains conversion
		t = 2.0*tan(0.5);
		w = 2.0*PId*fc;
		m = rp*rp + ip*ip;
		d = 4.0 - 4.0*rp*t + m*t*t;
		x0 = t*t/d;
		x1 = 2.0*t*t/d;
		x2 = t*t/d;
		y1 = (8.0 - 2.0*m*t*t)/d;
		y2 = (-4.0 - 4.0*rp*t - m*t*t)/d;

		// LP(s) to LP(z) or LP(s) to HP(z)
		if (highpass)
			k = -cos(w/2.0 + 0.5)/cos(w/2.0 - 0.5);
		else
			k = sin(0.5 - w/2.0)/sin(0.5 + w/2.0);
		d = 1.0 + y1*k - y2*k*k;
		a0 = (x0 - x1*k + x2*k*k)/d;
		a1 = (-2.0*x0*k + x1 + x1*k*k - 2.0*x2*k)/d;
		a2 = (x0*k*k - x1*k +x2)/d;
		b1 = (2.0*k + y1 + y1*k*k - 2.0*y2*k)/d;
		b2 = (-k*k - y1*k + y2)/d;
		if (highpass) {
			a1 *= -1.0;
			b1 *= -1.0;
		}

		// Add coefficients to the cascade
		memcpy(ta, a, (num_pole+3)*sizeof(*a));
		memcpy(tb, b, (num_pole+3)*sizeof(*b));
		for (i=2; i<=num_pole+2; i++) {
			a[i] = a0*ta[i] + a1*ta[i-1] + a2*ta[i-2];
			b[i] = tb[i] + b1*tb[i-1] + b2*tb[i-2];
		}
	}

	// Finish combining coefficients
	b[2] = 0;
	for (i=0; i<=num_pole; i++) {
		a[i] = a[i+2];
		b[i] = b[i+2];
	}

	// Normalize the gain
	sa = sb = 0.0;
	for (i=0; i<=num_pole; i++) {
		sa += a[i] * ((highpass && i%2) ? -1.0 : 1.0);
		sb += b[i] * ((highpass && i%2) ? -1.0 : 1.0);
	}
	gain = sa / (1.0-sb);
	for (i=0; i<=num_pole; i++) 
		a[i] /= gain;

	
	// Copy the results to the num and den
	num[0] = a[0];
	for (i=1; i<=num_pole; i++) {
		num[i] = a[i];
		den[i-1] = b[i];
	}

exit:
	free(a);
	free(b);
	free(ta);
	free(tb);
	return retval;
}


dfilter* create_fir_filter(unsigned int fir_length, unsigned int nchann, float** fir_out)
{
	dfilter* filt = NULL;
	float* fir = NULL;
	float* off = NULL;

	filt = malloc(sizeof(*filt));
	fir = malloc(fir_length*sizeof(*fir));
	off = malloc((fir_length-1)*nchann*sizeof(*off));

	// handle memory allocation problem
	if (!filt || !fir || !off) {
		free(filt);
		free(fir);
		free(off);
		return NULL;
	}

	memset(filt, 0, sizeof(*filt));
	memset(off, 0, (fir_length-1)*nchann*sizeof(*off)); 
	
	// prepare the filt struct
	filt->a = fir;
	filt->num_chann = nchann;
	filt->a_len = fir_length;
	filt->xoff = off;
	
	if (fir_out)
		*fir_out = fir;

	return filt;
}

dfilter* create_iir_filter(int a_len, int b_len, unsigned int nchann, float** a_out, float** b_out)
{
	dfilter* filt = NULL;
	float* a = NULL;
	float* xoff = NULL;
	float* b = NULL;
	float* yoff = NULL;
	int xoffsize, yoffsize;

	xoffsize = (a_len-1)*nchann;
	yoffsize = b_len*nchann;

	filt = malloc(sizeof(*filt));
	a = (a_len>0) ? malloc(a_len*sizeof(*a)) : NULL;
	b = (b_len>0) ? malloc(b_len*sizeof(*b)) : NULL;
	xoff = (xoffsize>0) ? malloc(xoffsize*sizeof(*xoff)) : NULL;
	yoff = (yoffsize>0) ? malloc(yoffsize*sizeof(*yoff)) : NULL;

	// handle memory allocation problem
	if (!filt || ((a_len>0) && !a) || ((xoffsize>0) && !xoff) 
		  || ((b_len>0) && !b) || ((yoffsize>0) && !yoff)) {
		free(filt);
		free(a);
		free(xoff);
		free(b);
		free(yoff);
		return NULL;
	}

	memset(filt, 0, sizeof(*filt));
	memset(xoff, 0, xoffsize*sizeof(*xoff)); 
	memset(yoff, 0, yoffsize*sizeof(*yoff)); 
	
	// prepare the filt struct
	filt->a = a;
	filt->num_chann = nchann;
	filt->a_len = a_len;
	filt->xoff = xoff;
	filt->b = b;
	filt->b_len = b_len;
	filt->yoff = yoff;
	
	if (a_out)
		*a_out = a;
	if (b_out)
		*b_out = b;

	return filt;
}

void destroy_filter(dfilter* filt)
{
	if (!filt)
		return;

	free((void*)(filt->a));
	free((void*)(filt->b));
	free(filt->xoff);
	free(filt->yoff);
	free(filt);
}

void reset_filter(dfilter* filt)
{
	memset(filt->xoff, 0, (filt->a_len-1)*filt->num_chann*sizeof(*(filt->xoff)));
	memset(filt->yoff, 0, (filt->b_len)*filt->num_chann*sizeof(*(filt->yoff)));
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
	filt->curr_sample += nsamples;
}


///////////////////////////////////////////////////////////////////////////////
//
//			Create particular filters
//
///////////////////////////////////////////////////////////////////////////////
dfilter* create_fir_filter_mean(unsigned int fir_length, unsigned int nchann)
{
	unsigned int i;
	float* fir = NULL;
	dfilter* filt;

	filt = create_fir_filter(fir_length, nchann, &fir);
	if (!filt)
		return NULL;

	// prepare the finite impulse response
	for (i=0; i<fir_length; i++)
		fir[i] = 1.0f/(float)fir_length;

	return filt;
}

dfilter* create_fir_filter_lowpass(float fc, unsigned int half_length, unsigned int nchann, KernelWindow window)
{
	float* fir = NULL;
	dfilter* filt;
	unsigned int fir_length = 2*half_length + 1;

	filt = create_fir_filter(fir_length, nchann, &fir);
	if (!filt)
		return NULL;

	// prepare the finite impulse response
	compute_fir_lowpass(fir, fir_length, fc);
	apply_window(fir, fir_length, window);
	normalize_fir(fir, fir_length);

	return filt;
}


dfilter* create_fir_filter_highpass(float fc, unsigned int half_length, unsigned int nchann, KernelWindow window)
{
	float* fir = NULL;
	dfilter* filt;
	unsigned int fir_length = 2*half_length + 1;

	filt = create_fir_filter(fir_length, nchann, &fir);
	if (!filt)
		return NULL;

	// prepare the finite impulse response
	compute_fir_lowpass(fir, fir_length, fc);
	apply_window(fir, fir_length, window);
	normalize_fir(fir, fir_length);
	reverse_fir(fir, fir_length);

	return filt;
}


dfilter* create_fir_filter_bandpass(float fc_low, float fc_high, unsigned int half_length, unsigned int nchann, KernelWindow window)
{
	unsigned int len = 2*(half_length/2)+1;
	float fir_low[len], fir_high[len];
	float* fir = NULL;
	dfilter* filt;
	unsigned int fir_length = 2*half_length + 1;
	

	filt = create_fir_filter(fir_length, nchann, &fir);
	if (!filt)
		return NULL;

	// Create the lowpass finite impulse response
	compute_fir_lowpass(fir_low, len, fc_low);
	apply_window(fir_low, len, window);
	normalize_fir(fir_low, len);

	// Create the highpass finite impulse response
	compute_fir_lowpass(fir_high, len, fc_high);
	apply_window(fir_high, len, window);
	normalize_fir(fir_high, len);
	reverse_fir(fir_high, len);

	// compute the convolution product of the two FIR
	compute_convolution(fir, fir_low, len, fir_high, len);

	return filt;
}


dfilter* create_chebychev_filter(float fc, unsigned int num_pole, unsigned int nchann, int highpass, float ripple)
{
	float *a = NULL, *b = NULL;
	dfilter* filt;
	
	if (num_pole%2 != 0)
		return NULL;

	filt = create_iir_filter(num_pole+1, num_pole, nchann, &a, &b);
	if (!filt)
		return NULL;

	// prepare the filter
	if (!compute_cheby_iir(a, b, num_pole, highpass, ripple, fc)) {
		destroy_filter(filt);
		return NULL;
	}

	return filt;
}

dfilter* create_butterworth_filter(float fc, unsigned int num_pole, unsigned int num_chann, int highpass)
{
	return create_chebychev_filter(fc, num_pole, num_chann, highpass, 0.0);
}

dfilter* create_integrate_filter(unsigned int nchann)
{
	float *a = NULL, *b = NULL;
	dfilter* filt;
	
	filt = create_iir_filter(1, 1, nchann, &a, &b);
	if (!filt)
		return NULL;

	// prepare the filter
	a[0] = 1.0;
	b[0] = 1.0;

	return filt;
}

dfilter* create_adhoc_filter(unsigned int nchann)
{
	
	float *a = NULL, *b = NULL;
	dfilter* filt;
	
	filt = create_iir_filter(5, 4, nchann, &a, &b);
	if (!filt)
		return NULL;

	/*a[0] = 1.0;
	a[1] = 2.0;
	a[1] = 1.0;
	b[0] = 1.9948781315;
	b[1] = -0.9968542274;*/

	//works
	/*a[0] = 0.3493e-4;
	a[1] = 0.6987e-4;
	a[2] = 0.3493e-4;
	b[0] = 1.9949;
	b[1] = -0.9969;*/

	a[0] = 0.0862e-7;
	a[1] = 0.3449e-7;
	a[2] = 0.5173e-7;
	a[3] = 0.3449e-7;
	a[4] = 0.0862e-7;
	b[0] = -3.9931;
	b[1] = 5.9834;
	b[2] = -3.9873;
	b[3] = 0.9971;
	return filt;
}

