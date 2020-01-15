/*
 *  Copyright (C) 2010-2011 Nicolas Bourdaud <nicolas.bourdaud@mindmaze.com>
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

#include <stdlib.h>
#include <string.h>

#include "filter-internal.h"
#include "rtfilter.h"

#define BUFFNS 64

struct sampler {
	struct rtf_filter filt;
	filter_proc lp_filter_fn;
	int r;
	int ci;
	int samsize;
	void* buff;
};

#define get_sampler(p)\
	((struct sampler*)(((char*)p)-offsetof(struct sampler, filt)))


static
int downsampler_filter(struct rtf_filter* filt,
                       const void* x, void* y, int ns)
{
	int i, j, nsproc;
	struct sampler* sampler = get_sampler(filt);
	const char* in = x;
	char* out = y, *tmpbuf = sampler->buff;
	int samsize = sampler->samsize, r = sampler->r;
	int nsret = 0, ci = sampler->ci;

	// Process data by chunk of BUFFNS samples maximum
	while (ns) {
		nsproc = (ns > BUFFNS) ? BUFFNS : ns;

		// Apply lowpass
		sampler->lp_filter_fn(&(sampler->filt), in, tmpbuf, nsproc);

		// Apply decimation
		i = r-1 - ci;
		j = 0;
		while (i < ns) {
			memcpy(out+j*samsize, tmpbuf+i*samsize, samsize);
			j++;
			i += r;
		}

		nsret += j;
		ns -= nsproc;
		in += nsproc*samsize;
		out += nsproc*samsize;
		ci = (ci + nsproc) % r;
	}

	sampler->ci = ci;
	return nsret;
}


static
void downsampler_init_filter(struct rtf_filter* filt, const void* in)
{
	struct sampler* sampler = get_sampler(filt);

	default_init_filter(filt, in);
	sampler->ci = 0;
}


static
void downsampler_destroy_filter(struct rtf_filter* filt)
{
	struct sampler* sampler = get_sampler(filt);

	default_free_filter(filt);
	align_free(sampler->buff);

	free(sampler);
}



DEPRECATED API_EXPORTED
hfilter rtf_create_downsampler(int nch, int type, int r)
{
	struct rtf_filter* lowpass;
	struct sampler* sampler;
	void* buff;
	double cutoff = 0.8/(double)(2*r);
	int samsize = nch*sizeof_data(type);

	// Allocate resource (+ create lowpass)
	sampler = malloc(sizeof(*sampler));
	buff = align_alloc(16, BUFFNS*samsize);
	lowpass = rtf_create_chebychev(nch, type, cutoff, 8, 0, 0.0005);
	if (sampler == NULL || buff == NULL || lowpass == NULL) {
		free(sampler);
		align_free(buff);
		rtf_destroy_filter(lowpass);
		return NULL;
	}

	// Integrate the lowpass into the sampler structure
	memcpy(&(sampler->filt), lowpass, sizeof(*lowpass));
	free((void*) lowpass);

	// fill sampler structure
	sampler->lp_filter_fn = sampler->filt.filter_fn;
	sampler->samsize = samsize;
	sampler->r = r;
	sampler->ci = 0;
	sampler->buff = buff;

	// Setup virtual methods
	sampler->filt.filter_fn = downsampler_filter;
	sampler->filt.destroy_filter_fn = downsampler_destroy_filter;
	sampler->filt.init_filter_fn = downsampler_init_filter;

	return &(sampler->filt);
}

