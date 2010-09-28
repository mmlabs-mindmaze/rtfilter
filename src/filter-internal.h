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
#ifndef FILTER_INTERNAL_H
#define FILTER_INTERNAL_H


#if HAVE_CONFIG_H
# include <config.h>
#endif


struct rtf_filter
{
	void (*filter_fn)(const struct rtf_filter*, const void*, void*, unsigned int);
	void (*init_filter_fn)(const struct rtf_filter*, const void*);
	void (*destroy_filter_fn)(const struct rtf_filter*);
	unsigned int num_chann, intype, outtype;
	unsigned int a_len;
	const void* a;
	unsigned int b_len;
	const void* b;
	void* xoff;
	void* yoff;
};

LOCAL_FN
void default_init_filter(const struct rtf_filter* filt, const void* data);
LOCAL_FN
void default_free_filter(const struct rtf_filter* filt);

LOCAL_FN
void filter_f(const struct rtf_filter* filt, const void* x, void* y, unsigned int ns);
LOCAL_FN
void filter_d(const struct rtf_filter* filt, const void* x, void* y, unsigned int ns);
LOCAL_FN
void filter_fcf(const struct rtf_filter* filt, const void* x, void* y, unsigned int ns);
LOCAL_FN
void filter_dcd(const struct rtf_filter* filt, const void* x, void* y, unsigned int ns);

#endif //FILTER_INTERNAL_H
