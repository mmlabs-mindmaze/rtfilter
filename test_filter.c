#include "common-filters.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <memory.h>
#include <xmmintrin.h>
#include <limits.h>
#include <unistd.h>

#define NCHANN	64
#define NSAMPLE	4
#define NITER 10000
#define FILTORDER 4


void filter_u(hfilter filt, const typereal* x, typereal* y, unsigned int num_samples);
void filter_a(hfilter filt, const typereal* x, typereal* y, unsigned int num_samples);

int main(int argc, char *argv[])
{
	int nchann, nsample, niter, filtorder;
	int i, j, k, opt;
	struct timespec start, stop;
	long long delay = 0, delayv = 0;
	long long tc, dt, timing, mintime, mintimev;
	hfilter filt1 = NULL, filt2 = NULL;
	typereal *buff1, *buff2;


	// Process command-line options
	nchann = NCHANN;
	nsample = NSAMPLE;
	niter = NITER;
	filtorder = FILTORDER;
	while ((opt = getopt(argc, argv, "c:s:i:o:")) != -1) {
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
		case 'h':
		default:	/* '?' */
			fprintf(stderr, "Usage: %s [-c numchannel] [-s numsample] [-i numiteration] [-o filterorder]\n",
				argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	printf("filter order: %i \tnumber of channels: %i \t\tlength of batch: %i\n",filtorder, nchann, nsample);



	// Allocate buffers
	if (posix_memalign
	    ((void **) &buff1, 16, sizeof(typereal) * nchann * nsample))
		buff1 = NULL;
	if (posix_memalign
	    ((void **) &buff2, 16, sizeof(typereal) * nchann * nsample))
		buff2 = NULL;
	if (!buff1 || !buff2) {
		fprintf(stderr, "buffer allocation failed\n");
		goto out;
	}
	// set signals (ramps)
	for (i = 0; i < nchann; i++)
		for (j = 0; j < nsample; j++)
			buff1[j * nchann + i] = j;


	// Estimate timecall of clockgettime
	tc = LONG_MAX;
	for (k=0; k<1000; k++) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		clock_gettime(CLOCK_MONOTONIC, &stop);
		delay = ((stop.tv_sec - start.tv_sec)*1000000000 + (stop.tv_nsec - start.tv_nsec));
		tc = delay >= tc ? tc : delay;
	}

	// create filters
	filt1 = create_butterworth_filter(0.02, filtorder, nchann, 0);
	filt2 = create_butterworth_filter(0.02, filtorder, nchann, 0);
	if (!filt1 || !filt2) {
		fprintf(stderr,"Creation of filters failed (filt1:%i filt2:%i)\n",
			filt1 ? 1 : 0,
			filt2 ? 1 : 0);
		goto out;
	}

	delay = delayv = 0;
	mintime = mintimev = LONG_MAX;
	for (k=0; k<niter; k++) {
		// Test normal version
		clock_gettime(CLOCK_MONOTONIC, &start);
		filter(filt1, buff1, buff2, nsample);
		clock_gettime(CLOCK_MONOTONIC, &stop);
		timing = ((stop.tv_sec - start.tv_sec)*1000000000 + (stop.tv_nsec - start.tv_nsec)) - tc;
		delay += timing;
		mintime = mintime > timing ? timing : mintime;

		// Test vectorized version
		clock_gettime(CLOCK_MONOTONIC, &start);
		filter_u(filt2, buff1, buff2, nsample);
		clock_gettime(CLOCK_MONOTONIC, &stop);
		timing = ((stop.tv_sec - start.tv_sec)*1000000000 + (stop.tv_nsec - start.tv_nsec)) - tc;
		delayv += timing;
		mintimev = mintimev > timing ? timing : mintimev;
	}

	printf("normal version:\n");
	dt = delay / niter;
//	printf("mean time per call: %i nsec\n",(int)dt);
//	printf("mean time per sample: %i nsec\n",(int)(dt/(nsample*nchann)));
	dt = mintime;
	printf("min time per call: %i nsec\n",(int)dt);
	printf("min time per sample: %i nsec\n",(int)(dt/(nsample*nchann)));
	printf("vector version:\n");
	dt = delayv / niter;
//	printf("mean time per call: %i nsec\n",(int)dt);
//	printf("mean time per sample: %i nsec\n",(int)(dt/(nsample*nchann)));
	dt = mintimev;
	printf("min time per call: %i nsec\n",(int)dt);
	printf("min time per sample: %i nsec\n",(int)(dt/(nsample*nchann)));

out:
	destroy_filter(filt1);
	destroy_filter(filt2);
	free(buff1);
	free(buff2);

	return 0;
}
