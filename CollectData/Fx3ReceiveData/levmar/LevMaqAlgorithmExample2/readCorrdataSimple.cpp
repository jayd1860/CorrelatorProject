
#include <stdio.h>
#include <string.h>

#pragma warning (disable : 4996)


// --------------------------------------------------------------------------------------------
int readCorrDataSimple(const char* fname, double* t, double* g, int* nch, int* nbins)
{
	double* input;
	int count = 0;
	bool breakflag=false;
	double t_0[200];
	double g_0[200];

	FILE* fid = fopen(fname, "rb");
	if (fid == NULL) {
		fprintf(stdout,"ERROR: Could not open file %s. Exiting.\n",fname);
		return -1;
	}

	fread(nbins, sizeof(int), 1, fid);
	fread(nch, sizeof(int), 1, fid);
	fread(t_0, sizeof(double), *nbins, fid);
	count = fread(g_0, sizeof(double), *nbins, fid);
	fclose(fid);

	memcpy(g, g_0, *nbins * sizeof(double));
	memcpy(t, t_0, *nbins * sizeof(double));

	return 0;
}





