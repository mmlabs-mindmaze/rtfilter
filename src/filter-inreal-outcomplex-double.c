/*
      Copyright (C) 2010 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

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
 *          complex output real input double precision version            *
 *                    ( complex double out double in)                     *
 *                                                                        *
 **************************************************************************/
typedef complex double cdouble;

#define TYPEIN				double
#define TYPEOUT				cdouble
#define FILTER_UNALIGNED_FUNC		filter_dcdu
#define FILTER_FUNC			filter_dcd
#define DINTYPE				RTF_DOUBLE
#define DOUTTYPE			RTF_CDOUBLE



#include "templates.c"

