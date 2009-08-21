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
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "filter.h"
#include "filter-internal.h"



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

void init_filter(hfilter filt, const void* val)
{
	int i;
	void* dest;
	int datlen = filt->num_chann*sizeof(filt->type);

	dest = filt->xoff;
	for (i=0; i<(filt->a_len-1); i++) {
		memcpy(dest, val, datlen);
		dest = (char*)dest + datlen;
	}

	dest = filt->yoff;
	for (i=0; i<filt->b_len; i++) {
		memcpy(dest, val, datlen);
		dest = (char*)dest + datlen;
	}
}

