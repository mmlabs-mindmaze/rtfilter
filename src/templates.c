/*
    Copyright (C) 2008-2011 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

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

#define NELEM_VEC	(sizeof(TYPEOUT_V)/sizeof(TYPEOUT))
#define NELEM_VECIN	(sizeof(TYPEIN_V)/sizeof(TYPEIN))


/*************************************************
 * Instanciate filter function on unaligned data *
 *************************************************/
#define add_dat(d1,d2)		((d1)+(d2))
#define mul_in_dat(d1,d2,part)	((d1)*(d2))
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
#define mul_in_dat(d1,d2,part)	mul_in_vec(d1,d2,part)
#define mul_dat(d1,d2)		mul_vec(d1,d2)
#define zero_dat()		zero_vec()
#define set1_dat(data)		set1_vec(data)
#define TYPEIN_LOCAL		TYPEIN_V
#define TYPEOUT_LOCAL		TYPEOUT_V
#define FILTER_DATADEP_FUNC	FILTER_ALIGNED_FUNC
#include "filter-func-template.c"


static HOTSPOT
unsigned int filtsimd(hfilter filt, const void* x, void* y, unsigned int ns)
{
	if ( !((uintptr_t)x % sizeof(TYPEIN_V))
	  && !((uintptr_t)y % sizeof(TYPEOUT_V)))
		FILTER_ALIGNED_FUNC(filt, x, y, ns);
	else 
		FILTER_UNALIGNED_FUNC(filt, x, y, ns);
	return ns;
}

#endif /* USE_SIMD */


static HOTSPOT
unsigned int filtnoop(hfilter filt, const void* x, void* y, unsigned int ns)
{
	FILTER_UNALIGNED_FUNC(filt, x, y, ns);
	return ns;
}


LOCAL_FN
void SET_FILTER_FUNC(struct rtf_filter* filt)
{
	filt->filter_fn = filtnoop;

#ifdef USE_SIMD
	// Check that sample can be aligned on 16 byte boundaries
	if (!(filt->num_chann%NELEM_VECIN))
		filt->filter_fn = filtsimd;
#endif //USE_SIMD
}


