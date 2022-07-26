#pragma once

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "levmarq.h"

#define PI 3.1415926535897
#define N_PARAMS 2



/////////////////////////////////////////////////////////////////////////
class BloodFlowIndex
{

public:

    // -------------------------------------------------------------
	BloodFlowIndex(int nbins)
    {
		// External input params to BloodFlowIndex; for now we're setting these to default
        mua    = .1;
        musp   = 6;
        rho    = 2;
        lamda  = 850e-7;
        n      = 1.37;
        alpha  = 1;

		// Calculate intermediate common variables
		R      = -1.440/(n*n) + 0.710 / n + 0.668 + 0.0636*n;
		z0     = 1.0/(mua + musp);
		zb     = 2.0/3.0 * (1.0 + R) / (1.0 - R) / musp;
		r1     = sqrt(rho*rho + z0*z0);
		r2     = sqrt(rho*rho + (z0 + 2.0 * zb)*(z0 + 2.0 * zb));
		k0     = 2.0 * PI*n / lamda;
		c2     = 3*mua*musp;
		c3     = k0*k0*6*alpha*musp*musp;
		G1_0   = exp(-r1*sqrt(c2))/r1 - exp(-r2*sqrt(c2))/r2;

		// Number of beginning taus to discard
		nDiscard = 40;

		// Init parameters for 2 different algorithms
		levmarq_init(&lmstat);
		brutForce_init(nbins);
	}



	/////////////// Brute force functions /////////////////////////

	// -------------------------------------------------------------
	void brutForce_init(int nbins)
	{
		// Parameters for brute force method of blood flow calculation
        nBFi  = 100;
        nbeta = 100;
        double BFi_min = 5e-11;
        double BFi_max = 3e-8;
        double beta_min = 0;
        double beta_max = .5;
        double BFi_0     = 1e-8;
        double beta_0    = 0.45;
        double rangeBFi  = 9e-9;
        double rangeBeta = 0.1;
        double incrBFi = (BFi_max - BFi_min) / (double)nBFi;
        double incrbeta = (beta_max - beta_min) / (double)nbeta;
        int    ii;

		BFi_try = new double[nBFi+10];
		beta_try = new double[nbeta+10];
		memset(BFi_try, 0, nBFi*sizeof(double));
		memset(beta_try, 0, nbeta*sizeof(double));

        // Initialize BFi array of guesses to try
		BFi_try[0] = BFi_min;
        ii=1;
        while(ii<nBFi)
        {	
			BFi_try[ii] = BFi_try[ii-1] + incrBFi;
			if(BFi_try[ii] > BFi_max)
                break;
            ii=ii+1;
        }

        // Initialize beta array of guesses to try
		beta_try[0] = beta_min;
        ii=1;
        while(ii<nbeta)
        {
			beta_try[ii] = beta_try[ii-1] + incrbeta;
			if(beta_try[ii] > beta_max)
                break;
            ii=ii+1;
        }
		g1     = new double[nbins];
		g2pred = new double[nbins];
	}
	



	// -------------------------------------------------------------------
	inline void G2(double BFi, double beta, double* tau, int nbins)
	{
		double par[2];

		par[0] = BFi;
		par[1] = beta;

		for(int ii=nDiscard; ii<nbins; ii++)
			g2pred[ii] = g2func(par, ii, tau);
    }
 


    // -------------------------------------------------------------
    inline void bruteForce(double* g2data, double* tau, int nbins)
    {   
        double err_curr;
        int N = nBFi*nbeta;
        int ii, jj;
		int idx;
		int idx1 = 0;
		int idx2 = 0;

        err = 9999;
        for(idx=0; idx < N; idx++)
        {
            ii = idx / nbeta;
            jj = idx % nbeta;

            G2(BFi_try[ii], beta_try[jj], tau, nbins);
            err_curr = 0;
            for(int kk=nDiscard; kk<nbins; kk++)
                err_curr = err_curr + (g2pred[kk] - g2data[kk]) * (g2pred[kk] - g2data[kk]);

            if (err_curr < err)
            {
                idx1 = ii;
                idx2 = jj;
                err = err_curr;
            }
        }

		BFi = BFi_try[idx1];
		beta = beta_try[idx2];

        err_curr=0;
		n_iterations = idx;
    }


	/////////////// Common function used by both bruteForce and levmar /////////////////////////

    // -------------------------------------------------------------------
	inline double g2func(double *par, int x, void *fdata)
	{
		double BFi = par[0];
		double B = par[1];
		double* tau = (double*)fdata;
		double  K;

		if(BFi > 0)
			K = sqrt(c2 + BFi*c3*tau[x]);
		else
			K = sqrt(c2);

		return(1 + B * pow(((exp(-r1*K) / r1 - exp(-r2*K) / r2) / G1_0), 2));
	}


	/////////////// Levenberg-Marquardt functions /////////////////////////

	// --------------------------------------------------------------------------------------
	inline void gradient(double *g, double *par, int x, void *fdata)
	{
		double* tau = (double*)fdata;
		double  BFi = par[0];
		double  beta = par[1];
		double  K;

		if(BFi > 0)
			K = sqrt(c2 + BFi*c3*tau[x]);
		else
			K = sqrt(c2);

		g[0] = -(2 * beta*(exp(-r1*K) / r1 - exp(-r2*K) / r2) * ((c3*tau[x] * exp(-r1*K)) / (2 * K) - (c3*tau[x] * exp(-r2*K)) / (2 * K))) / (G1_0*G1_0);
		g[1] = pow((exp(-r1*K) / r1 - exp(-r2*K) / r2), 2) / (G1_0*G1_0);
	}


	// -----------------------------------------------------------------------------
	inline static double g2funcwrap(double *par, int x, void *fdata)
	{
		BloodFlowIndex* p = (BloodFlowIndex*)fdata;
		return p->g2func(par, x, p->tau);
	}



	// -----------------------------------------------------------------------------
	inline static void gradientwrap(double *g, double *par, int x, void *fdata)
	{
		BloodFlowIndex* p = (BloodFlowIndex*)fdata;
		p->gradient(g, par, x, p->tau);
	}


	// -------------------------------------------------------------
	inline void levmar(double* g2data, double* t, int nbins)
	{   
		// Initial guess for parameter values
		double params[N_PARAMS] = {1e-8, .45};

		tau = &t[nDiscard];
		n_iterations = levmarq(N_PARAMS,
							   params,
							   nbins - nDiscard,
							   &g2data[nDiscard],
							   NULL,
							   g2funcwrap,
							   gradientwrap,
							   this,
							   &lmstat);

		BFi  = params[0];
		beta = params[1];
		err  = lmstat.final_err;
	}



	/////////////// generic common get/set functions /////////////////////////


	// -----------------------------------------------------------------------------
    double getBFi()
    {
		return BFi;
    }

	// -----------------------------------------------------------------------------
    double getBeta()
    {
		return beta;
    }

	// -----------------------------------------------------------------------------
    double getErr()
    {
        return err;
    }

	// -----------------------------------------------------------------------------
	double getNumIterations()
	{
		return n_iterations;
	}


public:

	// Common params
    double* tau;

    double  mua;
    double  musp;
    double  rho;
    double  R;
    double  z0;
    double  zb;
    double  r1;
    double  r2;
    double  k0;
    double  c2;
    double  c3;
    double  G1_0;
    double  lamda;
    double  n;
    double  alpha;

	double  BFi;
	double  beta;
	double  err;
	int     nDiscard;
	int     n_iterations;

	// Brute force params
	double* BFi_try;
	double* beta_try;
    int     nBFi;
    int     nbeta;
    double* g1;
    double* g2pred;
    
	// Levenberg-Marquardt params
	LMstat  lmstat;

};

