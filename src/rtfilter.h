/*
 *  Copyright (C) 2008-2011 Nicolas Bourdaud <nicolas.bourdaud@mindmaze.com>
 *
 *  This file is part of the rtfilter library
 *
 *  The rtfilter library is free software: you can redistribute it and/or
 *  modify it under the terms of the version 3 of the GNU Lesser General
 *  Public License as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef RTFILTER_H
#define RTFILTER_H

#include <stddef.h>

#ifdef __cplusplus
#include <complex>
typedef std::complex < double > rtf_cdouble;
#else /* __cplusplus */
#ifdef _MSC_VER
typedef struct {
	double real;
	double img;
} rtf_cdouble;
#else
#include <complex.h>
typedef complex double rtf_cdouble;
#endif /* _MSC_VER */
#endif /* __cplusplus */


#ifdef __cplusplus
extern "C" {
#endif

/* Data type specifications */
#define RTF_FLOAT 0
#define RTF_DOUBLE 1
#define RTF_CFLOAT 2
#define RTF_CDOUBLE 3
#define RTF_PRECISION_MASK 1
#define RTF_COMPLEX_MASK 2

//! Handle to a filter. Used by all the functions to manipulate a filter.
typedef struct rtf_filter* hfilter;

//! create a digital filter
hfilter rtf_create_filter(int nchann, int proctype,
                          int num_len, const void *num,
                          int denum_len, const void *denum,
                          int type);

//! set filter lazy-init
void rtf_filter_set_lazy_init(hfilter filt, int do_lazy_init);

//! filter chunk of data
int rtf_filter(hfilter filt, const void* x, void* y, int ns);

//! initialize a filter with particular data
void rtf_init_filter(hfilter filt, const void* data);
//! destroy a filter
void rtf_destroy_filter(hfilter filt);
//! Return the type of the input or output of a filter
int rtf_get_type(hfilter filt, int in);

// Creates a filter that downsample signals
hfilter rtf_create_downsampler(int nch, int type, int r);

//! Return the version of the library in a string
size_t rtf_get_version(char* string, size_t len, int line);

struct real_coeffs {
	int num_len;
	int denum_len;
	double * num;
	double * denum;
};
struct complex_coeffs {
	int num_len;
	int denum_len;
	rtf_cdouble * num;
	rtf_cdouble * denum;
};

struct rtf_coeffs {
	int is_complex;
	union {
		struct real_coeffs real;
		struct complex_coeffs cplx;
	} coeffs;
};


void rtf_coeffs_destroy(struct rtf_coeffs * coeffs);
struct rtf_coeffs* rtf_get_coeffs(const struct rtf_filter * filt);


hfilter rtf_create_filter_coeffs(int nchann, int data_type,
                                 struct rtf_coeffs * coeffs);

/* helpers to create common filters */

/* Enumeration specifying some kernel windows */
typedef enum {
	BLACKMAN_WINDOW,
	HAMMING_WINDOW,
	RECT_WINDOW
} KernelWindow;

//! Create a local mean filter based on a sliding window
hfilter rtf_create_fir_mean(int nchann, int proctype,
                            int nsamples);

//! Lowpass windowed sinc filter
hfilter rtf_create_fir_lowpass(int nchann, int proctype,
                               double fc, int half_length,
                               KernelWindow window);
//! Highpass windowed sinc filter
hfilter rtf_create_fir_highpass(int nchann, int proctype,
                                double fc, int half_length,
                                KernelWindow window);

//! Bandpass windowed sinc filter
hfilter rtf_create_fir_bandpass(int nchann, int proctype,
                                double fc_low, double fc_high,
                                int half_length,
                                KernelWindow window);

//! Butterworth filter (IIR filter)
hfilter rtf_create_butterworth(int nchann, int proctype,
                               double fc, int num_pole,
                               int highpass);

//! Chebychev filter (IIR filter)
hfilter rtf_create_chebychev(int nchann, int proctype,
                             double fc, int num_pole,
                             int highpass, double ripple);

//! Simple first order integral filter (IIR filter)
hfilter rtf_create_integral(int nchann, int proctype, double fs);


hfilter rtf_create_bandpass_analytic(int nchann,
                                     int proctype,
                                     double fl, double fh,
                                     int num_pole);


//! Combine filters
struct rtf_filter* rtf_filter_combine(int nfilters,
                                      struct rtf_filter ** filters);

#ifdef __cplusplus
}
#endif

#endif //RTFILTER_H
