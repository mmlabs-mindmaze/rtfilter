/*
      Copyright (C) 2008-2010 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

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
 * \file templates.c
 * \brief Templates for implemention of data specific primitives
 * \author Nicolas Bourdaud
 *
 * This file includes the templates of the functions that are specific to certain data type
 */

#include <assert.h>

#define NELEM_VEC	(sizeof(TYPEOUT_V)/sizeof(TYPEOUT))


/*************************************************
 * Instanciate filter function on unaligned data *
 *************************************************/
#define add_dat(d1,d2)		((d1)+(d2))
#define mul_in_dat(d1,d2)	((d1)*(d2))
#define mul_dat(d1,d2)		((d1)*(d2))
#define zero_dat()		(0)
#define set1_dat(data)		(data)
#define TYPEIN_LOCAL		TYPEIN
#define TYPEOUT_LOCAL		TYPEOUT
#define FILTER_DATADEP_FUNC	FILTER_UNALIGNED_FUNC
#include "filter-func-template.c"


#ifdef USE_SIMD

#undef add_dat
#undef mul_dat
#undef mul_in_dat
#undef zero_dat
#undef set1_dat
#undef TYPEIN_LOCAL
#undef TYPEOUT_LOCAL
#undef FILTER_DATADEP_FUNC

/****************************************************
 * Instanciate filter function on vectorizable data *
 ****************************************************/
#define add_dat(d1,d2)		add_vec(d1,d2)
#define mul_in_dat(d1,d2)	mul_in_vec(d1,d2)
#define mul_dat(d1,d2)		mul_vec(d1,d2)
#define zero_dat()		zero_vec()
#define set1_dat(data)		set1_vec(data)
#define TYPEIN_LOCAL		TYPEIN_V
#define TYPEOUT_LOCAL		TYPEOUT_V
#define FILTER_DATADEP_FUNC	FILTER_ALIGNED_FUNC
#include "filter-func-template.c"

#endif /* USE_SIMD */

LOCAL_FN
void FILTER_FUNC(const struct rtf_filter* filt, const void* in, void* out, unsigned int nsamples)
{
	assert(filt->intype == DINTYPE);
	assert(filt->outtype == DOUTTYPE);
#ifdef USE_SIMD
	// Check that data is aligned on 16 byte boundaries
	if ( !((filt->num_chann%NELEM_VEC) 
		|| ((uintptr_t)in % sizeof(TYPEIN_V)) 
		|| ((uintptr_t)out % sizeof(TYPEOUT_V))) )
		FILTER_ALIGNED_FUNC(filt, (const TYPEIN_V*)in, (TYPEOUT_V*)out, nsamples);
	else
#endif //USE_SIMD
		FILTER_UNALIGNED_FUNC(filt, (const TYPEIN*)in, (TYPEOUT*)out, nsamples);
}

