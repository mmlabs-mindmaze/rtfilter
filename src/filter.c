/*
      Copyright (C) 2008-2010 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

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

LOCAL_FN
size_t sizeof_data(int type)
{
	size_t dsize = 0;
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
	[RTF_FLOAT] = {[RTF_FLOAT] = copy_param_ff, 
	               [RTF_DOUBLE] = copy_param_fd,
                       [RTF_CFLOAT] = copy_param_fcf,
                       [RTF_CDOUBLE] = copy_param_fcd},
	[RTF_DOUBLE] = {[RTF_FLOAT] = copy_param_df, 
	                [RTF_DOUBLE] = copy_param_dd,
			[RTF_CFLOAT] = copy_param_dcf, 
			[RTF_CDOUBLE] = copy_param_dcd},
	[RTF_CFLOAT] = {[RTF_CFLOAT] = copy_param_cfcf, 
	                [RTF_CDOUBLE] = copy_param_cfcd},
	[RTF_CDOUBLE] = {[RTF_CFLOAT] = copy_param_cdcf,
	                 [RTF_CDOUBLE] = copy_param_cdcd},
};


static
filter_proc filtproctab[4][4] = {
	[RTF_FLOAT] = {[RTF_FLOAT] = filter_f, [RTF_CFLOAT] = filter_fcf},
	[RTF_DOUBLE] = {[RTF_DOUBLE] = filter_d,[RTF_CDOUBLE] = filter_dcd},
};



static
void reset_filter(hfilter filt)
{
	if (filt->xoff)
		memset(filt->xoff, 0,
		       (filt->a_len -
			1) * filt->num_chann * sizeof_data(filt->intype));
	
	if (filt->yoff)
		memset(filt->yoff, 0,
		       (filt->b_len) * filt->num_chann * sizeof_data(filt->outtype));
}


LOCAL_FN
void* align_alloc(size_t alignment, size_t size)
{
#if HAVE_POSIX_MEMALIGN
	void* memptr = NULL;
	if (posix_memalign(&memptr, alignment, size))
		return NULL;
	return memptr;
#else
	void* origptr = malloc(sizeof(void*) + alignment + size);
	if (origptr == NULL)
		return NULL;
	char* ptr = ((char*)origptr) + sizeof(void*);
	ptr += alignment - ((uintptr_t)ptr)%alignment;
	*(void**)(ptr-sizeof(origptr)) = origptr;
	return ptr;
#endif
}


LOCAL_FN
void  align_free(void* memptr)
{
#if HAVE_POSIX_MEMALIGN
	free(memptr);
#else
	if (memptr == NULL)
		return;
	free(*(((void**)memptr)-1));
#endif
}


static
void define_types(int proctp, int paramtp, int* intp, int* outtp)
{
	int tpi, tpo;

	tpi = proctp;
	tpo = proctp;

	if (paramtp & RTF_COMPLEX_MASK)
		tpo |= RTF_COMPLEX_MASK;

	*intp = tpi;
	*outtp = tpo;
}

LOCAL_FN
void default_init_filter(const struct rtf_filter* filt, const void* data)
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


LOCAL_FN
void default_free_filter(const struct rtf_filter* filt)
{
	free((void *) (filt->a));
	free((void *) (filt->b));
	align_free(filt->xoff);
	align_free(filt->yoff);
}

static
void default_destroy_filter(const struct rtf_filter* filt)
{
	default_free_filter(filt);
	free((void*) filt);
}

API_EXPORTED
void rtf_destroy_filter(hfilter filt)
{
	if (!filt)
		return;

	filt->destroy_filter_fn(filt);
}


API_EXPORTED
void rtf_init_filter(hfilter filt, const void *data)
{
	filt->init_filter_fn(filt, data);
}


API_EXPORTED
hfilter rtf_create_filter(unsigned int nchann, int proctype, 
                      unsigned int blen, const void *b,
		      unsigned int alen, const void *a,
		      int paramtype)
{
	struct rtf_filter *filt = NULL;
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
	filt->init_filter_fn = default_init_filter;
	filt->destroy_filter_fn = default_destroy_filter;
	filt->num_chann = nchann;
	filt->intype = intype;
	filt->outtype = outtype;
	filt->a = num;
	filt->a_len = blen;
	filt->xoff = xoff;
	filt->b = denum;
	filt->b_len = alen;
	filt->yoff = yoff;


	rtf_init_filter(filt, NULL);

	return filt;
}


API_EXPORTED
unsigned int rtf_filter(hfilter filt, const void* x, void* y, unsigned int ns)
{
	return filt->filter_fn(filt, x, y, ns);
}
