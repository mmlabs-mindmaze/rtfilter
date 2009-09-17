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
/** \internal
 * \file templates.c
 * \brief Templates for implemention of data specific primitives
 * \author Nicolas Bourdaud
 *
 * This file includes the templates of the functions that are specific to certain data type
 */

#include <assert.h>

#define NELEM_VEC	(sizeof(TYPEREAL_V)/sizeof(TYPEREAL))


void COPY_NUMDENUM(hfilter filt, unsigned int num_len, const TYPEREAL *num, unsigned int denum_len, const TYPEREAL *denum)
{
	unsigned int i;
	TYPEREAL normfactor = (num == NULL) ? 1.0 : denum[0];

	if (filt->type == DATATYPE_FLOAT) {
		float *af = (float*)(filt->a);
		float *bf = (float*)(filt->b);
		for (i=0; i<num_len; i++)
			af[i] = num[i] / normfactor;
		for (i=0; i<denum_len-1; i++)
			bf[i] = -denum[i+1] / normfactor;
	} else {
		double *ad = (double*)(filt->a);
		double *bd = (double*)(filt->b);
		for (i=0; i<num_len; i++)
			ad[i] = num[i] / normfactor;
		for (i=0; i<denum_len-1; i++)
			bd[i] = -denum[i+1] / normfactor;
	}
}


/*************************************************
 * Instanciate filter function on unaligned data *
 *************************************************/
#define add_dat(d1,d2)		((d1)+(d2))
#define mul_dat(d1,d2)		((d1)*(d2))
#define zero_dat()		(0)
#define set1_dat(data)		(data)
#define TYPEREAL_LOCAL		TYPEREAL
#define FILTER_DATADEP_FUNC	FILTER_UNALIGNED_FUNC
#include "filter-func-template.c"


#ifdef USE_SIMD

#undef add_dat
#undef mul_dat
#undef zero_dat
#undef set1_dat
#undef TYPEREAL_LOCAL
#undef FILTER_DATADEP_FUNC

/****************************************************
 * Instanciate filter function on vectorizable data *
 ****************************************************/
#define add_dat(d1,d2)		add_vec(d1,d2)
#define mul_dat(d1,d2)		mul_vec(d1,d2)
#define zero_dat()		zero_vec()
#define set1_dat(data)		set1_vec(data)
#define TYPEREAL_LOCAL		TYPEREAL_V
#define FILTER_DATADEP_FUNC	FILTER_ALIGNED_FUNC
#include "filter-func-template.c"

#endif /* USE_SIMD */


void FILTER_FUNC(hfilter filt, const TYPEREAL* in, TYPEREAL* out, unsigned int nsamples)
{
	assert(filt->type == DTYPE);
#ifdef USE_SIMD
	// Check that data is aligned on 16 byte boundaries
	if ( !((filt->num_chann%NELEM_VEC) 
		|| ((uintptr_t)in % sizeof(TYPEREAL_V)) 
		|| ((uintptr_t)out % sizeof(TYPEREAL_V))) )
		FILTER_ALIGNED_FUNC(filt, (const TYPEREAL_V*)in, (TYPEREAL_V*)out, nsamples);
	else
#endif //USE_SIMD
		FILTER_UNALIGNED_FUNC(filt, in, out, nsamples);
}

