
#include <stdio.h>
#include <string.h>
#include "readCorrData.h"

#pragma warning (disable : 4996)

// --------------------------------------------------------------------
int getCorrDataSize(FILE* fid, double** tmean, int* nch, int* nbins, int* ndpts)
{
	int tt;
	double* input;
	int fpos = 0;
	int count;
	bool breakflag = false;


	fread(nbins,sizeof(int),1,fid);
	fread(nch,sizeof(int),1,fid);

	fpos = sizeof(*nbins) + sizeof(*nch);

	*tmean = new double[*nbins];
	fread(*tmean,sizeof(double),*nbins,fid);
	input = new double[*nbins];

	fpos = fpos + (sizeof(double) * *nbins);

	tt = 0;
	while(1) {
		for (int jj=0; jj < *nch; jj++) {
			count = fread(input, sizeof(double), *nbins, fid);
			if (count == 0) {
				breakflag = true;
				break;
			}
		}
		if(breakflag)
			break;
		tt++;
	}

	*ndpts = tt;

	return fpos;
}


// --------------------------------------------------------------------------------------------
int readCorrData(const char* fname, double** t, double**** g, int* nch, int* nbins, int* ndpts)
{
	struct corrData d;
	double* input;
	int fposdata;
	int count = 0;
	bool breakflag=false;

	FILE* fid = fopen(fname,"rb");
	if (fid == NULL) {
		fprintf(stdout,"ERROR: Could not open file %s. Exiting.\n",fname);
		return -1;
	}

	fposdata = getCorrDataSize(fid, t, nch, nbins, ndpts);
	fseek(fid, fposdata+1, SEEK_SET);

	(*g) = new double**[*nch];

	for (int ii=0; ii < *nch; ii++) {		
		(*g)[ii] = new double*[*ndpts];
		for (int tt=0; tt < *ndpts; tt++) {
			(*g)[ii][tt] = new double[*nbins];
			count = fread((*g)[ii][tt], sizeof(double), *nbins, fid);
			if (count == 0) {
				breakflag = true;
				break;
			}
		}
		if(breakflag)
			break;
	}
	fclose(fid);
	return 0;
}





