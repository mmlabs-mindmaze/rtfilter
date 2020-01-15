/*
 *  Copyright (C) 2008-2011 Nicolas Bourdaud <nicolas.bourdaud@gmail.com>
 *
 *  This file is part of the rtfilter library
 *
 *  The rtfilter library is free software: you can redistribute it and/or
 *  modify it under the terms of the version 3 of the GNU Lesser General
 *  Public License as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <complex.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "filter-internal.h"
#include "rtfilter.h"

#define NS_MAX 8

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


#define DECLARE_COPY_PARAM_FN(copy_param_fn, in_t, out_t)\
	static void copy_param_fn(int len, void* dst, const void* src,\
	                          const void* factor, int isdenum)\
	{\
		int i;\
		const in_t *tsrc = src;\
		out_t *tdst = dst;\
		in_t normfactor = (!factor) ? 1.0 : *((const in_t*)factor);\
		if (!isdenum) {\
			for (i = 0; i < len; i++)\
				tdst[i] = tsrc[i] / normfactor;\
		} else {\
			for (i = 0; i < len; i++)\
				tdst[i] = -tsrc[i+1] / normfactor;\
		}\
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

typedef void (*copy_param_proc)(int, void*, const void*, const void*, int);

static
copy_param_proc convtab[4][4] = {
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
set_filterfn_proc setfiltfnproctab[4][4] = {
	[RTF_FLOAT] = {[RTF_FLOAT] = set_filterfn_f,
		       [RTF_CFLOAT] = set_filterfn_fcf},
	[RTF_DOUBLE] = {[RTF_DOUBLE] = set_filterfn_d,
		        [RTF_CDOUBLE] = set_filterfn_dcd},
	[RTF_CFLOAT] = {[RTF_CFLOAT] = set_filterfn_cf},
	[RTF_CDOUBLE] = {[RTF_CDOUBLE] = set_filterfn_cd}
};


typedef void (*init_filter_proc)(struct rtf_filter*, const void*);

static
const init_filter_proc init_filter_proc_tab[4][4] = {
	[RTF_FLOAT] = {[RTF_FLOAT] = init_filter_f,
		       [RTF_CFLOAT] = init_filter_fcf},
	[RTF_DOUBLE] = {[RTF_DOUBLE] = init_filter_d,
		        [RTF_CDOUBLE] = init_filter_dcd},
	[RTF_CFLOAT] = {[RTF_CFLOAT] = init_filter_cf},
	[RTF_CDOUBLE] = {[RTF_CDOUBLE] = init_filter_cd}
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
		       (filt->b_len) * filt->num_chann * sizeof_data(
			       filt->outtype));
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
#endif /* if HAVE_POSIX_MEMALIGN */
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
void default_init_filter(struct rtf_filter* filt, const void* data)
{
	int itp = filt->intype, otp = filt->outtype;

	if (data == NULL) {
		reset_filter(filt);
		return;
	}

	init_filter_proc_tab[itp][otp](filt, data);
}


LOCAL_FN
void default_free_filter(struct rtf_filter* filt)
{
	free((void*) filt->a);
	free((void*) filt->b);
	align_free(filt->xoff);
	align_free(filt->yoff);
}


static
void default_destroy_filter(struct rtf_filter* filt)
{
	default_free_filter(filt);
	free(filt);
}


static
void destroy_filter_seq(struct rtf_filter* filt)
{
	int i;
	struct filter_seq * seq = (struct filter_seq*) filt;

	for (i = 0; i < seq->num_filters; i++)
		default_destroy_filter(seq->filters[i]);

	free(seq->filters);
	align_free(seq->buffer);
	free(seq);
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
hfilter rtf_create_filter(int nchann, int proctype,
                          int numlen, const void *num,
                          int denlen, const void *den,
                          int paramtype)
{
	struct rtf_filter *filt = NULL;
	void *a = NULL;
	void *xoff = NULL;
	void *b = NULL;
	void *yoff = NULL;
	int xoffsize, yoffsize;
	int intype, outtype, adv_intype, adv_outtype;

	define_types(proctype, paramtype, &adv_intype, &adv_outtype);

	// Processing complex data with real transfer function is equivalent
	// (and faster) to process real and imaginary part as 2 channels
	if (  (proctype & RTF_COMPLEX_MASK)
	   && !(paramtype & RTF_COMPLEX_MASK)) {
		proctype &= ~RTF_COMPLEX_MASK;
		nchann *= 2;
	}

	define_types(proctype, paramtype, &intype, &outtype);

	// Check if a denominator exists
	if ((denlen == 0) || (den == NULL)) {
		denlen = 0;
		den = NULL;
	} else {
		denlen--;
	}

	xoffsize = (numlen - 1) * nchann;
	yoffsize = denlen * nchann;

	filt = malloc(sizeof(*filt));
	a = (numlen > 0) ? malloc(numlen * sizeof_data(outtype)) : NULL;
	b = (denlen > 0) ? malloc(denlen * sizeof_data(outtype)) : NULL;
	if (xoffsize > 0)
		xoff = align_alloc(16, xoffsize * sizeof_data(intype));

	if (yoffsize > 0)
		yoff = align_alloc(16, yoffsize * sizeof_data(outtype));

	// handle memory allocation problem
	if (  !filt || ((numlen > 0) && !a) || ((xoffsize > 0) && !xoff)
	   || ((denlen > 0) && !b) || ((yoffsize > 0) && !yoff)) {
		free(filt);
		free(a);
		align_free(xoff);
		free(b);
		align_free(yoff);
		return NULL;
	}

	// copy the numerator and denerator
	// (and normalize and convert to recursive rule)
	convtab[paramtype][outtype](numlen, a, num, den, 0);
	convtab[paramtype][outtype](denlen, b, den, den, 1);


	// prepare the filt struct
	memset(filt, 0, sizeof(*filt));
	filt->init_filter_fn = default_init_filter;
	filt->destroy_filter_fn = default_destroy_filter;
	filt->num_chann = nchann;
	filt->intype = intype;
	filt->outtype = outtype;
	filt->a = a;
	filt->a_len = numlen;
	filt->xoff = xoff;
	filt->b = b;
	filt->b_len = denlen;
	filt->yoff = yoff;
	filt->advertised_intype = adv_intype;
	filt->advertised_outtype = adv_outtype;
	filt->dispatch_code = 0;

	setfiltfnproctab[intype][outtype](filt);


	rtf_init_filter(filt, NULL);

	return filt;
}


/* typed with char instead of void to prevent compiler warning */
static inline
void swap_ptr(char ** ptr1, char ** ptr2)
{
	char * tmp = *ptr1;
	*ptr1 = *ptr2;
	*ptr2 = tmp;
}


/**
 * rtf_filter_seq() - apply filter cascade
 * @filt: initialized rtf_filter structure
 * @ns: number of samples (up to NS_MAX)
 * @in: array of (ns * nch) input data
 * @out: array of (ns * nch) output data
 */
static
int rtf_filter_seq(struct rtf_filter * filt, void const * _in,
                   void * _out, int ns)
{
	int i;
	int num_samples;
	char const * in = _in;
	char * out = _out;
	char * tmp_in, * tmp_out;
	struct rtf_filter * f;
	struct filter_seq * seq = (struct filter_seq*) filt;

	while (ns > 0) {
		num_samples = MIN(ns, NS_MAX);

		if (seq->num_filters % 2 == 1) {
			tmp_in = out;
			tmp_out = seq->buffer;
		} else {
			tmp_in = seq->buffer;
			tmp_out = out;
		}

		f = seq->filters[0];
		f->filter_fn(f, in, tmp_out, num_samples);

		for (i = 1; i < seq->num_filters; i++) {
			f = seq->filters[i];
			swap_ptr(&tmp_in, &tmp_out);
			(void) f->filter_fn(f, tmp_in, tmp_out, num_samples);
		}

		ns -= num_samples;
		in += num_samples * f->num_chann * sizeof_data(f->intype);
		out += num_samples * f->num_chann * sizeof_data(f->outtype);
	}

	return ns;
}


/**
 * rtf_filter_combine() - combine vector of filters into one
 * @nfilters: number of filters to combine
 * @filters: array of pointers to filters to combine
 *
 * On success, the combination-filter assumes ownership of all the filters
 * passed as arguments.
 *
 * Returns: a pointer to rtf_filter structure handling the combination,
 *          NULL on error
 */
API_EXPORTED
struct rtf_filter* rtf_filter_combine(int nfilters, struct
                                      rtf_filter ** filters)
{
	int i;
	struct filter_seq * seq;
	struct rtf_filter * last;

	if (nfilters <= 0 || filters == NULL)
		return NULL;

	/* ensure all the input filters exist */
	for (i = 0; i < nfilters; i++) {
		if (filters[i] == NULL)
			return NULL;
	}

	if (nfilters == 1)
		return *filters;

	seq = malloc(sizeof(*seq));
	if (seq == NULL)
		return NULL;

	last = filters[nfilters - 1];

	*seq = (struct filter_seq) {0};
	seq->filters = malloc(sizeof(*seq->filters) * nfilters);
	seq->buffer = align_alloc(16,
	                          NS_MAX * last->num_chann *
	                          sizeof_data(last->outtype));
	if (seq->filters == NULL || seq->buffer == NULL) {
		destroy_filter_seq((struct rtf_filter*) seq);
		return NULL;
	}

	memcpy(seq->filters, filters, sizeof(*seq->filters) * nfilters);
	seq->num_filters = nfilters;
	seq->filter.filter_fn = rtf_filter_seq;
	seq->filter.destroy_filter_fn = destroy_filter_seq;
	seq->filter.num_chann = last->num_chann;

	return (struct rtf_filter*) seq;
}


hfilter rtf_create_filter_coeffs(int nchann, int data_type,
                                 struct rtf_coeffs * coeffs)
{
	assert(nchann > 0 && coeffs != NULL);

	if (coeffs->is_complex) {
		return rtf_create_filter(nchann, data_type,
		                         coeffs->coeffs.cplx.num_len,
		                         coeffs->coeffs.cplx.num,
		                         coeffs->coeffs.cplx.denum_len,
		                         coeffs->coeffs.cplx.denum,
		                         RTF_CDOUBLE);
	} else {
		return rtf_create_filter(nchann, data_type,
		                         coeffs->coeffs.real.num_len,
		                         coeffs->coeffs.real.num,
		                         coeffs->coeffs.real.denum_len,
		                         coeffs->coeffs.real.denum,
		                         RTF_DOUBLE);
	}
}


API_EXPORTED
void rtf_filter_set_lazy_init(hfilter filt, int do_lazy_init)
{
	filt->lazy_init = do_lazy_init;
}


API_EXPORTED HOTSPOT
int rtf_filter(hfilter filt, const void* x, void* y, int ns)
{
	if (unlikely(filt->lazy_init)) {
		rtf_init_filter(filt, x);
		filt->lazy_init = 0;
	}

	return filt->filter_fn(filt, x, y, ns);
}


API_EXPORTED
int rtf_get_type(hfilter filt, int in)
{
	if (filt == NULL)
		return -1;

	return (in ? filt->advertised_intype : filt->advertised_outtype);
}


static
const char rtf_version_string[] = PACKAGE_STRING
#if HAVE_CPUID
                                  " (compiled with cpu detection)";
#elif __SSE3__
                                  " (compiled with sse3)";
#elif __SSE2__
                                  " (compiled with sse2)";
#elif __SSE__
                                  " (compiled with sse2)";
#elif __ARM_NEON__
                                  " (compiled with NEON)";
#else
                                  " (compiled with no SIMD)";
#endif /* if HAVE_CPUID */


API_EXPORTED
size_t rtf_get_version(char* string, size_t len, int line)
{
	if (line > 0)
		return 0;

	strncpy(string, rtf_version_string, len-1);
	string[len-1] = '\0';
	return strlen(string);
}


/**
 * rtf_coeffs_destroy() - destroy rtf_coeffs structure
 * @coeffs: rtf_coeffs structure to destroy
 */
void rtf_coeffs_destroy(struct rtf_coeffs * coeffs)
{
	if (coeffs != NULL) {
		if (coeffs->is_complex) {
			free(coeffs->coeffs.cplx.num);
			free(coeffs->coeffs.cplx.denum);
		} else {
			free(coeffs->coeffs.real.num);
			free(coeffs->coeffs.real.denum);
		}

		free(coeffs);
	}
}


static inline
int is_complex_filter(const struct rtf_filter * filt)
{
	return (filt->outtype == RTF_CFLOAT || filt->outtype == RTF_CDOUBLE);
}


/**
 * rtf_get_coeffs() - get filter coeffs
 * @filt: initialized rtfilter
 *
 * Note: the copy function we get from convtab is a tad too specific for our use:
 * it expects a flag specifying whether we copy the num/denum, but processes and
 * transforms the denum in a way we do not want here: we wand the raw values.
 * However, we still need to multiply their values by -1 so as to ensure their
 * values are the same as with scipy.
 *
 * Return: newly allocated rtf_coeffs structure on success, NULL on error
 */
API_EXPORTED
struct rtf_coeffs* rtf_get_coeffs(const struct rtf_filter * filt)
{
	int i;
	struct rtf_coeffs * coeffs;
	copy_param_proc copy_fn;

	coeffs = malloc(sizeof(*coeffs));
	if (coeffs == NULL)
		return NULL;

	memset(coeffs, 0, sizeof(*coeffs));

	coeffs->is_complex = is_complex_filter(filt);
	if (coeffs->is_complex) {
		coeffs->coeffs.cplx.num_len = filt->a_len;
		coeffs->coeffs.cplx.denum_len = filt->b_len + 1;
		coeffs->coeffs.cplx.num = malloc(
			coeffs->coeffs.cplx.denum_len * sizeof(rtf_cdouble));
		coeffs->coeffs.cplx.denum = malloc(
			coeffs->coeffs.cplx.num_len * sizeof(rtf_cdouble));
		if (  coeffs->coeffs.cplx.num == NULL
		   || coeffs->coeffs.cplx.denum == NULL)
			goto enomem;

		copy_fn = convtab[filt->outtype][RTF_CDOUBLE];
		copy_fn(filt->a_len, coeffs->coeffs.cplx.num, filt->a, NULL,
		        0);
		coeffs->coeffs.cplx.denum[0] = (rtf_cdouble) {1.};
		copy_fn(filt->b_len, coeffs->coeffs.cplx.denum + 1, filt->b,
		        NULL, 0);
		for (i = 1; i < coeffs->coeffs.cplx.denum_len; i++) {
#ifdef _MSC_VER
			coeffs->coeffs.cplx.denum[i] = cmul_d(
				coeffs->coeffs.cplx.denum[i], -1.);
#else
			coeffs->coeffs.cplx.denum[i] *= -1.;
#endif
		}
	} else {
		coeffs->coeffs.real.num_len = filt->a_len;
		coeffs->coeffs.real.denum_len = filt->b_len + 1;
		coeffs->coeffs.real.num = malloc(coeffs->coeffs.real.num_len *
		                                 sizeof(double));
		coeffs->coeffs.real.denum = malloc(
			coeffs->coeffs.real.denum_len * sizeof(double));
		if (  coeffs->coeffs.real.num == NULL
		   || coeffs->coeffs.real.denum == NULL)
			goto enomem;

		copy_fn = convtab[filt->outtype][RTF_DOUBLE];
		copy_fn(filt->a_len, coeffs->coeffs.real.num, filt->a, NULL, 0);
		coeffs->coeffs.real.denum[0] = 1.;
		copy_fn(filt->b_len, coeffs->coeffs.real.denum + 1, filt->b,
		        NULL, 0);
		for (i = 1; i < coeffs->coeffs.real.denum_len; i++) {
			coeffs->coeffs.real.denum[i] *= -1.;
		}
	}

	return coeffs;

enomem:
	rtf_coeffs_destroy(coeffs);
	return NULL;
}
