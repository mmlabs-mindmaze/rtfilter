/*
    Copyright (C) 2008-2011 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    This file is part of the rtfilter library

    The rtfilter library is free software: you can redistribute it and/or
    modify it under the terms of the version 3 of the GNU Lesser General
    Public License as published by the Free Software Foundation.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef FILTER_INTERNAL_H
#define FILTER_INTERNAL_H

#include <stddef.h>

typedef unsigned int (*filter_proc)(const struct rtf_filter*, const void*,
                                    void*, unsigned int);
typedef void (*set_filterfn_proc)(struct rtf_filter*);
typedef void (*init_filter_proc)(const struct rtf_filter*, const void*);
typedef	void (*destroy_filter_proc)(const struct rtf_filter*);

struct rtf_filter
{
	filter_proc filter_fn;
	int dispatch_code;
	unsigned int num_chann;
	unsigned int a_len;
	unsigned int b_len;
	const void* a;
	const void* b;
	void* xoff;
	void* yoff;

	init_filter_proc init_filter_fn;
	destroy_filter_proc destroy_filter_fn;
	int advertised_intype, advertised_outtype, intype, outtype;
};

LOCAL_FN
void default_init_filter(const struct rtf_filter* filt, const void* data);
LOCAL_FN
void default_free_filter(const struct rtf_filter* filt);

LOCAL_FN size_t sizeof_data(int type);
LOCAL_FN void* align_alloc(size_t alignment, size_t size);
LOCAL_FN void  align_free(void* memptr);

LOCAL_FN void set_filterfn_f(struct rtf_filter* filt);
LOCAL_FN void set_filterfn_d(struct rtf_filter* filt);
LOCAL_FN void set_filterfn_fcf(struct rtf_filter* filt);
LOCAL_FN void set_filterfn_dcd(struct rtf_filter* filt);
LOCAL_FN void set_filterfn_cf(struct rtf_filter* filt);
LOCAL_FN void set_filterfn_cd(struct rtf_filter* filt);

#endif //FILTER_INTERNAL_H
