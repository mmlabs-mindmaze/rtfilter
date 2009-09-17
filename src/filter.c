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
 * \file filter.c
 * \brief Implemention of common fundamental primitives
 * \author Nicolas Bourdaud
 *
 * This is the implementation of the functions part of the fundamental primitives that are shared (no specificity to data type)
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



static void* align_alloc(size_t alignment, size_t size)
{
	void* memptr = NULL;
	if (posix_memalign(&memptr, alignment, size))
		return NULL;
	return memptr;
}


static void  align_free(void* memptr)
{
	free(memptr);
}


/*!
 * \param filt \c handle of a filter 
 * 
 * Destroy a filter that you don't use anymore. It will free all its associated resources. After calling this function, you cannot use the handle anymore.
 */
void destroy_filter(hfilter filt)
{
	if (!filt)
		return;

	free((void *) (filt->a));
	free((void *) (filt->b));
	align_free(filt->xoff);
	align_free(filt->yoff);
	free((void*) filt);
}


static void reset_filter(hfilter filt)
{
	memset(filt->xoff, 0,
	       (filt->a_len -
		1) * filt->num_chann * sizeof_data(filt->type));
	memset(filt->yoff, 0,
	       (filt->b_len) * filt->num_chann * sizeof_data(filt->type));
}

/*!
 * \param filt \c handle of a filter 
 * \param data pointer to an array of values (float or double, depending on the filter). Can be \c NULL.
 * 
 * Initialize the internal states of the filter with provided data.
 *
 * If \c data is \c NULL, it will initialize the internal states with 0.0.
 *
 * If \c data is not \c NULL, it should point to an array of values whose the type and the number depends respectively on the type and the number of channels processed by the filter.\n
 * The type should be the same than the type used for the creation of the filter. The number of values in the array should be the same as the number of channels of the filter.\n
 * The internal states will then be initialized as if we had constantly fed the filter during its past with the values provided for each channel.
 */
void init_filter(hfilter filt, const void* data)
{
	int i;
	void* dest;
	int datlen = filt->num_chann*sizeof(filt->type);
	
	if (data == NULL) {
		reset_filter(filt);
		return;
	}

	dest = filt->xoff;
	for (i=0; i<(filt->a_len-1); i++) {
		memcpy(dest, data, datlen);
		dest = (char*)dest + datlen;
	}

	dest = filt->yoff;
	for (i=0; i<filt->b_len; i++) {
		memcpy(dest, data, datlen);
		dest = (char*)dest + datlen;
	}
}


/*!
 * \param nchann 	number of channel to process
 * \param proctype	constant representing the type of data processed by the filter (\c DATA_FLOAT or \c DATA_DOUBLE)
 * \param num_len 	length of the numerator of the z-transform
 * \param num 		array of values representing the numerator of the z-transform
 * \param denum_len 	length of the denumerator of the z-transform
 * \param denum		array  of values representing the the denumerator of the z-transform
 * \param paramtype	constant representing the type of data supplied as parameters (\c DATA_FLOAT or \c DATA_DOUBLE)
 * \return		Handle to the created filter
 *
 * dfswf
 */
hfilter create_filter(unsigned int nchann, unsigned int proctype, 
                      unsigned int num_len, const void *num,
		      unsigned int denum_len, const void *denum,
		      unsigned int paramtype)
{
	struct _dfilter *filt = NULL;
	void *a = NULL;
	void *xoff = NULL;
	void *b = NULL;
	void *yoff = NULL;
	int xoffsize, yoffsize;

	// Check if a denominator exists
	if ((denum_len==0) || (denum==NULL)) {
		denum_len = 0;
		denum = NULL;
	}
	else {
		denum_len--;
	}

	xoffsize = (num_len - 1) * nchann;
	yoffsize = denum_len * nchann;

	filt = malloc(sizeof(*filt));
	a = (num_len > 0) ? malloc(num_len * sizeof_data(proctype)) : NULL;
	b = (denum_len > 0) ? malloc(denum_len * sizeof_data(proctype)) : NULL;
	if (xoffsize > 0) 
		xoff = align_alloc(16, xoffsize * sizeof_data(proctype));
	if (yoffsize > 0) 
		yoff = align_alloc(16, yoffsize * sizeof_data(proctype));

	// handle memory allocation problem
	if (!filt || ((num_len > 0) && !a) || ((xoffsize > 0) && !xoff)
	    || ((denum_len > 0) && !b) || ((yoffsize > 0) && !yoff)) {
		free(filt);
		free(a);
		align_free(xoff);
		free(b);
		align_free(yoff);
		return NULL;
	}

	memset(filt, 0, sizeof(*filt));

	// prepare the filt struct
	filt->num_chann = nchann;
	filt->type = proctype;
	filt->a = a;
	filt->a_len = num_len;
	filt->xoff = xoff;
	filt->b = b;
	filt->b_len = denum_len;
	filt->yoff = yoff;

	// copy the numerator and denumerator 
	// (and normalize and convert to recursive rule)
	if (paramtype == DATATYPE_FLOAT)
		copy_numdenum_f(filt, proctype, num_len, num, denum_len+1, denum);
	else
		copy_numdenum_d(filt, proctype, num_len, num, denum_len+1, denum);

	init_filter(filt, NULL);

	return filt;
}

