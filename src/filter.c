#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#ifdef USE_SIMD
#include <xmmintrin.h>
#endif


#include "filter.h"

#define sizeof_data(type)	((type == DATATYPE_FLOAT) ? sizeof(float) : sizeof(double))

struct _dfilter
{
	unsigned int num_chann, type;
	unsigned int a_len;
	const void* a;
	unsigned int b_len;
	const void* b;
	void* xoff;
	void* yoff;
};


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


void reset_filter(hfilter filt)
{
	memset(filt->xoff, 0,
	       (filt->a_len -
		1) * filt->num_chann * sizeof_data(filt->type));
	memset(filt->yoff, 0,
	       (filt->b_len) * filt->num_chann * sizeof_data(filt->type));
}

/**************************************************************************
 *                                                                        *
 *                        Single precision version                        *
 *                               ( float )                                *
 *                                                                        *
 **************************************************************************/
#undef add_vec
#undef mul_vec
#undef zero_vec
#undef set1_vec
#undef TYPEREAL
#undef TYPEREAL_V
#undef FILTER_ALIGNED_FUNC
#undef FILTER_UNALIGNED_FUNC
#undef FILTER_FUNC
#undef CREATE_FILTER_FUNC

#define add_vec(v1,v2)			_mm_add_ps(v1,v2)
#define mul_vec(v1,v2)			_mm_mul_ps(v1,v2)
#define zero_vec()			_mm_setzero_ps()
#define set1_vec(data)			_mm_set1_ps(data)
#define TYPEREAL			float
#define TYPEREAL_V			__m128
#define FILTER_ALIGNED_FUNC		filter_fa
#define FILTER_UNALIGNED_FUNC		filter_fu
#define FILTER_FUNC			filter_f
#define CREATE_FILTER_FUNC		create_filter_f

#include "filter-templates.c"



/**************************************************************************
 *                                                                        *
 *                        Double precision version                        *
 *                               ( double )                               *
 *                                                                        *
 **************************************************************************/
#undef add_vec
#undef mul_vec
#undef zero_vec
#undef set1_vec
#undef TYPEREAL
#undef TYPEREAL_V
#undef FILTER_ALIGNED_FUNC
#undef FILTER_UNALIGNED_FUNC
#undef FILTER_FUNC
#undef CREATE_FILTER_FUNC

#define add_vec(v1,v2)			_mm_add_pd(v1,v2)
#define mul_vec(v1,v2)			_mm_mul_pd(v1,v2)
#define zero_vec()			_mm_setzero_pd()
#define set1_vec(data)			_mm_set1_pd(data)
#define TYPEREAL			double
#define TYPEREAL_V			__m128d
#define FILTER_ALIGNED_FUNC		filter_da
#define FILTER_UNALIGNED_FUNC		filter_du
#define FILTER_FUNC			filter_d
#define CREATE_FILTER_FUNC		create_filter_d

#include "filter-templates.c"


