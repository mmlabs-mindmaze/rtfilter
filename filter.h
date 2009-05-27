#ifndef FILTER_H
#define FILTER_H

#include <xmmintrin.h>

#ifdef USE_DOUBLE
typedef double	typereal;
typedef __m128d typereal_a;
#else
typedef float	typereal;
typedef __m128  typereal_a;
#endif


typedef struct _dfilter dfilter;

const dfilter* create_dfilter(unsigned int nchann, unsigned int alen, const typereal *a, unsigned int blen, const typereal *b);
void filter(const dfilter* filt, const typereal* x, typereal* y, int num_samples);
void filtera(const dfilter* filt, const typereal_a* x, typereal_a* y, unsigned int num_samples);
void reset_filter(const dfilter* filt);
void destroy_filter(const dfilter* filt);

#endif //FILTER_H
