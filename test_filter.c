#include "common-filters.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <memory.h>
#include <xmmintrin.h>
#include <limits.h>
#include <unistd.h>
#include <stdint.h>

#define NCHANN	64
#define NSAMPLE	6
#define NITER 100
#define FILTORDER 4
#define FC_DEF	0.2

// fc = 0.010 (wn=0.005 in MATLAB)
//float num[5] = {3.728051659046550e-09, 1.491220663618620e-08, 2.236830995427930e-08, 1.491220663618620e-08, 3.728051659046550e-09, };
//float denum[4] = {3.958953318647084e+00, -5.877700273536147e+00, 3.878530549051736e+00, -9.597836538114997e-01, };

// fc = 0.020 (wn=0.010 in MATLAB)
//float num[5] = {5.845142428406724e-08, 2.338056971362690e-07, 3.507085457044035e-07, 2.338056971362690e-07, 5.845142428406724e-08, };
//float denum[4] = {3.917907865391989e+00, -5.757076379118071e+00, 3.760349507694531e+00, -9.211819291912375e-01, };

// fc = 0.050 (wn=0.025 in MATLAB)
//float num[5] = {2.150568737285807e-06, 8.602274949143229e-06, 1.290341242371484e-05, 8.602274949143229e-06, 2.150568737285807e-06, };
//float denum[4] = {3.794791103079410e+00, -5.405166861726178e+00, 3.424747347274252e+00, -8.144059977272810e-01, };


// fc = 0.100 (wn=0.050 in MATLAB)
//float num[5] = {3.123897691706401e-05, 1.249559076682560e-04, 1.874338615023841e-04, 1.249559076682560e-04, 3.123897691706401e-05, };
//float denum[4] = {3.589733887112176e+00, -4.851275882519419e+00, 2.924052656162461e+00, -6.630104843858918e-01, };

// fc = 0.200 (wn=0.100 in MATLAB)
float num[5] = {4.165992044065786e-04, 1.666396817626314e-03, 2.499595226439472e-03, 1.666396817626314e-03, 4.165992044065786e-04, };
float denum[4] = {3.180638548874721e+00, -3.861194348994217e+00, 2.112155355110971e+00, -4.382651422619805e-01, };

// fc = 0.500 (wn=0.250 in MATLAB)
//float num[5] = {1.020948079120312e-02, 4.083792316481250e-02, 6.125688474721874e-02, 4.083792316481250e-02, 1.020948079120312e-02, };
//float denum[4] = {1.968427786938517e+00, -1.735860709208885e+00, 7.244708295073621e-01, -1.203895998962444e-01, };

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
			fprintf(stderr, "Usage: %s [-c numchannel] [-s numsample] [-i numiteration] [-o filterorder]\n",
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
			buffin[j * nchann + i] = (float)j - ((float)(nsample-1))/2.0f;

	// create filters
//	filt = create_filter_f(nchann, sizeof(num)/sizeof(num[0]), num, sizeof(denum)/sizeof(denum[0]), denum, DATATYPE_FLOAT);
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
