#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <common-filters.h>
#include <filter.h>
#include <math.h>


unsigned int i,j,nchann,nsample,num_pole;
unsigned int Fr[4];


float *X;
float features[5];
double fl,fh,fs;

complex float *Y;
size_t buffsize,cbuffsize;

hfilter cfilt;

FILE *file;

char path_data[] = "data.bin";
char path_filter[] = "Filt.bin";
char path_features[] = "features.bin";


int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	nchann=2;
	nsample=1024;
	Fr[0]=10;
	Fr[1]=8;
	Fr[2]=20;
	Fr[3]=60;
	

	fs=1024.0;
	fl=8.0;
	fh=12.0;
	num_pole=2;

	features[0]=(float)(num_pole);
	features[1]=(float)(fl);
	features[2]=(float)(fh);
	features[3]=(float)(fs);
	features[4]=(float)(nsample);


	buffsize = sizeof(*X)*nchann*nsample; 
	cbuffsize = sizeof(*Y)*nchann*nsample;


	// Allocate buffers
	X = malloc(buffsize);
	Y = malloc(cbuffsize);

	cfilt=create_bandpass_analytic_filter(nchann, RTF_FLOAT,
					      fl/fs, fh/fs, num_pole);

	if (!X || !Y || !cfilt)
		goto exit;

	// Create Data
	for(i=0;i<nchann;i++){
		for(j=0;j<nsample;j++){
			X[i+j*nchann] = sin((2.0*M_PI*Fr[i]*j/fs))
			                 + sin((2.0*M_PI*Fr[i+2]*j/fs));
		}
	}

	filter(cfilt,X,Y,nsample);


	file= fopen(path_features,"w");
	fwrite(features,sizeof(features[0]),5,file);
	fclose(file);


	file= fopen(path_data,"w");
	fwrite(X,sizeof(X[0]),nsample*nchann,file);
	fclose(file);


	file= fopen(path_filter,"w");
	fwrite(Y,sizeof(Y[0]),nsample*nchann,file);
	fclose(file);

exit:
	free(X);
	free(Y);
	destroy_filter(cfilt);
	return 0;
}
