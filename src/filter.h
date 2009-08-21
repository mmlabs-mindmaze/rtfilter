#ifndef FILTER_H
#define FILTER_H


#define DATATYPE_FLOAT	0
#define DATATYPE_DOUBLE	1

typedef const struct _dfilter* hfilter;

hfilter create_filter_f(unsigned int nchann, unsigned int alen, const float *a, unsigned int blen, const float *b, unsigned int type);
hfilter create_filter_d(unsigned int nchann, unsigned int alen, const double *a, unsigned int blen, const double *b, unsigned int type);
void filter_f(hfilter filt, const float* x, float* y, unsigned int num_samples);
void filter_d(hfilter filt, const double* x, double* y, unsigned int num_samples);
void reset_filter(hfilter filt);
void init_filter(hfilter filt, const void* val);
void destroy_filter(hfilter filt);

#endif //FILTER_H
