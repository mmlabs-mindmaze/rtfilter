/*
    Copyright (C) 2010-2011 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    The program is free software: you can redistribute it and/or
    modify it under the terms of the version 3 of the GNU General
    Public License as published by the Free Software Foundation.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <complex.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "rtfilter.h"

#define NCHANN	64
#define NSAMPLE	4
#define NITER 10000
#define FILTORDER 4

static
void* align_alloc(size_t alignment, size_t size)
{
#if HAVE_POSIX_MEMALIGN
	void* memptr = NULL;
	if (posix_memalign(&memptr, alignment, size))
		return NULL;
	return memptr;
#else
	void* origptr = malloc(sizeof(void*) + alignment + size);
	char* ptr = ((char*)origptr) + sizeof(void*);
	ptr += alignment - ((uintptr_t)ptr)%alignment;
	*(void**)(ptr-sizeof(origptr)) = origptr;
	return ptr;
#endif
}

static
void  align_free(void* memptr)
{
#if HAVE_POSIX_MEMALIGN
	free(memptr);
#else
	free(*(((void**)memptr)-1));
#endif
}


static
size_t sizeof_data(int type)
{
	size_t dsize = 0;
	if (type == RTF_FLOAT)
		dsize = sizeof(float);
	else if (type == RTF_DOUBLE)
		dsize = sizeof(double);
	else if (type == RTF_CFLOAT)
		dsize = sizeof(complex float);
	else if (type == RTF_CDOUBLE)
		dsize = sizeof(complex double);

	return dsize;
}

static
void set_coeffs(unsigned int len, int type, void* coef,
                void** num, void** den)
{
	memset(coef, 0, sizeof_data(type)*2*len);
	*num = coef;
	if (type == RTF_FLOAT) {
		((float*)coef)[len] = 1.0;
		*den = ((float*)coef) + len;
	} else if (type == RTF_DOUBLE) {
		((double*)coef)[len] = 1.0;
		*den = ((double*)coef) + len;
	} else if (type == RTF_CFLOAT) {
		((complex float*)coef)[len] = 1.0;
		*den = ((complex float*)coef) + len;
	} else if (type == RTF_CDOUBLE) {
		((complex double*)coef)[len] = 1.0;
		*den = ((complex double*)coef) + len;
	}
}

static
void print_version(void)
{
	unsigned int line = 0;
	char version[128];

	while (rtf_get_version(version, sizeof(version), line))
		printf((line++ ? "       %s\n" : "version: %s\n"), version);
}


int main(int argc, char *argv[])
{
	int nch, ns, niter, filtorder;
	int k, opt, datintype, datouttype, ptype, vect;
	struct timespec start, stop;
	long long delay = 0, delayv = 0;
	long long tc, dt, timing, mintime, mintimev;
	hfilter filt = NULL;
	void *buff1, *buff2, *origbuff, *coef;
	void *num = NULL, *den = NULL;

	// Process command-line options
	nch = NCHANN;
	ns = NSAMPLE;
	niter = NITER;
	filtorder = FILTORDER;
	vect = 1;
	ptype = datintype = RTF_FLOAT;

	while ((opt = getopt(argc, argv, "c:s:i:o:d:p:v:h")) != -1) {
		switch (opt) {
		case 'c':
			nch = atoi(optarg);
			break;
		case 's':
			ns = atoi(optarg);
			break;
		case 'i':
			niter = atoi(optarg);
			break;
		case 'o':
			filtorder = atoi(optarg);
			break;
		case 'd':
			datintype = atoi(optarg);
			break;
		case 'p':
			ptype = atoi(optarg);
			break;
		case 'v':
			vect = atoi(optarg);
			break;
		case 'h':
		default:	/* '?' */
			fprintf(stderr, "Usage: %s [-c numchannel] [-s numsample] [-i numiteration] [-o filterorder]\n",
				argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	
	print_version();
	printf("filter order: %i \tnumber of channels: %i \t\tlength of batch: %i\n",filtorder, nch, ns);

	datouttype = datintype;
	if (ptype & RTF_COMPLEX_MASK)
		datouttype |= RTF_COMPLEX_MASK;


	// Allocate buffers
	origbuff = align_alloc(16, sizeof_data(datintype)*nch*ns + 8);
	buff2 = align_alloc(16, sizeof_data(datouttype)*nch*ns);
	coef = malloc(2*sizeof_data(ptype)*filtorder);
	if (!origbuff || !buff2 || ! coef) {
		fprintf(stderr, "buffer allocation failed\n");
		goto out;
	}
	buff1 = (vect == 0) ? (char*)origbuff + 8 : origbuff;
	// set signals (zeros)
	memset(buff1, 0, nch*ns*sizeof_data(datintype));	
	set_coeffs(filtorder, ptype, coef, &num, &den);


	// Estimate timecall of clockgettime
	tc = LONG_MAX;
	for (k=0; k<1000; k++) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		clock_gettime(CLOCK_MONOTONIC, &stop);
		delay = ((stop.tv_sec - start.tv_sec)*1000000000 + (stop.tv_nsec - start.tv_nsec));
		tc = delay >= tc ? tc : delay;
	}

	// create filters
	filt = rtf_create_filter(nch, datintype, 
	                         filtorder, num,
				 filtorder, den, ptype);
	if (!filt) {
		fprintf(stderr,"Creation of filter failed\n");
		goto out;
	}

	delay = delayv = 0;
	mintime = mintimev = LONG_MAX;
	for (k=0; k<niter; k++) {
		// Test normal version
		clock_gettime(CLOCK_REALTIME, &start);
		rtf_filter(filt, buff1, buff2, ns);
		clock_gettime(CLOCK_REALTIME, &stop);
		timing = ((stop.tv_sec - start.tv_sec)*1000000000 + (stop.tv_nsec - start.tv_nsec)) - tc;
		delay += timing;
		mintime = mintime > timing ? timing : mintime;
	}

	dt = mintime;
	printf("min time per call: %i nsec\n",(int)dt);
	printf("min time per sample: %lg nsec\n",(double)dt/(double)(ns*nch));
out:
	rtf_destroy_filter(filt);
	align_free(origbuff);
	align_free(buff2);
	free(coef);

	return 0;
}
