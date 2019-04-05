/*
 *  Copyright (C) 2010-2011 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>
 *
 *  The program is free software: you can redistribute it and/or
 *  modify it under the terms of the version 3 of the GNU General
 *  Public License as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* This program presents a very simple example of the use of the rtfilter
 * library. It simply creates a multichannel signal. Each channel contains a
 * ramp of the same length with an amplitude specific to each channel.
 *
 * The input data in created and processd by chunk of CHUNKNS samples. The
 * input and output data are then recorded on 2 files: input.bin and
 * output.bin.
 */
#include <rtfilter.h>
#include <stdlib.h>
#include <stdio.h>

#define NCH 8
#define FS 512                  /* in Hz */
#define CUTOFF 8                /* in Hz */
#define CHUNKNS 16
#define RAMPDUR 0.1             /* in seconds */
#define DURATION 10             /* in seconds */
#define NPOLES 2

/* Create a ramp signal in each channel */
static
void create_input(float* data, unsigned int ns, unsigned int cs)
{
	unsigned int j, i, ramplen;

	ramplen = (unsigned int)(RAMPDUR*FS);

	for (j = 0; j < NCH; j++) {
		for (i = 0; i < ns; i++) {
			data[i*NCH+j] = (j+1)*((cs+i) % ramplen);
		}
	}
}


int main(int argc, char* argv[])
{
	(void)argc;     /* Disable warning about unused variable */
	(void)argv;     /* Disable warning about unused variable */

	float input[CHUNKNS * NCH], output[CHUNKNS * NCH];
	double normfc;
	unsigned int i;
	hfilter filt = NULL;
	FILE *fin = NULL, *fout = NULL;
	size_t input_ns, output_ns;
	int retcode = EXIT_FAILURE;

	/* Create the butterworth filter */
	normfc = (double) CUTOFF / (double) FS;
	filt = rtf_create_butterworth(NCH, RTF_FLOAT, normfc, NPOLES, 0);
	if (filt == NULL)
		goto exit;

	/* Open the streams for writing input and output signals */
	fin = fopen("input.bin", "wb");
	fout = fopen("output.bin", "wb");
	if (fin == NULL || fout == NULL)
		goto exit;

	/* Loop over sample */
	for (i = 0; i < DURATION * FS; i += CHUNKNS) {
		/* prepare the input chunk */
		create_input(input, CHUNKNS, i);

		/* Filter the data */
		rtf_filter(filt, input, output, CHUNKNS);

		/* Write the input and output on files */
		input_ns = fwrite(input, sizeof(float) * NCH, CHUNKNS, fin);
		output_ns = fwrite(output, sizeof(float) * NCH, CHUNKNS, fout);
		if (input_ns < CHUNKNS || output_ns < CHUNKNS)
			goto exit;
	}

	retcode = EXIT_SUCCESS;

exit:
	fclose(fin);
	fclose(fout);
	rtf_destroy_filter(filt);
	return retcode;
}
