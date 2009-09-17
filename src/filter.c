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


static void reset_filter(hfilter filt)
{
	if (filt->xoff)
		memset(filt->xoff, 0,
		       (filt->a_len -
			1) * filt->num_chann * sizeof_data(filt->type));
	
	if (filt->yoff)
		memset(filt->yoff, 0,
		       (filt->b_len) * filt->num_chann * sizeof_data(filt->type));
}


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
void init_filter(hfilter filt, const void *data)
{
	unsigned int i;
	void *dest;
	int datlen = filt->num_chann * sizeof(filt->type);

	if (data == NULL) {
		reset_filter(filt);
		return;
	}

	dest = filt->xoff;
	if (dest) {
		for (i = 0; i < (filt->a_len - 1); i++) {
			memcpy(dest, data, datlen);
			dest = (char *) dest + datlen;
		}
	}

	dest = filt->yoff;
	if (dest) {
		for (i = 0; i < filt->b_len; i++) {
			memcpy(dest, data, datlen);
			dest = (char *) dest + datlen;
		}
	}
}

/*!
 * \param nchann 	number of channel to process
 * \param proctype	constant representing the type of data processed by the filter (\c DATA_FLOAT or \c DATA_DOUBLE)
 * \param blen	 	length of the array representing the numerator of the z-transform
 * \param b 		array of values representing the numerator of the z-transform ordered increasingly with the powers of \f$z^{-1}\f$
 * \param alen	 	length of the array representing the denominator of the z-transform
 * \param a		array  of values representing the the denominator of the z-transform ordered increasingly with the powers of \f$z^{-1}\f$
 * \param paramtype	constant representing the type of data supplied as parameters (\c DATA_FLOAT or \c DATA_DOUBLE)
 * \return		Handle to the created filter
 *
 * This function create and initialize the necessary resources for a digital filter to process a number of channels specified by the parameter \c nchann.
 * 
 * The type of data the filter will process is specified once for all by the \c proctype parameter. Set it to \c DATATYPE_FLOAT for processing \c float values or to \c DATATYPE_DOUBLE for \c double values thus allowing to call respectively filter_f() or filter_d() with the filter handle returned by the function. 
 * 
 * The parameters \c b and \c a are used to specify the coefficients of the z-transform of the filter (\c blen and \c alen are respectively \f$nb+1\f$ and \f$na+1\f$):
 * \f[
 * H(z) = \frac{b_0 + b_1z^{-1} + b_2z^{-2}+\ldots+b_{nb}z^{-nb}}{a_0 + a_1z^{-1} + a_2z^{-2}+\ldots+a_{na}z^{-na}}
 * \f]
 * The arrays pointed by \c b and \c a can be either \c float or \c double values. This is specified by the \c paramtype parameter by taking the value \c DATATYPE_FLOAT or \c DATATYPE_DOUBLE.\n
 * The integer \c alen can also be \c 0 or the pointer \c a be \c NULL. In that case, the denominator is 1 thus specifying a filter with a finite impulse response.
 */
hfilter create_filter(unsigned int nchann, unsigned int proctype, 
                      unsigned int blen, const void *b,
		      unsigned int alen, const void *a,
		      unsigned int paramtype)
{
	struct _dfilter *filt = NULL;
	void *num = NULL;
	void *xoff = NULL;
	void *denum = NULL;
	void *yoff = NULL;
	int xoffsize, yoffsize;

	// Check if a denominator exists
	if ((alen==0) || (a==NULL)) {
		alen = 0;
		a = NULL;
	}
	else {
		alen--;
	}

	xoffsize = (blen - 1) * nchann;
	yoffsize = alen * nchann;

	filt = malloc(sizeof(*filt));
	num = (blen > 0) ? malloc(blen * sizeof_data(proctype)) : NULL;
	denum = (alen > 0) ? malloc(alen * sizeof_data(proctype)) : NULL;
	if (xoffsize > 0) 
		xoff = align_alloc(16, xoffsize * sizeof_data(proctype));
	if (yoffsize > 0) 
		yoff = align_alloc(16, yoffsize * sizeof_data(proctype));

	// handle memory allocation problem
	if (!filt || ((blen > 0) && !num) || ((xoffsize > 0) && !xoff)
	    || ((alen > 0) && !denum) || ((yoffsize > 0) && !yoff)) {
		free(filt);
		free(num);
		align_free(xoff);
		free(denum);
		align_free(yoff);
		return NULL;
	}

	memset(filt, 0, sizeof(*filt));

	// prepare the filt struct
	filt->num_chann = nchann;
	filt->type = proctype;
	filt->a = num;
	filt->a_len = blen;
	filt->xoff = xoff;
	filt->b = denum;
	filt->b_len = alen;
	filt->yoff = yoff;

	// copy the numerator and denumerator 
	// (and normalize and convert to recursive rule)
	if (paramtype == DATATYPE_FLOAT)
		copy_numdenum_f(filt, blen, b, alen+1, a);
	else
		copy_numdenum_d(filt, blen, b, alen+1, a);

	init_filter(filt, NULL);

	return filt;
}

