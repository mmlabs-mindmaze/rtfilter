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
#define NELEM_DAT	(sizeof(TYPEOUT_LOCAL)/sizeof(TYPEOUT))

static void FILTER_DATADEP_FUNC(hfilter filt, const TYPEIN_LOCAL *in, TYPEOUT_LOCAL *out, unsigned int nsamples)
{
	unsigned int i;
	int k, ichann, ii, len, midlen;
	const TYPEIN_LOCAL *x;
	const TYPEOUT_LOCAL *y;

	int a_len = filt->a_len;
	const TYPEOUT *a = filt->a;
	int b_len = filt->b_len;
	const TYPEOUT *b = filt->b;
	int nchann = filt->num_chann / NELEM_DAT;
	const TYPEIN_LOCAL *xprev = (TYPEIN_LOCAL*)(filt->xoff) + (a_len - 1) * nchann;
	const TYPEOUT_LOCAL *yprev = (TYPEOUT_LOCAL*)(filt->yoff) + b_len * nchann;
	TYPEOUT_LOCAL coef, *currout, *odest;
	const TYPEOUT_LOCAL *osrc;
	TYPEIN_LOCAL *idest;
	const TYPEIN_LOCAL *isrc;

	if (!nchann)
		return;



	// compute the product of convolution of the input with the infinite
	// impulse response (iir)
	for (i = 0; i < nsamples; i++) {
		currout = out + i*nchann;

		// Init to convolution to 0
		for (ichann=0; ichann<nchann; ichann++)
			currout[ichann] = zero_dat();


		// Compute the convolution with numerator
		for (k = 0; k < a_len; k++) {
			ii = (i - k) * nchann;
			coef = set1_dat(a[k]);

			// If the convolution must be done with samples not
			// provided, use the stored ones
			x = (ii >= 0) ? in : xprev;

			for (ichann = 0; ichann < nchann; ichann++)
				currout[ichann] = add_dat(mul_in_dat(coef,x[ii+ichann]),currout[ichann]);
		}

		// compute the convolution in the denominator
		for (k = 0; k < b_len; k++) {
			ii = (i - k - 1) * nchann;
			coef = set1_dat(b[k]);

			// If the convolution must be done with samples not
			// provided, use the stored ones
			y = (ii >= 0) ? out : yprev;

			for (ichann = 0; ichann < nchann; ichann++)
				currout[ichann] = add_dat(mul_dat(coef,y[ii+ichann]),currout[ichann]);
		}
	}

	// Store the latest input samples
	idest = (TYPEIN_LOCAL*)(filt->xoff);
	len = (a_len-1)*nchann;
	midlen = (a_len-1-nsamples)*nchann;
	if (midlen > 0) {
		isrc = idest + nsamples*nchann;
		memmove(idest, isrc, midlen*sizeof(*isrc));
		idest += midlen;
		len -= midlen;
		isrc = in;
	} else
		isrc = in-midlen;
	memcpy(idest, isrc, len*sizeof(*isrc));
	
	
	// Store the latest output samples
	odest = (TYPEOUT_LOCAL*)(filt->yoff);
	len = b_len*nchann;
	midlen = (b_len-nsamples)*nchann;
	if (midlen > 0) {
		osrc = odest + nsamples*nchann;
		memmove(odest, osrc, midlen*sizeof(*osrc));
		odest += midlen;
		len -= midlen;
		osrc = out;
	} else
		osrc = out-midlen;
	memcpy(odest, osrc, len*sizeof(*osrc));

}
