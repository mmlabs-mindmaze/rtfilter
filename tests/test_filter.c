#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <memory.h>
#include <limits.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#include "common-filters.h"

#define NCHANN	64
#define NSAMPLE	6
#define NITER 100
#define FILTORDER 4
#define FC_DEF	0.2


// fc = 0.100 (wn=0.100 in MATLAB)
float num[5] = {7.397301684767964e-04, 2.958920673907186e-03, 4.438381010860778e-03, 2.958920673907186e-03, 7.397301684767964e-04, };
float denum[5] = {1.000000000000000e+00, -3.152761111579526e+00, 3.860934099037117e+00, -2.161019850256067e+00, 4.647120978065145e-01, };

int main(int argc, char *argv[])
{
	int nchann, nsample, niter, filtorder;
	int i, j, k, opt;
	hfilter filt = NULL;
	float *buffin, *buffout;
	FILE *filein = NULL, *fileout = NULL;
	float fc = FC_DEF;
	size_t buffsize;
	uint32_t nchan32;


	// Process command-line options
	nchann = NCHANN;
	nsample = NSAMPLE;
	niter = NITER;
	filtorder = FILTORDER;
	while ((opt = getopt(argc, argv, "hc:s:i:o:f:")) != -1) {
		switch (opt) {
		case 'c':
			nchann = atoi(optarg);
			break;
		case 's':
			nsample = atoi(optarg);
			break;
		case 'i':
			niter = atoi(optarg);
			break;
		case 'o':
			filtorder = atoi(optarg);
			break;
		case 'f':
			fc = atof(optarg);
			break;
		case 'h':
		default:	/* '?' */
			fprintf(stderr, "Usage: %s [-c numchannel] [-s numsample] [-i numiteration] [-o filterorder] [-f cutoff_freq]\n",
				argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	printf("filter order: %i \tnumber of channels: %i \t\tlength of batch: %i\n",filtorder, nchann, nsample);

	buffsize = sizeof(*buffin) * nchann * nsample;
	nchan32 = nchann;

	// Allocate buffers
	if (posix_memalign((void **) &buffin, 16, buffsize))
		buffin = NULL;
	if (posix_memalign((void **) &buffout, 16, buffsize))
		buffout = NULL;
	if (!buffin || !buffout) {
		fprintf(stderr, "buffer allocation failed\n");
		goto out;
	}


	// Open files for writing
	filein = fopen("filein.bin","w");
	fileout = fopen("fileout.bin","w");
	if (!filein || !fileout) {
		fprintf(stderr, "File opening failed\n");
		goto out;
	}
	fwrite(&nchan32, sizeof(nchan32), 1, filein);
	fwrite(&nchan32, sizeof(nchan32), 1, fileout);

	// set signals (ramps)
	for (i = 0; i < nchann; i++)
		for (j = 0; j < nsample; j++)
			buffin[j * nchann + i] = sin(6.28/(double)(i+1)*(double)j);//(float)j - ((float)(nsample-1))/2.0f;

	// create filters
//	filt = create_filter(nchann, DATATYPE_FLOAT, sizeof(num)/sizeof(num[0]), num, sizeof(denum)/sizeof(denum[0]), denum, DATATYPE_FLOAT);
//	filt = create_butterworth_filter(fc, filtorder, nchann, 0, DATATYPE_FLOAT);
	filt = create_chebychev_filter(fc, filtorder, nchann, 0, 0.005, DATATYPE_FLOAT);
//	filt = create_integrate_filter(nchann, DATATYPE_FLOAT);
	if (!filt) {
		fprintf(stderr,"Creation of filter failed\n");
		goto out;
	}

	for (k=0; k<niter; k++) {
		filter_f(filt, buffin, buffout, nsample);
		if ( !fwrite(buffin, buffsize, 1, filein)
		     || !fwrite(buffout, buffsize, 1, fileout) ) {
		        fprintf(stderr,"Error while writing file\n");
			break;
		}
	}


out:
	destroy_filter(filt);
	free(buffin);
	free(buffout);
	if (filein)
		fclose(filein);
	if (fileout)
		fclose(fileout);

	return 0;
}
