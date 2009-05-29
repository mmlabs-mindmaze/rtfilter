#ifndef FILTER_H
#define FILTER_H

#ifdef USE_DOUBLE
typedef double	typereal;
#else
typedef float	typereal;
#endif


typedef const struct _dfilter* hfilter;

hfilter create_dfilter(unsigned int nchann, unsigned int alen, const typereal *a, unsigned int blen, const typereal *b);
void filter(hfilter filt, const typereal* x, typereal* y, int num_samples);
void filtera(hfilter filt, const typereal* xaligned, typereal* yaligned, unsigned int num_samples);
void reset_filter(hfilter filt);
void destroy_filter(hfilter filt);

#endif //FILTER_H
