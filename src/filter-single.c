/*
      Copyright (C) 2009 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

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
 * \file filter-single.c
 * \brief Implemention of single precision fundamental primitives
 * \author Nicolas Bourdaud
 *
 * This is the implementation of the functions part of the fundamental primitives that are specificatic to single precision data (\c float)
 */
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "filter.h"
#include "filter-internal.h"


/**************************************************************************
 *                                                                        *
 *                        Single precision version                        *
 *                               ( float )                                *
 *                                                                        *
 **************************************************************************/
#ifdef __SSE__
# include <xmmintrin.h>
# define USE_SIMD
#endif

#define add_vec(v1,v2)			_mm_add_ps(v1,v2)
#define mul_vec(v1,v2)			_mm_mul_ps(v1,v2)
#define zero_vec()			_mm_setzero_ps()
#define set1_vec(data)			_mm_set1_ps(data)
#define TYPEREAL			float
#define TYPEREAL_V			__m128
#define FILTER_ALIGNED_FUNC		filter_fa
#define FILTER_UNALIGNED_FUNC		filter_fu
#define FILTER_FUNC			filter_f
#define COPY_NUMDENUM			copy_numdenum_f
#define DTYPE				DATATYPE_FLOAT



#include "templates.c"

/** \fn void filter_f(hfilter filt, const float* x, float* y, unsigned int ns)
 * \param filt	handle to a digital filter resource
 * \param x	pointer to an array of input data
 * \param y	pointer to an array of output data
 * \param ns	number of time sample that should be processed
 *
 * This function apply the filter on the data specified by pointer \c x and write the filter data on the array pointed by \c y. You can call this funtion ONLY if the filter used has been created using the DATATYPE_FLOAT for the parameter \c proctype in the function create_filter()
 * 
 * The arrays pointed by \c x and \c y must be made of \c float values their number of elements have to be equal to \c ns multiplied by the number of channels processed. The arrays should be packed by channels with the following pattern: | S1C1 | S1C2 | ... | S1Ck | S2C1 | S2C2 | .... | S2Ck | ... | SnCk | where SiCj refers to the data in the i-th sample of the j-th channel.
 *
 * \sa create_filter(), filter_d()
 */
