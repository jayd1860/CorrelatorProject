#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "levmarq.h"
#include "main.h"
#include "readCorrData.h"

#define N_PARAMS 2

double params[N_PARAMS] = {0, 0}; // Initial values of parameters

LMstat lmstat;

#define PI 3.1415926535897

extern int readCorrDataSimple(const char* fname, double* ydata, double* t, int N);


/* @brief   Function, describes Newton law of heating/cooling
 *
 * @usage   par[0] - temperature of heater,
 *          par[1] - initial temperature of water,
 *          par[2] - heat transmission coefficient
 *
 * @par     input of Newton Law:
 * @x       samplenumber
 * @fdata   additional data, not used
 *
 * @return  temperature at the time x
 */

double BloodFlowIndex::g2(double *par, int x, void *fdata)
{
    double  a = par[0];
    double  c = par[1];
	double* t = (double*)fdata;

	return a*pow(t[x], 4) - b*pow(t[x],3) + c*pow(t[x],2) - d*t[x] + e;
}


double g2func(double *par, int x, void *fdata)
{
    BloodFlowIndex* p = (BloodFlowIndex*)fdata;
    return p->g2(par, x, p->t);
}



/*
 * @brief   Gradient function for Newton law of heating
 */
void BloodFlowIndex::gradient(double *g, double *par, int x, void *fdata)
{
    double  a = par[0];
    double  c = par[1];
	double* t = (double*)fdata;

    g[0] = 	pow(t[x],4);
    g[1] = 	pow(t[x],2);
}


void gradientfunc(double *g, double *par, int x, void *fdata)
{
    BloodFlowIndex* p = (BloodFlowIndex*)fdata;

    p->gradient(g, par, x, p->t);
}



int main(int argc, char *argv[])
{
    int n_iterations;
    levmarq_init(&lmstat);
	double t[201];
    double ydata[201];
	char* fname = "C:\\jdubb\\workspaces\\correlatorproject\\Matlab_proj\\bloodFlowIndxFitting\\ydata.txt";
	int N = sizeof(t)/sizeof(double);


    memset(t, 0, sizeof(t));
	memset(ydata, 0, sizeof(ydata));

    readCorrDataSimple(fname, ydata, t, N);
	BloodFlowIndex* bfiobj = new BloodFlowIndex(t);

	printf("\n");

	n_iterations = levmarq(N_PARAMS,
						   params,
						   N,
						   ydata,
						   NULL,
						   g2func,
						   gradientfunc,
						   bfiobj,
						   &lmstat);

	printf("**************** End of calculation ***********************\n");
	printf("N iterations: %d\n", n_iterations);
	printf("a: %0.3g, c: %0.3g\n\n", params[0], params[1]);

	getchar();
	return 0;
}
