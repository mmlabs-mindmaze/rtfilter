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
#ifndef FILTER_H
#define FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#define DATATYPE_FLOAT	0
#define DATATYPE_DOUBLE	1

typedef const struct _dfilter* hfilter;

hfilter create_filter_f(unsigned int nchann, 
                        unsigned int alen, const float *a,
			unsigned int blen, const float *b,
			unsigned int type);
hfilter create_filter_d(unsigned int nchann,
                        unsigned int alen, const double *a,
			unsigned int blen, const double *b,
			unsigned int type);
void filter_f(hfilter filt, const float* x, float* y, unsigned int ns);
void filter_d(hfilter filt, const double* x, double* y, unsigned int ns);
void reset_filter(hfilter filt);
void init_filter(hfilter filt, const void* val);
void destroy_filter(hfilter filt);

#ifdef __cplusplus
}
#endif

#endif //FILTER_H
