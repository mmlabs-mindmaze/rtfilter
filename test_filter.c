#include "common-filters.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <memory.h>
#include <xmmintrin.h>
#include <limits.h>

#define NCHANN	256
#define NSAMPLE	4
#define NITER 10000


int main(int argc, char* argv[])
{
	int i, j, k;
	struct timespec start, stop;
	struct timeval startv, stopv;
	long long delay = 0, delayv = 0;
	long long tc, dt;
	dfilter *filt1, *filt2;
	float buff1[NCHANN*NSAMPLE], buff2[NCHANN*NSAMPLE];
	__m128 vbuff1[NCHANN/4*NSAMPLE], vbuff2[NCHANN/4*NSAMPLE];

	// set signals
	for (i=0; i<NCHANN; i++)
		for (j=0; j<NSAMPLE; j++)
			buff1[j*NCHANN+i] = j;
	memcpy(vbuff1, buff1, sizeof(buff1));


	// Estimate timecall of clockgettime
	tc = LONG_MAX;
	for (k=0; k<100; k++) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		clock_gettime(CLOCK_MONOTONIC, &stop);
		delay = ((stop.tv_sec - start.tv_sec)*1000000000 + (stop.tv_nsec - start.tv_nsec));
		tc = delay >= tc ? tc : delay;
	}


	filt1 = create_butterworth_filter(0.02, 8, NCHANN, 0);
	filt2 = create_butterworth_filter(0.02, 8, NCHANN, 0);
	for (k=0; k<NITER; k++) {
		// Test normal version
		clock_gettime(CLOCK_MONOTONIC, &start);
		filter(filt1, buff1, buff2, NSAMPLE);
		clock_gettime(CLOCK_MONOTONIC, &stop);
		delay += ((stop.tv_sec - start.tv_sec)*1000000000 + (stop.tv_nsec - start.tv_nsec)) - tc;

		// Test vectorized version
		clock_gettime(CLOCK_MONOTONIC, &start);
		filtera(filt2, vbuff1, vbuff2, NSAMPLE);
		clock_gettime(CLOCK_MONOTONIC, &stop);
		delayv += ((stop.tv_sec - start.tv_sec)*1000000000 + (stop.tv_nsec - start.tv_nsec)) - tc;
	}

	printf("normal version:\n");
	dt = delay / NITER;
	printf("mean time per call: %i nsec\n",(int)dt);
	printf("mean time per sample: %i nsec\n",(int)(dt/(NSAMPLE*NCHANN)));
	printf("vector version:\n");
	dt = delayv / NITER;
	printf("mean time per call: %i nsec\n",(int)dt);
	printf("mean time per sample: %i nsec\n",(int)(dt/(NSAMPLE*NCHANN)));

/*	printf("gettimeofday\n");
	dt = delayv / NITER;
	printf("mean time per call: %i nsec\n",(int)dt);
	printf("mean time per sample: %i nsec\n",(int)(dt/(NSAMPLE*NCHANN)));
*/

	return 0;
}
