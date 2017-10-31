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
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "rtfilter.h"


#define NCHANN	1
#define NSAMPLE	64
#define NITER 100
#define FILTORDER 4
#define FC_DEF	0.2
#define TYPE_DEF	RTF_FLOAT

char line[128], command[1024];
const char infilename[] = "dsfilein.bin";
const char outfilename[] = "dsfileout.bin";

static void* align_alloc(size_t alignment, size_t size)
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


static void  align_free(void* memptr)
{
#if HAVE_POSIX_MEMALIGN
	free(memptr);
#else
	free(*(((void**)memptr)-1));
#endif
}


static size_t sizeof_data(int type)
{
	size_t dsize;
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


#if HAVE_MATLAB
static int compare_results(double thres)
{
	FILE *pipe;
	double errval = 1.0e10;
	int rval = 1;
	

	sprintf(command, "matlab -nojvm -nodisplay -nosplash -nodesktop -r "
	                 "\"addpath('%s');checkdown('%s','%s');exit;\"",
			 getenv("srcdir"), infilename, outfilename);
	pipe = popen(command, "r");
	while (fgets(line, 127, pipe)) {
		if (sscanf(line, " Error value = %lg", &errval) == 1) {
			rval = 0;
			break;
		}
	}
	pclose(pipe);
	if (rval)
		return rval;

	fprintf(stdout, "\t\tError value = %lg (thresehold = %lg)\n",
	                errval, thres);

	rval = (errval < thres) ? 0 : 1;
	return rval;
}
#endif


static void set_signals(unsigned int nchann, unsigned int nsample, unsigned int intype, void* buff)
{
	static unsigned int cs = 0;
	unsigned int i, j;
	
	if (intype & RTF_DOUBLE) {
		double* data = buff;
		for (i = 0; i < nchann; i++)
			for (j = 0; j < nsample; j++)
				data[j * nchann + i] = ((double)(i+1))
				     *sin((double)((j+cs))/(double)nsample)
				     +cos(5.6*(double)((j+cs))/(double)nsample);
	} else if (intype== RTF_FLOAT) {
		float* data = buff;
		for (i = 0; i < nchann; i++)
			for (j = 0; j < nsample; j++)
				data[j * nchann + i] = ((float)(i+1))
				      *sinf((float)((j+cs))/(float)nsample)
				     +cosf(5.6*(float)((j+cs))/(float)nsample);
	} else if (intype & RTF_CDOUBLE) {
		complex double* data = buff;
		for (i = 0; i < nchann; i++)
			for (j = 0; j < nsample; j++)
				data[j * nchann + i] = ((double)(i+1))
				     *sin((double)((j+cs))/(double)nsample)
				     +cos(5.6*(double)((j+cs))/(double)nsample);
	} else if (intype == RTF_CFLOAT) {
		complex float* data = buff;
		for (i = 0; i < nchann; i++)
			for (j = 0; j < nsample; j++)
				data[j * nchann + i] = ((float)(i+1))
				      *sinf((float)((j+cs))/(float)nsample)
				     +cosf(5.6*(float)((j+cs))/(float)nsample);
	}
	cs += nsample;
}


int main(int argc, char *argv[])
{
	int nchann, nsample, niter;
	int k, opt, ns;
	hfilter filt = NULL;
	void *buffin, *buffout;
	int filein = -1, fileout = -1;
	size_t samsize;
	uint32_t nchan32;
	uint32_t type;
	uint32_t r = 4;
	int retval = 1;
	int keepfiles = 0;


	// Process command-line options
	nchann = NCHANN;
	nsample = NSAMPLE;
	niter = NITER;
	type = TYPE_DEF;
	while ((opt = getopt(argc, argv, "hc:s:i:t:r:k:")) != -1) {
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
		case 'r':
			r = atoi(optarg);
			break;
		case 't':
			type = atoi(optarg);
			break;
		case 'k':
			keepfiles = atoi(optarg);
			break;
		case 'h':
		default:	/* '?' */
			fprintf(stderr, "Usage: %s [-c numchannel] [-s numsample] [-i numiteration] "
					"[-t (0 for float/1 for double)]\n",
				argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	printf("\tnumber of channels: %i \t\tlength of batch: %i\n", nchann, nsample);
	samsize = sizeof_data(type) * nchann;
	nchan32 = nchann;

	// Allocate buffers
	buffin = align_alloc(16, nsample*samsize);
	buffout = align_alloc(16, nsample*samsize);
	if (!buffin || !buffout) {
		fprintf(stderr, "buffer allocation failed\n");
		goto out;
	}


	// Open files for writing
	filein = open(infilename, O_WRONLY|O_CREAT, S_IRWXU);
	fileout = open(outfilename, O_WRONLY|O_CREAT, S_IRWXU);
	if ((filein < 0) || (fileout < 0)) {
		fprintf(stderr, "File opening failed\n");
		goto out;
	}

	// write filter params on fileout
	if (write(filein, &type, sizeof(type)) == -1 ||
	    write(filein, &nchan32, sizeof(nchan32)) == -1 ||
	    write(fileout, &type, sizeof(type)) == -1 ||
	    write(fileout, &nchan32, sizeof(nchan32)) == -1  ||
	    write(fileout, &r, sizeof(r)) == -1)
		goto out;



	// create filters
	filt = rtf_create_downsampler(nchann, type, r);
	if (!filt) {
		fprintf(stderr,"Creation of filter failed\n");
		goto out;
	}

	// Filter chunks of data and write input and output on files
	for (k=0; k<niter; k++) {
		set_signals(nchann, nsample, type, buffin);
		ns = rtf_filter(filt, buffin, buffout, nsample);
		if ( write(filein, buffin, nsample*samsize) == -1
		     || write(fileout, buffout, ns*samsize) == -1 ) {
		        fprintf(stderr,"Error while writing file\n");
			break;
		}
	}
	retval = 0;

out:
	rtf_destroy_filter(filt);
	align_free(buffin);
	align_free(buffout);
	if (filein != -1)
		close(filein);
	if (fileout != -1)
		close(fileout);

#if HAVE_MATLAB
	if (retval == 0)
		retval = compare_results(0.02);
#endif
	if (!keepfiles) {
		unlink(infilename);
		unlink(outfilename);
	}

	return retval;
}
