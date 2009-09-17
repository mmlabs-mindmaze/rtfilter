#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <memory.h>
#include <limits.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include "filter.h"


#define NCHANN	64
#define NSAMPLE	6
#define NITER 100
#define FILTORDER 4
#define FC_DEF	0.2
#define TYPE_DEF	DATATYPE_FLOAT


// fc = 0.100 (wn=0.100 in MATLAB)
float num[5] = {7.397301684767964e-04, 2.958920673907186e-03, 4.438381010860778e-03, 2.958920673907186e-03, 7.397301684767964e-04, };
float denum[5] = {1.000000000000000e+00, -3.152761111579526e+00, 3.860934099037117e+00, -2.161019850256067e+00, 4.647120978065145e-01, };
uint32_t numlen = sizeof(num)/sizeof(num[0]);
uint32_t denumlen = sizeof(denum)/sizeof(num[0]);
uint32_t pdattype = DATATYPE_FLOAT;



static void set_signals(unsigned int nchann, unsigned int nsample, unsigned int dattype, void* buff)
{
	int i, j;
	
	if (dattype == DATATYPE_FLOAT) {
		float* data = buff;
		for (i = 0; i < nchann; i++)
			for (j = 0; j < nsample; j++)
				data[j * nchann + i] = sin(6.28/(double)(i+1)*(double)j);//(float)j - ((float)(nsample-1))/2.0f;
	} else {
		double* data = buff;
		for (i = 0; i < nchann; i++)
			for (j = 0; j < nsample; j++)
				data[j * nchann + i] = sin(6.28/(double)(i+1)*(double)j);//(float)j - ((float)(nsample-1))/2.0f;
	}
}


int main(int argc, char *argv[])
{
	int nchann, nsample, niter, filtorder;
	int i, j, k, opt;
	hfilter filt = NULL;
	void *buffin, *buffout;
	FILE *filein = NULL, *fileout = NULL;
	float fc = FC_DEF;
	size_t buffsize;
	uint32_t nchan32;
	uint32_t dattype;
	int retval = 1;


	// Process command-line options
	nchann = NCHANN;
	nsample = NSAMPLE;
	niter = NITER;
	filtorder = FILTORDER;
	dattype = TYPE_DEF;
	while ((opt = getopt(argc, argv, "hc:s:i:o:f:d:")) != -1) {
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
		case 'd':
			dattype = atoi(optarg);
			break;
		case 'h':
		default:	/* '?' */
			fprintf(stderr, "Usage: %s [-c numchannel] [-s numsample] [-i numiteration] "
					"[-o filterorder] [-f cutoff_freq] [-d (0 for float/1 for double)]\n",
				argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	printf("filter order: %i \tnumber of channels: %i \t\tlength of batch: %i\n",filtorder, nchann, nsample);

	buffsize = sizeof_data(dattype) * nchann * nsample;
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

	// write filter params on fileout
	fwrite(&pdattype, sizeof(dattype), 1, fileout);
	fwrite(&numlen, sizeof(numlen), 1, fileout);
	fwrite(&num, sizeof(num), 1, fileout);
	fwrite(&denumlen, sizeof(denumlen), 1, fileout);
	fwrite(&denum, sizeof(denum), 1, fileout);

	// Write data info on files
	fwrite(&dattype, sizeof(dattype), 1, filein);
	fwrite(&nchan32, sizeof(nchan32), 1, filein);
	fwrite(&dattype, sizeof(dattype), 1, fileout);
	fwrite(&nchan32, sizeof(nchan32), 1, fileout);



	// set signals (ramps)
	set_signals(nchann, nsample, dattype, buffin);

	// create filters
	filt = create_filter(nchann, dattype, numlen, num, denumlen, denum, pdattype);
	if (!filt) {
		fprintf(stderr,"Creation of filter failed\n");
		goto out;
	}

	// Filter chunks of data and write input and output on files
	for (k=0; k<niter; k++) {
		if (dattype == DATATYPE_FLOAT)
			filter_f(filt, buffin, buffout, nsample);
		else
			filter_d(filt, buffin, buffout, nsample);
		if ( !fwrite(buffin, buffsize, 1, filein)
		     || !fwrite(buffout, buffsize, 1, fileout) ) {
		        fprintf(stderr,"Error while writing file\n");
			break;
		}
	}
	retval = 0;

	popen("matlab -nojvm -nodisplay -r checkfiltres", "r");


out:
	destroy_filter(filt);
	free(buffin);
	free(buffout);
	if (filein)
		fclose(filein);
	if (fileout)
		fclose(fileout);

	return retval;
}
