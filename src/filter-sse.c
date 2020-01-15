/*
    Copyright (C) 2009-2011 Nicolas Bourdaud <nicolas.bourdaud@gmail.com>

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

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>

#include "filter-funcs.h"
#include "filter-internal.h"
#include "probesimd.h"
#include "rtfilter.h"


/**************************************************************************
 *                   Real single precision version : SSE                  *
 *                               ( float )                                *
 **************************************************************************/

#define TYPEIN				float
#define TYPEOUT				float
#define add_dat(d1,d2)			_mm_add_ps(d1,d2)
#define mul_in_dat(d1,d2,part)		_mm_mul_ps(d1,d2)
#define mul_dat(d1,d2)			_mm_mul_ps(d1,d2)
#define zero_dat()			_mm_setzero_ps()
#define set1_dat(data)			_mm_set1_ps(data)
#define TYPEIN_LOCAL			__m128
#define TYPEOUT_LOCAL			__m128
#define FILTER_DATADEP_FUNC		filter_f_sse
#include "filter-func-template.c"


