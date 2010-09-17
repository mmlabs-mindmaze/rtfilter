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
/** \internal
 * \file filter-internal.h
 * \brief Definitions of implementation specifics of rtfilter library
 * \author Nicolas Bourdaud
 * \sa common-filter.h
 */
#ifndef FILTER_INTERNAL_H
#define FILTER_INTERNAL_H


#if HAVE_CONFIG_H
# include <config.h>
#endif


struct rtf_filter
{
	void (*filter_fn)(const struct rtf_filter*, const void*, void*, unsigned int);
	unsigned int num_chann, intype, outtype;
	unsigned int a_len;
	const void* a;
	unsigned int b_len;
	const void* b;
	void* xoff;
	void* yoff;
};

LOCAL_FN
void filter_f(const struct rtf_filter* filt, const void* x, void* y, unsigned int ns);
LOCAL_FN
void filter_d(const struct rtf_filter* filt, const void* x, void* y, unsigned int ns);
LOCAL_FN
void filter_fcf(const struct rtf_filter* filt, const void* x, void* y, unsigned int ns);
LOCAL_FN
void filter_dcd(const struct rtf_filter* filt, const void* x, void* y, unsigned int ns);

#endif //FILTER_INTERNAL_H
