/*
    Copyright (C) 2009-2011 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

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

#include <emmintrin.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "filter-funcs.h"
#include "filter-internal.h"
#include "probesimd.h"
#include "rtfilter.h"


/**************************************************************************
 *                   Real double precision version : SSE2                 *
 *                               ( double )                               *
 **************************************************************************/

#define TYPEIN				double
#define TYPEOUT				double
#define add_dat(d1,d2)			_mm_add_pd(d1,d2)
#define mul_in_dat(d1,d2,part)		_mm_mul_pd(d1,d2)
#define mul_dat(d1,d2)			_mm_mul_pd(d1,d2)
#define zero_dat()			_mm_setzero_pd()
#define set1_dat(data)			_mm_set1_pd(data)
#define TYPEIN_LOCAL			__m128d
#define TYPEOUT_LOCAL			__m128d
#define FILTER_DATADEP_FUNC		filter_d_sse2
#include "filter-func-template.c"

