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
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <memory.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <stdint.h>
#include "filter.h"
#include "filter-internal.h"

static size_t sizeof_data(int type)
{
	size_t dsize;
	if (type == RTF_FLOAT)
		dsize = sizeof(float);
	else if (type == RTF_DOUBLE)
		dsize = sizeof(double);
	else if (type == RTF_CFLOAT)
		dsize = sizeof(complex float);
	else if (type == RTF_CDOUBLE)
		dsize = sizeof(complex double);

	return dsize;
}


#define DECLARE_COPY_PARAM_FN(copy_param_fn, in_t, out_t)		\
static void copy_param_fn(unsigned int len, void* dst, const void* src, const void* factor, int isdenum)	\
{									\
	unsigned int i;							\
	const in_t *tsrc = src;						\
	out_t *tdst = dst;						\
	in_t normfactor = (!factor) ? 1.0 : *((const in_t*)factor);	\
									\
	if (!isdenum)							\
		for (i=0; i<len; i++)					\
			tdst[i] = tsrc[i] / normfactor;			\
	else								\
		for (i=0; i<len; i++)					\
			tdst[i] = -tsrc[i+1] / normfactor;		\
}		

DECLARE_COPY_PARAM_FN(copy_param_ff, float, float)
DECLARE_COPY_PARAM_FN(copy_param_fd, float, double)
DECLARE_COPY_PARAM_FN(copy_param_df, double, float)
DECLARE_COPY_PARAM_FN(copy_param_dd, double, double)
DECLARE_COPY_PARAM_FN(copy_param_fcf, float, complex float)
DECLARE_COPY_PARAM_FN(copy_param_fcd, float, complex double)
DECLARE_COPY_PARAM_FN(copy_param_dcf, double, complex float)
DECLARE_COPY_PARAM_FN(copy_param_dcd, double, complex double)
DECLARE_COPY_PARAM_FN(copy_param_cfcf, complex float, complex float)
DECLARE_COPY_PARAM_FN(copy_param_cfcd, complex float, complex double)
DECLARE_COPY_PARAM_FN(copy_param_cdcf, complex double, complex float)
DECLARE_COPY_PARAM_FN(copy_param_cdcd, complex double, complex double)

typedef void (*copy_param_proc)(unsigned int, void*, const void*, const void*, int);

static copy_param_proc convtab[4][4] = {
	[RTF_FLOAT] = {[RTF_FLOAT] = copy_param_ff, [RTF_DOUBLE] = copy_param_fd, [RTF_CFLOAT] = copy_param_fcf, [RTF_CDOUBLE] = copy_param_fcd},
	[RTF_DOUBLE] = {[RTF_FLOAT] = copy_param_df, [RTF_DOUBLE] = copy_param_dd, [RTF_CFLOAT] = copy_param_dcf, [RTF_CDOUBLE] = copy_param_dcd},
	[RTF_CFLOAT] = {[RTF_CFLOAT] = copy_param_cfcf, [RTF_CDOUBLE] = copy_param_cfcd},
	[RTF_CDOUBLE] = {[RTF_CFLOAT] = copy_param_cdcf, [RTF_CDOUBLE] = copy_param_cdcd},
};

typedef void (*filter_proc)(const struct _dfilter*, const void*, void*, unsigned int);

static filter_proc filtproctab[4][4] = {
	[RTF_FLOAT] = {[RTF_FLOAT] = filter_f, [RTF_CFLOAT] = filter_fcf},
	[RTF_DOUBLE] = {[RTF_DOUBLE] = filter_d, [RTF_CDOUBLE] = filter_dcd},
};



static void reset_filter(hfilter filt)
{
	if (filt->xoff)
		memset(filt->xoff, 0,
		       (filt->a_len -
			1) * filt->num_chann * sizeof_data(filt->intype));
	
	if (filt->yoff)
		memset(filt->yoff, 0,
		       (filt->b_len) * filt->num_chann * sizeof_data(filt->outtype));
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

static void define_types(int proctp, int paramtp, int* intp, int* outtp)
{
	int tpi, tpo;

	tpi = proctp;
	tpo = proctp;

	if (paramtp & RTF_COMPLEX_MASK)
		tpo |= RTF_COMPLEX_MASK;

	*intp = tpi;
	*outtp = tpo;

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
	void *dest;
	int nc = filt->num_chann, itp = filt->intype, otp = filt->outtype;
	unsigned int i, isize = sizeof_data(itp), osize = sizeof_data(otp);

	if (data == NULL) {
		reset_filter(filt);
		return;
	}

	dest = filt->xoff;
	if (dest) {
		for (i = 0; i < (filt->a_len - 1); i++) {
			convtab[itp][itp](nc, dest, data, NULL, 0);
			dest = (char *) dest + nc*isize;
		}
	}

	dest = filt->yoff;
	if (dest) {
		for (i = 0; i < filt->b_len; i++) {
			convtab[itp][otp](nc, dest, data, NULL, 0);
			dest = (char *) dest + nc*osize;
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
 * The type of data the filter will process is specified once for all by the \c proctype parameter. Set it to \c RTF_FLOAT for processing \c float values or to \c RTF_DOUBLE for \c double values thus allowing to call respectively filter_f() or filter_d() with the filter handle returned by the function. 
 * 
 * The parameters \c b and \c a are used to specify the coefficients of the z-transform of the filter (\c blen and \c alen are respectively \f$nb+1\f$ and \f$na+1\f$):
 * \f[
 * H(z) = \frac{b_0 + b_1z^{-1} + b_2z^{-2}+\ldots+b_{nb}z^{-nb}}{a_0 + a_1z^{-1} + a_2z^{-2}+\ldots+a_{na}z^{-na}}
 * \f]
 * The arrays pointed by \c b and \c a can be either \c float or \c double values. This is specified by the \c paramtype parameter by taking the value \c RTF_FLOAT or \c RTF_DOUBLE.\n
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
	int intype, outtype;

	define_types(proctype, paramtype, &intype, &outtype);

	// Check if a denominator exists
	if ((alen==0) || (a==NULL)) {
		alen = 0;
		a = NULL;
	} else {
		alen--;
	}

	xoffsize = (blen - 1) * nchann;
	yoffsize = alen * nchann;

	filt = malloc(sizeof(*filt));
	num = (blen > 0) ? malloc(blen * sizeof_data(outtype)) : NULL;
	denum = (alen > 0) ? malloc(alen * sizeof_data(outtype)) : NULL;
	if (xoffsize > 0) 
		xoff = align_alloc(16, xoffsize * sizeof_data(intype));
	if (yoffsize > 0) 
		yoff = align_alloc(16, yoffsize * sizeof_data(outtype));

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

	// copy the numerator and denumerator 
	// (and normalize and convert to recursive rule)
	convtab[paramtype][outtype](blen, num, b, a, 0);
	convtab[paramtype][outtype](alen, denum, a, a, 1);


	// prepare the filt struct
	memset(filt, 0, sizeof(*filt));
	filt->filter_fn = filtproctab[intype][outtype];
	filt->num_chann = nchann;
	filt->intype = intype;
	filt->outtype = outtype;
	filt->a = num;
	filt->a_len = blen;
	filt->xoff = xoff;
	filt->b = denum;
	filt->b_len = alen;
	filt->yoff = yoff;


	init_filter(filt, NULL);

	return filt;
}


/** 
 * \param filt	handle to a digital filter resource
 * \param x	pointer to an array of input data
 * \param y	pointer to an array of output data
 * \param ns	number of time sample that should be processed
 *
 * This function apply the filter on the data specified by pointer \c x and
 * write the filter data on the array pointed by \c y. The arrays pointed by
 * \c x and \c y must be made of values whose correspond to the type
 * specified at the creation of the filter.
 *
 * Their number of elements have to be equal to \c ns multiplied by the
 * number of channels processed. The arrays should be packed by channels
 * with the following pattern:
 * | S1C1 | S1C2 | ... | S1Ck | S2C1 | S2C2 | .... | S2Ck | ... | SnCk |
 * where SiCj refers to the data in the i-th sample of the j-th channel.
 *
 * \sa create_filter()
 */
void filter(hfilter filt, const void* x, void* y, unsigned int ns)
{
	filt->filter_fn(filt, x, y, ns);
}
