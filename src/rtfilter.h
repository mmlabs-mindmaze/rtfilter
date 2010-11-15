/*
      Copyright (C) 2008-2010 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    This file is part of the rtfilter library

    The rtfilter library is free software: you can redistribute it and/or
    modify it under the terms of the version 3 of the GNU General Public
    License as published by the Free Software Foundation.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef RTFILTER_H
#define RTFILTER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Data type specifications */
#define RTF_FLOAT	0
#define RTF_DOUBLE	1
#define RTF_CFLOAT	2
#define RTF_CDOUBLE	3
#define RTF_PRECISION_MASK	1
#define RTF_COMPLEX_MASK	2

//! Handle to a filter. Used by all the functions to manipulate a filter.
typedef const struct rtf_filter* hfilter;

//! create a digital filter 
hfilter rtf_create_filter(unsigned int nchann, int proctype,
                      unsigned int num_len, const void *num,
                      unsigned int denum_len, const void *denum,
                      int type);
//! filter chunk of data
unsigned int rtf_filter(hfilter filt, const void* x, void* y,
                        unsigned int ns);
//! initialize a filter with particular data
void rtf_init_filter(hfilter filt, const void* data);
//! destroy a filter
void rtf_destroy_filter(hfilter filt);

// Creates a filter that downsample signals
hfilter rtf_create_downsampler(unsigned int nch, int type, unsigned int r);


#ifdef __cplusplus
}
#endif

#endif //RTFILTER_H
