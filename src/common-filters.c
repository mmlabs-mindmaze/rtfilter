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
/** \internal
 * \file common-filters.c
 * \brief Implemention of designing filters
 * \author Nicolas Bourdaud
 *
 * This is the implementation of the functions that design filters.
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include "rtfilter.h"
#include "rtf_common.h"

#define PId	3.1415926535897932384626433832795L
#define PIf	3.1415926535897932384626433832795f

/***************************************************************
 *                                                             *
 *                     Helper functions                        *
 *                                                             *
 ***************************************************************/
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

static void FFT(complex double *X, double *t, unsigned int length){

	unsigned int i,j;

	for(i=0; i<length; i++){
		X[i]=0;
		for(j=0; j<length; j++){
			X[i]=X[i] + t[j]*cexp((-2.0*I*M_PI)*(j)*(i)/length);				
		}
	}	

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

/*	Algorithm taken form:
	
	"Introduction to Digital Filters with Audio Applications",
	by  Julius O. Smith III, (September 2007 Edition). 
	https://ccrma.stanford.edu/~jos/filters/Numerical_Computation_Group_Delay.html 
*/
static double compute_IIR_filter_delay(double *num, double *den,
					unsigned int length)
{
	unsigned int i,length_c;
	double *a,*b,*c,*cr; 
	complex double *X,*Y;
	double Delay = 0.0, d = 0.0;	

	length_c=2*length-1;

	a = malloc( (length)*sizeof(*a));
	b = malloc( (length)*sizeof(*b));
	c = malloc( (length_c)*sizeof(*c));
	cr = malloc( (length_c)*sizeof(*cr));
	X = malloc( (length_c)*sizeof(*X));
	Y = malloc( (length_c)*sizeof(*Y));
	if (!a || !b || !c || !cr|| !X || !Y){
		Delay=0.0;
		goto exit;
	}		

	for(i=0;i<length;i++){
		b[i]=den[length-i-1];
		a[i]=num[i];	
	}
	compute_convolution(c,b,length,a,length);

	for (i=0;i<length_c;i++)
		cr[i] = c[i]*i;
		
	FFT(Y,c,length_c);
	FFT(X,cr,length_c);

	for (i=0;i<length_c;i++) {
		d = creal(X[i]/Y[i]);
		
		if (d > Delay)
		     Delay = d;
	}
	
	exit:
	free(a);
	free(b);
	free(c);
	free(cr);
	free(X);
	free(Y);
	return Delay;
}


/******************************
 * inspired by DSP guide ch33 *
 ******************************/
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

/******************************
 * inspired by DSP guide ch33 *
 ******************************/
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

/**	compute_bandpass_complex_filter:
 * \param fl		normalized lowest cutoff freq of the bandpass filter. (the normal frequency divided by the sampling freq)
 * \param fh		normalized highest cutoff freq of the bandpass filter. (the normal frequency divided by the sampling freq)
 * \param num_pole	The number of pole the z-transform of the filter should possess

	This function creates a complex bandpass filter from a Chebyshev low pass filter.
*/
static int compute_bandpass_complex_filter(complex double *num,
					   complex double *den,
					   unsigned int num_pole,
					   double fl, double fh)
{
	double *a=NULL, *b=NULL;	
	complex double *ac,*bc;
	double ripple,fc,alpha,Delay;
	unsigned int i,retval=1;

	// Allocate temporary arrays
	a = malloc( (num_pole+1)*sizeof(*a));
	b = malloc( (num_pole+1)*sizeof(*b));
	ac = malloc((num_pole+1)*sizeof(*ac));
	bc = malloc( (num_pole+1)*sizeof(*bc));
	if (!a || !b || !ac || !bc){
		retval=0;		
		goto exit;
	}

	alpha = M_PI*(fl+fh);   // Rotation angle in radians to produces
				// the desired analitic filter
	fc = (fh-fl)/2.0;       // Normalized cutoff frequency
				// of the low pass filter
	ripple = 0.01;

	// prepare the z-transform of low pass filter 
	if (!compute_cheby_iir(b, a, num_pole, 0, ripple, fc)){
		retval=0;		
		goto exit;
	}
	
	// Compute the low pass filter delay; the complex filter 
	Delay=compute_IIR_filter_delay(b, a,num_pole+1);  

	/* Note: The complex filter introduces a delay equal to
	e^(j*alpha*D) (D: Delay low pass filter).To get rid of the
	undesired frequency independent phase factor, the filter with
	rotated poles and zeros should be multiplied by
	the constant e^(-j*alpha*D).*/


	// compute complex coefficients (rotating poles and zeros). 
	for(i=0;i<num_pole + 1; i++) {
		// complex numerator
		ac[i]= 2.0*cexp(-1.0*I*alpha*Delay)
		     *b[i]*cexp(1.0*I*alpha*(i+1));

		// complex denominator
		bc[i]= a[i]*cexp(1.0*I*alpha*(i+1));
	}

	for(i=0;i<num_pole + 1; i++){
		num[i]=ac[i];
		den[i]=bc[i];
	}

exit:
	free(a);
	free(b);
	free(ac);
	free(bc);
	return retval;
}


/**************************************************************************
 *                                                                        *
 *                      Create particular filters                         *
 *                                                                        *
 **************************************************************************/
/**
 * \param nchann	number of channels the filter will process
 * \param type		type of data the filter will process (\c RTF_FLOAT or \c RTF_DOUBLE)
 * \param fir_length	number of sample used to compute the mean
 * \return	the handle of the newly created filter in case of success, \c NULL otherwise. 
 */
API_EXPORTED
hfilter rtf_create_fir_mean(unsigned int nchann, int proctype,
                               unsigned int fir_length)
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
	
	filt = rtf_create_filter(nchann, proctype,
	                     fir_length, fir, 0, NULL,
			     RTF_DOUBLE);

	free(fir);
	return filt;
}

/**
 * \param nchann	number of channels the filter will process
 * \param proctype	type of data the filter will process (\c RTF_FLOAT or \c RTF_DOUBLE)
 * \param fc		Normalized cutoff frequency (the normal frequency divided by the sampling frequency)
 * \param half_length	the half size of the impulse response (in number of samples)
 * \param window	The type of the kernel wondow to use for designing the filter
 * \return	the handle of the newly created filter in case of success, \c NULL otherwise. 
 */
API_EXPORTED
hfilter rtf_create_fir_lowpass(unsigned int nchann, int proctype,
                                  double fc, unsigned int half_length,
                                  KernelWindow window)
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

	filt = rtf_create_filter(nchann, proctype,
	                     fir_length, fir, 0, NULL,
	                     RTF_DOUBLE);

	free(fir);
	return filt;
}


/**
 * \param nchann	number of channels the filter will process
 * \param proctype	type of data the filter will process (\c RTF_FLOAT or \c RTF_DOUBLE)
 * \param fc		Normalized cutoff frequency (the normal frequency divided by the sampling frequency)
 * \param half_length	the half size of the impulse response (in number of samples)
 * \param window	The type of the kernel wondow to use for designing the filter
 * \return	the handle of the newly created filter in case of success, \c NULL otherwise. 
 */
API_EXPORTED
hfilter rtf_create_fir_highpass(unsigned int nchann, int proctype,
                                   double fc, unsigned int half_length,
                                   KernelWindow window)
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

	filt = rtf_create_filter(nchann, proctype,
	                     fir_length, fir, 0, NULL,
	                     RTF_DOUBLE);

	free(fir);
	return filt;
}


/**
 * \param nchann	number of channels the filter will process
 * \param proctype	type of data the filter will process (\c datatype_float or \c datatype_double)
 * \param fc_low	normalized cutoff frequency of the lowpass part (the normal frequency divided by the sampling frequency)
 * \param fc_high	normalized cutoff frequency of the highpass part (the normal frequency divided by the sampling frequency)
 * \param half_length	the half size of the impulse response (in number of samples)
 * \param window	the type of the kernel wondow to use for designing the filter
 * \return	the handle of the newly created filter in case of success, \c null otherwise. 
 */
API_EXPORTED
hfilter rtf_create_fir_bandpass(unsigned int nchann, int proctype,
                                   double fc_low, double fc_high,
				   unsigned int half_length,
				   KernelWindow window)
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

	filt = rtf_create_filter(nchann, proctype,
	                     fir_length, fir, 0, NULL,
	                     RTF_DOUBLE);

	free(fir);
	return filt;
}


/**
 * \param nchann	number of channels the filter will process
 * \param proctype	type of data the filter will process (\c datatype_float or \c datatype_double)
 * \param fc		normalized cutoff frequency (the normal frequency divided by the sampling frequency)
 * \param num_pole	The number of pole the z-transform of the filter should possess
 * \param highpass	flag to specify the type of filter (0 for a lowpass, 1 for a highpass)
 * \param ripple	ripple
 * \return	the handle of the newly created filter in case of success, \c null otherwise. 
 */
API_EXPORTED
hfilter rtf_create_chebychev(unsigned int nchann, int proctype,
                                double fc, unsigned int num_pole,
				int highpass, double ripple)
{
	double *a = NULL, *b = NULL;
	hfilter filt = NULL;

	if (num_pole % 2 != 0)
		return NULL;

	a = malloc( (num_pole+1)*sizeof(*a));
	b = malloc( (num_pole+1)*sizeof(*b));
	if (!a || !b)
		goto out;

	// prepare the z-transform of the filter
	if (!compute_cheby_iir(a, b, num_pole, highpass, ripple, fc))
		goto out;

	filt = rtf_create_filter(nchann, proctype,
	                     num_pole+1, a, num_pole+1, b,
	                     RTF_DOUBLE);

out:
	free(a);
	free(b);
	return filt;
}

/**
 * \param nchann	number of channels the filter will process
 * \param proctype	type of data the filter will process (\c datatype_float or \c datatype_double)
 * \param fc		normalized cutoff frequency (the normal frequency divided by the sampling frequency)
 * \param num_pole	The number of pole the z-transform of the filter should possess
 * \param highpass	flag to specify the type of filter (0 for a lowpass, 1 for a highpass)
 * \return	the handle of the newly created filter in case of success, \c null otherwise. 
 */
API_EXPORTED
hfilter rtf_create_butterworth(unsigned int nchann, int proctype,
                                  double fc, unsigned int num_pole,
				  int highpass)
{
	return rtf_create_chebychev(nchann, proctype,
	                               fc, num_pole, highpass, 0.0);
}

/**
 * \param nchann	number of channels the filter will process
 * \param type		type of data the filter will process (\c datatype_float or \c datatype_double)
 * \param fs		sampling frequency in Hz
 * \return	the handle of the newly created filter in case of success, \c null otherwise. 
 */
API_EXPORTED
hfilter rtf_create_integral(unsigned int nchann, int type, double fs)
{
	hfilter filt;
	double a = 1.0/fs, b[2] = {1.0, -1.0};

	filt = rtf_create_filter(nchann, type, 1, &a, 2, &b, RTF_DOUBLE);

	return filt;
}

/**
	rtf_create_bandpass_analytic:

	Analytic filter: Is a complex filter generating a signal whose spectrum equals the positive spectrum from a (real) 		input signal. The resulting signal is said to be “analytic".	
	The technique relies on the rotation of the pole-zero plot of a low pass filter.
	
 * \param nchann	number of channels the filter will process.
 * \param proctype	type of data the filter will process (\c datatype_float or \c datatype_double).
 * \param fl		normalized low bandpass cutoff frequency (the normal frequency divided by the sampling frequency).
 * \param fh		normalized hihg bandpass cutoff frequency (the normal frequency divided by the sampling frequency).
 * \param num_pole	The number of pole the z-transform of the filter should possess.
 * \return		The handle of the newly created filter in case of success, \c null otherwise. 
 */
API_EXPORTED
hfilter rtf_create_bandpass_analytic(unsigned int nchann,
					int proctype, 
					double fl, double fh, 
					unsigned int num_pole)
{
	complex double *a = NULL, *b = NULL;
	hfilter filt = NULL;

	if (num_pole % 2 != 0)
		return NULL;

	a = malloc((num_pole+1)*sizeof(*a));
	b = malloc((num_pole+1)*sizeof(*b));
	if (!a || !b)
		goto out;

	// prepare the z-transform of the complex bandpass filter
	if (!compute_bandpass_complex_filter(b,a,num_pole,fl,fh))
		goto out;

	filt = rtf_create_filter(nchann, proctype,
	                     num_pole+1, b, num_pole+1, a,
	                     RTF_CDOUBLE);
out:
	free(a);
	free(b);
	return filt;
}





