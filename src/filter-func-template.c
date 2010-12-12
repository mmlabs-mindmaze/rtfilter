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
/** \internal
 * \file filter-func-template.c
 * \brief Templates for filter functions
 * \author Nicolas Bourdaud
 *
 * This file includes the template of the different filter functions
 */

#undef NELEM_DAT
#undef NELEM_DATIN
#undef RATIO_INOUT
#define NELEM_DAT	(sizeof(TYPEOUT_LOCAL)/sizeof(TYPEOUT))
#define NELEM_DATIN	(sizeof(TYPEIN_LOCAL)/sizeof(TYPEIN))
#define RATIO_INOUT	((int)(NELEM_DATIN/NELEM_DAT))

static void FILTER_DATADEP_FUNC(hfilter filt,
                                const TYPEIN_LOCAL * restrict in,
                                TYPEOUT_LOCAL * restrict out,
				unsigned int nsamples)
{
	unsigned int i, p;
	int k, ic, ii, len, midlen;
	const TYPEIN_LOCAL *x;
	const TYPEOUT_LOCAL *y;

	int a_len = filt->a_len;
	const TYPEOUT *restrict a = filt->a;
	int b_len = filt->b_len;
	const TYPEOUT *restrict b = filt->b;
	int nch = filt->num_chann / NELEM_DAT;
	int nchin = nch/RATIO_INOUT;
	const TYPEIN_LOCAL *restrict xprev = (TYPEIN_LOCAL*)(filt->xoff) + (a_len - 1) * (nch/RATIO_INOUT);
	const TYPEOUT_LOCAL *restrict yprev = (TYPEOUT_LOCAL*)(filt->yoff) + b_len * nch;
	TYPEOUT_LOCAL h, *s, *odest;
	const TYPEOUT_LOCAL *osrc;
	TYPEIN_LOCAL *idest;
	const TYPEIN_LOCAL *isrc;

	if (!nch)
		return;

	// compute the product of convolution of the input with the infinite
	// impulse response (iir)
	for (i = 0; i < nsamples; i++) {
		s = out + i*nch;

		// Init to convolution to 0
		for (ic=0; ic<nch; ic++)
			s[ic] = zero_dat();

		// Compute the convolution with numerator
		for (k=0; k < a_len; k++) {
			ii = (i - k) * nchin;
			h = set1_dat(a[k]);

			// If the convolution must be done with samples not
			// provided, use the stored ones
			x = (ii >= 0) ? in : xprev;

			for (ic=0; ic < nchin; ic++) 
				for (p=0; p<RATIO_INOUT; p++) {
					int ind = RATIO_INOUT*ic+p;
					s[ind] = add_dat(mul_in_dat(h,x[ii+ic],p),s[ind]);
				}
		}

		// compute the convolution in the denominator
		for (k = 0; k < b_len; k++) {
			ii = (i - k - 1) * nch;
			h = set1_dat(b[k]);

			// If the convolution must be done with samples not
			// provided, use the stored ones
			y = (ii >= 0) ? out : yprev;

			for (ic = 0; ic < nch; ic++)
				s[ic] = add_dat(mul_dat(h,y[ii+ic]),s[ic]);
		}
	}

	// Store the latest input samples
	idest = (TYPEIN_LOCAL*)(filt->xoff);
	len = (a_len-1)*nchin;
	midlen = (a_len-1-nsamples)*nchin;
	if (midlen > 0) {
		isrc = idest + nsamples*nchin;
		memmove(idest, isrc, midlen*sizeof(*isrc));
		idest += midlen;
		len -= midlen;
		isrc = in;
	} else
		isrc = in-midlen;
	memcpy(idest, isrc, len*sizeof(*isrc));
	
	
	// Store the latest output samples
	odest = (TYPEOUT_LOCAL*)(filt->yoff);
	len = b_len*nch;
	midlen = (b_len-nsamples)*nch;
	if (midlen > 0) {
		osrc = odest + nsamples*nch;
		memmove(odest, osrc, midlen*sizeof(*osrc));
		odest += midlen;
		len -= midlen;
		osrc = out;
	} else
		osrc = out-midlen;
	memcpy(odest, osrc, len*sizeof(*osrc));

}
