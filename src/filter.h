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
/**
 * \file filter.h
 * \brief Definitions of foundamental primitives of rtfilter library
 * \author Nicolas Bourdaud
 *
 * These are the definitions of the minimal primitives used to interact with digital filters in the rtfilter library. The primitives described here for using a filter, not designing it.
 *
 * \sa common-filter.h
 */
#ifndef FILTER_H
#define FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#define DATATYPE_FLOAT	0
#define DATATYPE_DOUBLE	1

typedef const struct _dfilter* hfilter;

//! create a digital filter 
hfilter create_filter(unsigned int nchann, unsigned int proctype,
                      unsigned int num_len, const void *num,
		      unsigned int denum_len, const void *denum,
		      unsigned int type);
//! filter chunk of \c float data
void filter_f(hfilter filt, const float* x, float* y, unsigned int ns);
//! filter chunk of \c double data
void filter_d(hfilter filt, const double* x, double* y, unsigned int ns);
//! initialize a filter with particular data
void init_filter(hfilter filt, const void* data);
//! destroy a filter
void destroy_filter(hfilter filt);



#ifdef __cplusplus
}
#endif

#endif //FILTER_H
