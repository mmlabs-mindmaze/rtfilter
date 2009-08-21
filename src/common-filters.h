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
#ifndef COMMON_FILTERS_H
#define COMMON_FILTERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <filter.h>

typedef enum {
	BLACKMAN_WINDOW,
	HAMMING_WINDOW,
	RECT_WINDOW
} KernelWindow;

hfilter create_fir_filter_mean(unsigned int nsamples, unsigned int nchann,
                               unsigned int type);
hfilter create_fir_filter_lowpass(double fc, unsigned int half_length,
                                  unsigned int nchann, KernelWindow window,
				  unsigned int type);
hfilter create_fir_filter_highpass(double fc, unsigned int half_length,
                                   unsigned int nchann, KernelWindow window,
				   unsigned int type);
hfilter create_fir_filter_bandpass(double fc_low, double fc_high,
                                   unsigned int half_length,
				   unsigned int nchann, KernelWindow window,
				   unsigned int type);
hfilter create_butterworth_filter(double fc, unsigned int num_pole,
                                  unsigned int num_chann, int highpass,
				  unsigned int type);
hfilter create_chebychev_filter(double fc, unsigned int num_pole,
                                unsigned int nchann, int highpass,
				double ripple, unsigned int type);
hfilter create_integrate_filter(unsigned int nchann, unsigned int type);

#ifdef __cplusplus
}
#endif

#endif /*COMMON_FILTERS_H*/
