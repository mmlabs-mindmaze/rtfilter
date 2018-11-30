/*
    Copyright (C) 2008-2011 Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

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
#include <limits.h>
#include <float.h>
#include <unistd.h>
#include <stdint.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "rtfilter.h"


#define NCHANN	64
#define NSAMPLE	6
#define NITER 100
#define FILTORDER 4
#define FC_DEF	0.2
#define TYPE_DEF	RTF_FLOAT

char line[128], command[1024];
char infilename[] = "filein.bin.";
char outfilename[] = "fileout.bin.";

double numd[5] = {7.397301684767964e-04, 2.958920673907186e-03, 4.438381010860778e-03, 2.958920673907186e-03, 7.397301684767964e-04, };
double denumd[5] = {1.000000000000000e+00, -3.152761111579526e+00, 3.860934099037117e+00, -2.161019850256067e+00, 4.647120978065145e-01, };
float numf[5] = {7.397301684767964e-04, 2.958920673907186e-03, 4.438381010860778e-03, 2.958920673907186e-03, 7.397301684767964e-04, };
float denumf[5] = {1.000000000000000e+00, -3.152761111579526e+00, 3.860934099037117e+00, -2.161019850256067e+00, 4.647120978065145e-01, };
//complex double numcd[5] = {7.397301684767964e-04, 2.958920673907186e-03+1.0*I, 4.438381010860778e-03, 2.958920673907186e-03, 7.397301684767964e-04, };
//complex double denumcd[5] = {1.000000000000000e+00, -3.152761111579526e+00, 3.860934099037117e+00, -2.161019850256067e+00, 4.647120978065145e-01, };
complex float numcf[5] = {
	3.1231e-04 - 2.8627e-04*I,
        6.9001e-04 - 4.9178e-04*I,
	3.7251e-04 - 2.0181e-04*I
};
complex float denumcf[5] = {
	9.9248e-01 + 1.2241e-01*I,
	-1.9058e+00 - 4.7739e-01*I,
	9.0091e-01 + 3.4752e-01*I
};
complex double numcd[5] = {
	3.1231e-04 - 2.8627e-04*I,
        6.9001e-04 - 4.9178e-04*I,
	3.7251e-04 - 2.0181e-04*I
};
complex double denumcd[5] = {
	9.9248e-01 + 1.2241e-01*I,
	-1.9058e+00 - 4.7739e-01*I,
	9.0091e-01 + 3.4752e-01*I
};

uint32_t numlen = sizeof(numd)/sizeof(numd[0]);
uint32_t denumlen = sizeof(denumd)/sizeof(numd[0]);
int32_t ptype = RTF_DOUBLE;
void *num = numd, *denum = denumd;

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


static void set_param(int ptype)
{
	if (ptype == RTF_FLOAT) {
		num = numf;
		denum = denumf;
	} else if (ptype == RTF_DOUBLE) {
		num = numd;
		denum = denumd;
	}else if (ptype == RTF_CFLOAT) {
		num = numcf;
		denum = denumcf;
	} else if (ptype == RTF_CDOUBLE) {
		num = numcd;
		denum = denumcd;
	}
}

static size_t sizeof_data(int type)
{
	switch (type) {
		case RTF_FLOAT:
			return sizeof(float);
		case RTF_DOUBLE:
			return sizeof(double);
		case RTF_CFLOAT:
			return sizeof(complex float);
		case RTF_CDOUBLE:
			return sizeof(complex double);
		default:
			return 0;
	}
}


#if ENABLE_MATLAB_TESTS
#if HAVE_MATLAB
#define MPROG_CMD       "matlab -nojvm -nodisplay -nosplash -nodesktop -r "
#elif HAVE_OCTAVE
#define MPROG_CMD       "octave --no-gui --eval "
#endif

static int compare_results(double thres)
{
	FILE *pipe;
	double errval = DBL_MAX;
	double matdt = 0.0;
	int rval = 1;

	sprintf(command, MPROG_CMD"\"addpath('%s');checkfiltres('%s','%s');exit;\"", getenv("srcdir"), infilename, outfilename);
	pipe = popen(command, "r");
	while (fgets(line, 127, pipe)) {
		if (sscanf(line, " time per sample: %lg ns", &matdt) == 1)
			continue;
		if (sscanf(line, " Error value = %lg", &errval) == 1) {
			rval = 0;
			break;
		}
	}
	pclose(pipe);
	if (rval)
		return rval;

	fprintf(stderr, "\t\tError value = %lg (thresehold = %lg)\n"
	                "\t\tMatlab time process per sample: %lg ns\n",
			errval, thres, matdt);

	

	rval = (errval < thres) ? 0 : 1;
	return rval;
}
#endif

static void set_signals(unsigned int nchann, unsigned int nsample, unsigned int intype, void* buff)
{
	unsigned int i, j;
	
	if (intype == RTF_DOUBLE) {
		double* data = buff;
		for (i = 0; i < nchann; i++)
			for (j = 0; j < nsample; j++)
				data[j * nchann + i] = sin(6.28/(double)(i+1)*(double)j);//(float)j - ((float)(nsample-1))/2.0f;
	} else if (intype == RTF_FLOAT) {
		float* data = buff;
		for (i = 0; i < nchann; i++)
			for (j = 0; j < nsample; j++)
				data[j * nchann + i] = sin(6.28/(double)(i+1)*(double)j);//(float)j - ((float)(nsample-1))/2.0f;
	} else if (intype == RTF_CDOUBLE) {
		complex double* data = buff;
		for (i = 0; i < nchann; i++)
			for (j = 0; j < nsample; j++)
				data[j * nchann + i] = (1.0+1.0*I)*sin(6.28/(double)(i+1)*(double)j);//(float)j - ((float)(nsample-1))/2.0f;
	} else if (intype == RTF_CFLOAT) {
		complex float* data = buff;
		for (i = 0; i < nchann; i++)
			for (j = 0; j < nsample; j++)
				data[j * nchann + i] = (1.0+1.0*I)*sin(6.28/(double)(i+1)*(double)j);//(float)j - ((float)(nsample-1))/2.0f;
	}
}


int main(int argc, char *argv[])
{
	int nchann, nsample, niter, filtorder;
	int k, opt;
	hfilter filt = NULL;
	void *buffin, *buffout;
	int filein = -1, fileout = -1;
	float fc = FC_DEF;
	size_t buffinsize, buffoutsize;
	uint32_t nchan32, ntotsample32;
	int32_t datintype, datouttype;
	int retval = 1;
	int keepfiles = 0;


	// Process command-line options
	nchann = NCHANN;
	nsample = NSAMPLE;
	niter = NITER;
	filtorder = FILTORDER;
	datouttype = datintype = TYPE_DEF;
	while ((opt = getopt(argc, argv, "hc:s:i:o:f:d:p:k:")) != -1) {
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
			datintype = atoi(optarg);
			break;
		case 'p':
			ptype = atoi(optarg);
			break;
		case 'k':
			keepfiles = atoi(optarg);
			break;
		case 'h':
		default:	/* '?' */
			fprintf(stderr, "Usage: %s [-c numchannel] [-s numsample] [-i numiteration] "
					"[-o filterorder] [-f cutoff_freq] [-d (0 for float/1 for double)]\n",
				argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	printf("\tfilter order: %i \tnumber of channels: %i \t\tlength of batch: %i\n",filtorder, nchann, nsample);
	set_param(ptype);
	datouttype = datintype;
	if (ptype & RTF_COMPLEX_MASK)
		datouttype |= RTF_COMPLEX_MASK;

	buffinsize = sizeof_data(datintype) * nchann * nsample;
	buffoutsize = sizeof_data(datouttype) * nchann * nsample;
	nchan32 = nchann;
	ntotsample32 = nsample*niter;

	// Allocate buffers
	buffin = align_alloc(16, buffinsize);
	buffout = align_alloc(16, buffoutsize);
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

	fprintf(stdout, "datin=%i datout=%i ptype=%i\n", datintype, datouttype, ptype );
	// write filter params on fileout
	if (write(fileout, &ptype, sizeof(ptype)) == -1 ||
	    write(fileout, &numlen, sizeof(numlen)) == -1 ||
	    write(fileout, num, numlen*sizeof_data(ptype)) == -1 ||
	    write(fileout, &denumlen, sizeof(denumlen)) == -1 ||
	    write(fileout, denum, denumlen*sizeof_data(ptype)) == -1 ||
	    write(filein, &datintype, sizeof(datintype)) == -1 ||
	    write(filein, &nchan32, sizeof(nchan32)) == -1 ||
	    write(filein, &ntotsample32, sizeof(ntotsample32)) == -1 ||
	    write(fileout, &datouttype, sizeof(datouttype)) == -1 ||
	    write(fileout, &nchan32, sizeof(nchan32)) == -1 ||
	    write(fileout, &ntotsample32, sizeof(ntotsample32)) == -1)
		goto out;



	// set signals (ramps)
	set_signals(nchann, nsample, datintype, buffin);

	// create filters
	filt = rtf_create_filter(nchann, datintype, numlen, num, denumlen, denum, ptype);
	if (!filt) {
		fprintf(stderr,"Creation of filter failed\n");
		goto out;
	}

	// Test the type
	if (datintype != rtf_get_type(filt, 1)
	  || datouttype != rtf_get_type(filt, 0)) {
		fprintf(stderr, "Unexpected data type\n"
		                "expected: in=%i out=%i\n"
				"returned: in=%i out=%i\n",
				datintype, datouttype,
				rtf_get_type(filt, 1),
				rtf_get_type(filt, 0));
		goto out;
	}

	// Filter chunks of data and write input and output on files
	for (k=0; k<niter; k++) {
		rtf_filter(filt, buffin, buffout, nsample);
		if ( write(filein, buffin, buffinsize) == -1
		     || write(fileout, buffout, buffoutsize) == -1 ) {
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

#if ENABLE_MATLAB_TESTS
	if (retval == 0)
		retval = compare_results( (datintype & RTF_PRECISION_MASK) ? 1e-12 : 1e-4);
#endif
	if (!keepfiles) {
		unlink(infilename);
		unlink(outfilename);
	}

	return retval;
}
