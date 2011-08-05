/*
    Copyright (C) 2010-2011 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    This file is part of the rtfilter library

    The rtfilter library is free software: you can redistribute it and/or
    modify it under the terms of the version 3 of the GNU Lesser General
    Public License as published by the Free Software Foundation.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <complex.h>
#include "rtfilter.h"
#include "filter-internal.h"


/**************************************************************************
 *                                                                        *
 *                     Complex double precision version                   *
 *                            ( complex double )                          *
 *                                                                        *
 **************************************************************************/
#ifdef __SSE3__
# define USE_SIMD
# include "complex-simd.h"
#endif

typedef complex double cdouble;
#define add_vec(v1,v2)			_mm_add_pd(v1,v2)
#define mul_vec(v1,v2)			complex_mul_pd(v1,v2)
#define mul_in_vec(v1,v2,part)		complex_mul_pd(v1,v2)
#define zero_vec()			_mm_setzero_pd()
#define set1_vec(data)			complex_set1_pd(data)
#define TYPEIN				cdouble
#define TYPEOUT				cdouble
#define FILTER_UNALIGNED_FUNC		filter_cdu
#define TYPEIN_V			__m128d
#define TYPEOUT_V			__m128d
#define FILTER_ALIGNED_FUNC		filter_cda
#define FILTER_FUNC			filter_cd
#define DOUTTYPE			RTF_CDOUBLE
#define DINTYPE				RTF_CDOUBLE

#include "templates.c"

