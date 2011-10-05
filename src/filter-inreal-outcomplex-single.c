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
#include <complex.h>
#include <math.h>
#include <stdint.h>
#include "rtfilter.h"
#include "filter-internal.h"


/**************************************************************************
 *                                                                        *
 *          complex output real input single precision version            *
 *                      ( complex float out float in)                     *
 *                                                                        *
 **************************************************************************/
#ifdef __SSE3__
# define USE_SIMD
# include "complex-simd.h"
#endif

typedef complex float cfloat;
#define add_vec(v1,v2)			_mm_add_ps(v1,v2)
#define mul_vec(v1,v2)			complex_mul_ps(v1,v2)
#define mul_in_vec(v1,v2,part)		realcomp_mul_ps(v1,v2,part)
#define zero_vec()			_mm_setzero_ps()
#define set1_vec(data)			complex_set1_ps(data)
#define TYPEIN				float
#define TYPEOUT				cfloat
#define FILTER_UNALIGNED_FUNC		filter_fcfu
#define TYPEIN_V			__m128
#define TYPEOUT_V			__m128
#define FILTER_ALIGNED_FUNC		filter_fcfa
#define SET_FILTER_FUNC			set_filterfn_fcf
#define DINTYPE				RTF_FLOAT
#define DOUTTYPE			RTF_CFLOAT



#include "templates.c"

