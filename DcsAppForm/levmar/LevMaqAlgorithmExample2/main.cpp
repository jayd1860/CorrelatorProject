
// This code uses the a Levenberg-Marquardt algorithm C implementation to calculate 
// blood flow index, beta and error from a measurement acquired by a DCS device. 

// The Levenberg-Marquardt code was written by Ron Babich, May 2008 and is distributed 
// freely on github under a GNU licence. Please read the license agreement file LICENSE
// contained in this project.

// The code that uses to run blood flow was written by Jay Dubb. 


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "main.h"

#define N_PARAMS 2

extern int readCorrDataSimple(const char* fname, double* t, double* gmean, int* nch, int* nbins);


double g2func(double *par, int x, void *fdata)
{
    BloodFlowIndex* p = (BloodFlowIndex*)fdata;
    return p->g2func(par, x, p->tau);
}



void gradient(double *g, double *par, int x, void *fdata)
{
    BloodFlowIndex* p = (BloodFlowIndex*)fdata;
    p->gradient(g, par, x, p->tau);
}



int main(int argc, char *argv[])
{
    int    n_iterations;
	double tau[500];
    double g2data[500];
	int    nch; 
	int    nbins; 
	double BFi;
	double beta;
	double err;
	// char*  fname = argv[1];
	char*  fname = ".\\CollectData_subj3.bin.corr";

    memset(tau, 0, sizeof(tau));
	memset(g2data, 0, sizeof(g2data));

    if(readCorrDataSimple(fname, tau, g2data, &nch, &nbins) < 0)
		goto Error;

    // Initialize all the input parameters that go into calculating BFI
	BloodFlowIndex* bfiobj = new BloodFlowIndex(nbins);

	printf("\n");

	bfiobj->bruteForce(g2data, tau, nbins);

	BFi = bfiobj->getBFi();
	beta = bfiobj->getBeta();
	err = bfiobj->getErr();
	n_iterations = bfiobj->getNumIterations();

	printf("**************** End of calculation brute force method ***********************\n");
	printf("N iterations: %d\n", n_iterations);
	printf("BFi: %0.3g, beta: %0.3g, err: %0.3f\n\n", BFi, beta, err);

	
	bfiobj->levmar(g2data, tau, nbins);

	BFi = bfiobj->getBFi();
	beta = bfiobj->getBeta();
	err = bfiobj->getErr();
	n_iterations = bfiobj->getNumIterations();

	printf("**************** End of calculation levmar method ***********************\n");
	printf("N iterations: %d\n", n_iterations);
	printf("BFi: %0.3g, beta: %0.3g, err: %0.3f\n\n", BFi, beta, err);


Error:

	getchar();
	return 0;
}


