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



void* align_alloc(size_t alignment, size_t size)
{
	void* memptr = NULL;
	if (posix_memalign(&memptr, alignment, size))
		return NULL;
	return memptr;
}


void  align_free(void* memptr)
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


/*! \fn void init_filter(hfilter filt, const void* data)
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
