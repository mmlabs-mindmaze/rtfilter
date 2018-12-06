/*
    Copyright (C) 2018 MindMaze SA
    Nicolas Bourdaud <nicolas.bourdaud@mindmaze.ch>

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


/**
 * INIT_DATADEP_FUNC - template for intialization of filter states
 * @filt:       filter to initialize
 * @data:       pointer to input data to use for initialization (underlying
 *              type must be the fitler input type and the length equal to
 *              the number of channel)
 *
 * This is the template for the initialization function. It computes the
 * internal states of the filter @filt as the steady state response of a
 * step whose amplitude is given by @data. In other words, it sets the
 * states as if @data would have been constantly given as input to the
 * filter since beginning of the times.
 */
LOCAL_FN
void INIT_DATADEP_FUNC(const struct rtf_filter* filt, const void* data)
{
	const TYPEIN* in = data;
	TYPEIN* xoff = filt->xoff;
	TYPEOUT* yoff = filt->yoff;

	const TYPEOUT* a = filt->a;
	const TYPEOUT* b = filt->b;
	int a_len = filt->a_len;
	int b_len = filt->b_len;
	int nch = filt->num_chann;

	int i, k;
	TYPEOUT scale, a_sum, b_sum;

	// Copy initializer to past input buffer
	for (i = 0; i < a_len-1; i++) {
		for (k = 0; k < nch; k++)
			xoff[i*nch + k] = in[k];
	}

	// If no denuminator, nothing else has to be set
	if (b_len == 0)
		return;

	// Compute the scaling factor to apply to output when signal is in
	// steady state
	a_sum = 0.0;
	for (i = 0; i < a_len; i++)
		a_sum += a[i];

	b_sum = 0.0;
	for (i = 0; i < b_len; i++)
		b_sum += b[i];

	scale = a_sum / (1.0 - b_sum);

	// Set past output to scaled input (when in steady state)
	for (i = 0; i < b_len; i++) {
		for (k = 0; k < nch; k++)
			yoff[i*nch + k] = scale * in[k];
	}
}
