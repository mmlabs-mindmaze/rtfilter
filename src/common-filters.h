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
/** \file common-filters.h
 * \brief Definitions for designing filters
 * \author Nicolas Bourdaud
 * 
 * This header provides the definitions for function to design some of the usual filters. Those comprises some windowed sinc filters and some filters with an infinite impulse response (IIR). 
 */ 
#ifndef COMMON_FILTERS_H
#define COMMON_FILTERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <filter.h>

/**
 * Enumeration specifying some kernel windows
 */
typedef enum {
	BLACKMAN_WINDOW,
	HAMMING_WINDOW,
	RECT_WINDOW
} KernelWindow;

//! Create a local mean filter based on a sliding window 
hfilter create_fir_filter_mean(unsigned int nsamples, unsigned int nchann,
                               unsigned int type);

//! Lowpass windowed sinc filter
hfilter create_fir_filter_lowpass(double fc, unsigned int half_length,
                                  unsigned int nchann, KernelWindow window,
				  unsigned int type);
//! Highpass windowed sinc filter
hfilter create_fir_filter_highpass(double fc, unsigned int half_length,
                                   unsigned int nchann, KernelWindow window,
				   unsigned int type);

//! Highpass windowed sinc filter
hfilter create_fir_filter_bandpass(double fc_low, double fc_high,
                                   unsigned int half_length,
				   unsigned int nchann, KernelWindow window,
				   unsigned int type);

//! Butterworth filter (IIR filter)
hfilter create_butterworth_filter(double fc, unsigned int num_pole,
                                  unsigned int num_chann, int highpass,
				  unsigned int type);

//! Chebychev filter (IIR filter)
hfilter create_chebychev_filter(double fc, unsigned int num_pole,
                                unsigned int nchann, int highpass,
				double ripple, unsigned int type);

//! Simple first order integral filter (IIR filter) 
hfilter create_integral_filter(unsigned int nchann, double fs, unsigned int type);

#ifdef __cplusplus
}
#endif

#endif /*COMMON_FILTERS_H*/
