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
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include "filter.h"
#include "common-filters.h"

#define PId	3.1415926535897932384626433832795L
#define PIf	3.1415926535897932384626433832795f

// TODO change the data type used for filter function (use double) to improve stability of recursive filters

static void apply_window(double *fir, unsigned int length, KernelWindow window)
{
	unsigned int i;
	double M = length - 1;

	switch (window) {
	case HAMMING_WINDOW:
		for (i = 0; i < length; i++)
			fir[i] *= 0.54 + 0.46 * cos(2.0 * PIf * ((double)i / M - 0.5));
		break;

	case BLACKMAN_WINDOW:
		for (i = 0; i < length; i++)
			fir[i] *=
			    0.42 +
			    0.5 * cos(2.0 * PIf * ((double) i / M - 0.5)) +
			    0.08 * cos(4.0 * PIf * ((double) i / M - 0.5));
		break;

	case RECT_WINDOW:
		break;
	}
}




static void normalize_fir(double *fir, unsigned int length)
{
	unsigned int i;
	double sum = 0.0;

	for (i = 0; i < length; i++)
		sum += fir[i];

	for (i = 0; i < length; i++)
		fir[i] /= sum;
}

static void compute_convolution(double *product, double *sig1, unsigned int len1,
			 double *sig2, unsigned int len2)
{
	unsigned int i, j;

	memset(product, 0, (len1 + len2 - 1) * sizeof(*product));

	for (i = 0; i < len1; i++)
		for (j = 0; j < len2; j++)
			product[i + j] += sig1[i] * sig2[j];
}


static void compute_fir_lowpass(double *fir, unsigned int length, double fc)
{
	unsigned int i;
	double half_len = (double) ((unsigned int) (length / 2));

	for (i = 0; i < length; i++)
		if (i != length / 2)
			fir[i] =
			    sin(2.0 * PIf * (double) fc *
				((double) i - half_len)) / ((double) i -
							   half_len);
		else
			fir[i] = 2.0 * PIf * fc;
}

static void reverse_fir(double *fir, unsigned int length)
{
	unsigned int i;

	// compute delay minus lowpass fir
	for (i = 0; i < length; i++)
		fir[i] = -1.0 * fir[i];
	fir[length - 1] += 1.0;
}

// inspired by DSP guide ch33
//

static void get_pole_coefs(double p, double np, double fc, double r, int highpass, double a[3], double b[3])
{
	double rp, ip, es, vx, kx, t, w, m, d, x0, x1, x2, y1, y2, k;

	// calculate pole locate on the unit circle
	rp = -cos(PId / (np * 2.0) + (p - 1.0) * PId / np);
	ip = sin(PId / (np * 2.0) + (p - 1.0) * PId / np);

	// Warp from a circle to an ellipse
	if (r != 0.0) {
	/*	es = sqrt(pow(1.0 / (1.0 - r), 2) - 1.0);
		vx = (1.0 / np) * log((1.0 / es) +
				      sqrt((1.0 / (es * es)) +
					   1.0));
		kx = (1.0 / np) * log((1.0 / es) +
				      sqrt((1.0 / (es * es)) -
					   1.0));
		kx = (exp(kx) + exp(-kx)) / 2.0;
		rp = rp * ((exp(vx) - exp(-vx)) / 2.0) / kx;
		ip = ip * ((exp(vx) + exp(-vx)) / 2.0) / kx;*/
	
		es = sqrt(pow(1.0 / (1.0 - r), 2) - 1.0);
		vx = asinh(1/es) / np;
		kx = acosh(1/es) / np;
		kx = cosh( kx );
		rp = rp * sinh(vx) / kx;
		ip = ip * cosh(vx) / kx;
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
	a[0] = (x0 - x1*k + x2*k*k)/d;
	a[1] = (-2.0*x0*k + x1 + x1*k*k - 2.0*x2*k)/d;
	a[2] = (x0*k*k - x1*k + x2)/d;
	b[1] = (2.0*k + y1 + y1*k*k - 2.0*y2*k)/d;
	b[2] = (-k*k - y1*k + y2)/d;
	if (highpass) {
		a[1] *= -1.0;
		b[1] *= -1.0;
	}
}

static int compute_cheby_iir(double *num, double *den, unsigned int num_pole,
		      int highpass, double ripple, double cutoff_freq)
{
	double *a, *b, *ta, *tb;
	double ap[3], bp[3];
	double sa, sb, gain;
	unsigned int i, p;
	int retval = 1;

	// Allocate temporary arrays
	a = malloc((num_pole + 3) * sizeof(*a));
	b = malloc((num_pole + 3) * sizeof(*b));
	ta = malloc((num_pole + 3) * sizeof(*ta));
	tb = malloc((num_pole + 3) * sizeof(*tb));
	if (!a || !b || !ta || !tb) {
		retval = 0;
		goto exit;
	}
	memset(a, 0, (num_pole + 3) * sizeof(*a));
	memset(b, 0, (num_pole + 3) * sizeof(*b));

	a[2] = 1.0;
	b[2] = 1.0;

	for (p = 1; p <= num_pole / 2; p++) {
		// Compute the coefficients for this pole
		get_pole_coefs(p, num_pole, cutoff_freq, ripple, highpass, ap, bp);

		// Add coefficients to the cascade
		memcpy(ta, a, (num_pole + 3) * sizeof(*a));
		memcpy(tb, b, (num_pole + 3) * sizeof(*b));
		for (i = 2; i <= num_pole + 2; i++) {
			a[i] = ap[0]*ta[i] + ap[1]*ta[i-1] + ap[2]*ta[i-2];
			b[i] = tb[i] - bp[1]*tb[i-1] - bp[2]*tb[i-2];
		}
	}

	// Finish combining coefficients
	b[2] = 0.0;
	for (i = 0; i <= num_pole; i++) {
		a[i] = a[i + 2];
		b[i] = -b[i + 2];
	}

	// Normalize the gain
	sa = sb = 0.0;
	for (i = 0; i <= num_pole; i++) {
		sa += a[i] * ((highpass && i % 2) ? -1.0 : 1.0);
		sb += b[i] * ((highpass && i % 2) ? -1.0 : 1.0);
	}
	gain = sa / (1.0 - sb);
	for (i = 0; i <= num_pole; i++)
		a[i] /= gain;



	// Copy the results to the num and den
	for (i = 0; i <= num_pole; i++) {
		num[i] = a[i];
		den[i] = -b[i];
	}
	// den[0] must be 1.0
	den[0] = 1.0;

      exit:
	free(a);
	free(b);
	free(ta);
	free(tb);
	return retval;
}




////////////////////////////////////////////////////////////////////////////
//
//                      Create particular filters
//
////////////////////////////////////////////////////////////////////////////
/**
 * \param fir_length	number of sample used to compute the mean
 * \param nchann	number of channels the filter will process
 * \param type		type of data the filter will process (\c DATATYPE_FLOAT or \c DATATYPE_DOUBLE)
 * \return	the handle of the newly created filter in case of success, \c NULL otherwise. 
 */
hfilter create_fir_filter_mean(unsigned int fir_length,
				unsigned int nchann, unsigned int type)
{
	unsigned int i;
	double* fir= NULL;
	hfilter filt;

	// Alloc temporary fir
	fir = malloc(fir_length*sizeof(*fir));
	if (!fir)
		return NULL;

	// prepare the finite impulse response
	for (i = 0; i < fir_length; i++)
		fir[i] = 1.0f / (double) fir_length;
	
	filt = create_filter(nchann, type, fir_length, fir, 0, NULL, DATATYPE_DOUBLE);

	free(fir);
	return filt;
}

/**
 * \param fc		Normalized cutoff frequency (the normal frequency divided by the sampling frequency)
 * \param half_length	the half size of the impulse response (in number of samples)
 * \param nchann	number of channels the filter will process
 * \param window	The type of the kernel wondow to use for designing the filter
 * \param type		type of data the filter will process (\c DATATYPE_FLOAT or \c DATATYPE_DOUBLE)
 * \return	the handle of the newly created filter in case of success, \c NULL otherwise. 
 */
hfilter create_fir_filter_lowpass(double fc, unsigned int half_length,
				   unsigned int nchann,
				   KernelWindow window, unsigned int type)
{
	double *fir = NULL;
	hfilter filt;
	unsigned int fir_length = 2 * half_length + 1;

	// Alloc temporary fir
	fir = malloc(fir_length*sizeof(*fir));
	if (!fir)
		return NULL;

	// prepare the finite impulse response
	compute_fir_lowpass(fir, fir_length, fc);
	apply_window(fir, fir_length, window);
	normalize_fir(fir, fir_length);

	filt = create_filter(nchann, type, fir_length, fir, 0, NULL, DATATYPE_DOUBLE);

	free(fir);
	return filt;
}


/**
 * \param fc		Normalized cutoff frequency (the normal frequency divided by the sampling frequency)
 * \param half_length	the half size of the impulse response (in number of samples)
 * \param nchann	number of channels the filter will process
 * \param window	The type of the kernel wondow to use for designing the filter
 * \param type		type of data the filter will process (\c DATATYPE_FLOAT or \c DATATYPE_DOUBLE)
 * \return	the handle of the newly created filter in case of success, \c NULL otherwise. 
 */
hfilter create_fir_filter_highpass(double fc, unsigned int half_length,
				    unsigned int nchann,
				    KernelWindow window, unsigned int type)
{
	double *fir = NULL;
	hfilter filt;
	unsigned int fir_length = 2 * half_length + 1;
	
	// Alloc temporary fir
	fir = malloc(fir_length*sizeof(*fir));
	if (!fir)
		return NULL;

	// prepare the finite impulse response
	compute_fir_lowpass(fir, fir_length, fc);
	apply_window(fir, fir_length, window);
	normalize_fir(fir, fir_length);
	reverse_fir(fir, fir_length);

	filt = create_filter(nchann, type, fir_length, fir, 0, NULL, DATATYPE_DOUBLE);
	if (!filt)
		return NULL;

	free(fir);
	return filt;
}


/**
 * \param fc_low	normalized cutoff frequency of the lowpass part (the normal frequency divided by the sampling frequency)
 * \param fc_high	normalized cutoff frequency of the highpass part (the normal frequency divided by the sampling frequency)
 * \param half_length	the half size of the impulse response (in number of samples)
 * \param nchann	number of channels the filter will process
 * \param window	the type of the kernel wondow to use for designing the filter
 * \param type		type of data the filter will process (\c datatype_float or \c datatype_double)
 * \return	the handle of the newly created filter in case of success, \c null otherwise. 
 */
hfilter create_fir_filter_bandpass(double fc_low, double fc_high,
				    unsigned int half_length,
				    unsigned int nchann,
				    KernelWindow window, unsigned int type)
{
	unsigned int len = 2 * (half_length / 2) + 1;
	double fir_low[len], fir_high[len];
	double *fir = NULL;
	hfilter filt;
	unsigned int fir_length = 2 * half_length + 1;


	// Alloc temporary fir
	fir = malloc(fir_length*sizeof(*fir));
	if (!fir)
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


	filt = create_filter(nchann, type, fir_length, fir, 0, NULL, DATATYPE_DOUBLE);

	free(fir);
	return filt;
}


/**
 * \param fc		normalized cutoff frequency (the normal frequency divided by the sampling frequency)
 * \param num_pole	The number of pole the z-transform of the filter should possess
 * \param nchann	number of channels the filter will process
 * \param highpass	flag to specify the type of filter (0 for a lowpass, 1 for a highpass)
 * \param ripple	ripple
 * \param type		type of data the filter will process (\c datatype_float or \c datatype_double)
 * \return	the handle of the newly created filter in case of success, \c null otherwise. 
 */
hfilter create_chebychev_filter(double fc, unsigned int num_pole,
				 unsigned int nchann, int highpass,
				 double ripple, unsigned int type)
{
	double *a = NULL, *b = NULL;
	hfilter filt;

	if (num_pole % 2 != 0)
		return NULL;

	a = malloc( (num_pole+1)*sizeof(*a));
	b = malloc( (num_pole+1)*sizeof(*b));
	if (!a || !b) {
		free(a);
		free(b);
		return NULL;
	}

	// prepare the filter
	if (!compute_cheby_iir(a, b, num_pole, highpass, ripple, fc)) {
		free(a);
		free(b);
		return NULL;
	}


	filt = create_filter(nchann, type, num_pole+1, a, num_pole+1, b, DATATYPE_DOUBLE);
	if (!filt)
		return NULL;

	free(a);
	free(b);
	return filt;
}

/**
 * \param fc		normalized cutoff frequency (the normal frequency divided by the sampling frequency)
 * \param num_pole	The number of pole the z-transform of the filter should possess
 * \param nchann	number of channels the filter will process
 * \param highpass	flag to specify the type of filter (0 for a lowpass, 1 for a highpass)
 * \param type		type of data the filter will process (\c datatype_float or \c datatype_double)
 * \return	the handle of the newly created filter in case of success, \c null otherwise. 
 */
hfilter create_butterworth_filter(double fc, unsigned int num_pole,
				   unsigned int num_chann, int highpass,
				   unsigned int type)
{
	return create_chebychev_filter(fc, num_pole, num_chann, highpass,
				       0.0, type);
}

/**
 * \param nchann	number of channels the filter will process
 * \param fs		sampling frequency in Hz
 * \param type		type of data the filter will process (\c datatype_float or \c datatype_double)
 * \return	the handle of the newly created filter in case of success, \c null otherwise. 
 */
hfilter create_integral_filter(unsigned int nchann, double fs, unsigned int type)
{
	hfilter filt;
	double a = 1.0/fs, b[2] = {1.0, -1.0};

	filt = create_filter(nchann, type, 1, &a, 2, &b, DATATYPE_DOUBLE);
	if (!filt)
		return NULL;


	return filt;
}

