
#include <stdio.h>
#include <string.h>

#pragma warning (disable : 4996)


// --------------------------------------------------------------------------------------------
int readCorrDataSimple(const char* fname, double* ydata, double* t, int N)
{
	double* input;
	int count = 0;
	bool breakflag=false;
	int r;
	double f;


	FILE* fid = fopen(fname, "r");
	if (fid == NULL) {
		fprintf(stdout,"ERROR: Could not open file %s. Exiting.\n",fname);
		return -1;
	}

	for(int ii=0;  ii<N; ii++) {
		r = fscanf(fid, "%lf %lf", &t[ii], &ydata[ii]);
		if(r==EOF)
			break;
	}

	fclose(fid);
	return 0;
}





