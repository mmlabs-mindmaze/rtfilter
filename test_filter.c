#include "filter.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define NCHANN	64
#define NSAMPLE	4
#define NITER 10000


int main(int argc, char* argv[])
{
	int i, j, k;
	struct timespec start, stop;
	struct timeval startv, stopv;
	long long delay = 0, delayv = 0;
	long long dt;
	dfilter* filt;
	float buff1[NCHANN*NSAMPLE], buff2[NCHANN*NSAMPLE];
	volatile float dummy;

	// set signals
	for (i=0; i<NCHANN; i++)
		for (j=0; j<NSAMPLE; j++)
			buff1[j*NCHANN+i] = j;


	filt = create_butterworth_filter(0.02, 4, NCHANN, 0);
	for (k=0; k<NITER; k++) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		filter(filt, buff1, buff2, NSAMPLE);
		clock_gettime(CLOCK_MONOTONIC, &stop);
		delay += ((stop.tv_sec - start.tv_sec)*1000000000 + (stop.tv_nsec - start.tv_nsec));
		//gettimeofday(&startv, NULL);
		//gettimeofday(&stopv, NULL);
		//delayv += ((stopv.tv_sec - startv.tv_sec)*1000000000 + 100*(stopv.tv_usec - startv.tv_usec));
		clock_gettime(CLOCK_MONOTONIC, &start);
		clock_gettime(CLOCK_MONOTONIC, &stop);
		delay -= ((stop.tv_sec - start.tv_sec)*1000000000 + (stop.tv_nsec - start.tv_nsec));
		for (i=0; i<NCHANN*NSAMPLE; i++) {
			buff1[i] = dummy = buff2[i];
		}
	}

	printf("clock_gettime:\n");
	dt = delay / NITER;
	printf("mean time per call: %i nsec\n",(int)dt);
	printf("mean time per sample: %i nsec\n",(int)(dt/(NSAMPLE*NCHANN)));

/*	printf("gettimeofday\n");
	dt = delayv / NITER;
	printf("mean time per call: %i nsec\n",(int)dt);
	printf("mean time per sample: %i nsec\n",(int)(dt/(NSAMPLE*NCHANN)));
*/

	return 0;
}
