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
hfilter create_fir_filter_mean(unsigned int nchann, unsigned int type,
                               unsigned int nsamples);

//! Lowpass windowed sinc filter
hfilter create_fir_filter_lowpass(unsigned int nchann, unsigned int type, 
                                  double fc, unsigned int half_length,
                                  KernelWindow window);
//! Highpass windowed sinc filter
hfilter create_fir_filter_highpass(unsigned int nchann, unsigned int type,
                                   double fc, unsigned int half_length,
                                   KernelWindow window);

//! Bandpass windowed sinc filter
hfilter create_fir_filter_bandpass(unsigned int nchann, unsigned int type,
                                   double fc_low, double fc_high,
                                   unsigned int half_length,
				   KernelWindow window);

//! Butterworth filter (IIR filter)
hfilter create_butterworth_filter(unsigned int nchann, unsigned int type,
                                  double fc, unsigned int num_pole,
                                  int highpass);

//! Chebychev filter (IIR filter)
hfilter create_chebychev_filter(unsigned int nchann, unsigned int type,
                                double fc, unsigned int num_pole,
                                int highpass, double ripple);

//! Simple first order integral filter (IIR filter) 
hfilter create_integral_filter(unsigned int nchann, unsigned int type, double fs);

#ifdef __cplusplus
}
#endif

#endif /*COMMON_FILTERS_H*/
