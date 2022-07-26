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
		int    n_iterations;

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




//
// This is a code port from matlab code written by Kuan-Cheng (Tony) Wu 
//  MGH May 2016
//
//  with ideas from: 
//  * Emmanuel Schaub, "High countrate real-time FCS using F2Cor," Opt. Express 21, 23543-23555 (2013)
//  * Davide Magatti and Fabio Ferri, "Fast multi-tau real-time software correlator for dynamic light scattering," Appl. Opt. 40, 4011-4021 (2001)
//  * Bernhard Zimmermann
// 
// This C++ port was written by Jay Dubb, MGH Sept, 2016
//


/////////////////////////////////////////////////////////////////////////
class CorrFunc
{
public:

	// -----------------------------------------------------------------------------
	CorrFunc(int num)
	{
		nCh = num;
		elimi = 1;
		samplef = 1.5e8;
		maxlag = 6e5;
		double P = 20;
		double q = 2;
		int ncorr = ceil(log((double)maxlag / P) / log(q)) + 1;
		nbins = P + P / q*(ncorr - 2);

		maxphotcount = 8e4;

		g = new double[maxlag+1];
		t = new double[maxlag+1];

		for(int ii=0; ii<maxlag+1; ii++)
			t[ii] = (double)ii * 1 / samplef;

		t_binedge = new double[nbins+1];
		memset(t_binedge, 0, sizeof(double)*(nbins+1));

		i_binedge = new double[nbins+1];
		gbinsum = new double[nbins];
		t_binmean = new double[nbins];

		gmean = new double*[nCh];
		for(int ii=0; ii<nCh; ii++)
		{
			gmean[ii] = new double[nbins];
			memset(gmean[ii], 0, sizeof(double)*nbins);
		}

		tmean = new double[nbins];
		memset(tmean, 0, sizeof(double)*nbins);

		mylogbin2_precalc();

		BFi_obj = new BloodFlowIndex(nbins);
	}



	// -----------------------------------------------------------------------------
	inline int isnan(double x) 
	{ 
		return x != x; 
	}


	// -----------------------------------------------------------------------------
	inline int isinf(double x) 
	{ 
		return !isnan(x) && isnan(x - x); 
	}


	// -----------------------------------------------------------------------------
	void calcAutocorrCurve(unsigned long long* y, int size, int iCh)
	{
		if(size<1)
			return;

		if(size>maxphotcount)
			size = maxphotcount;

		memset(gmean[iCh], 0, sizeof(double)*nbins);

		my_pulse_xcorr_zero(y, y, size, size);
		mylogbin2(iCh);
	}


	// -----------------------------------------------------------------------------
	void calcBloodFlowIndex(int iCh, double* BFi, double* beta, double* err)
	{
		//BFi_obj->bruteForce(gmean[iCh], tmean, nbins);
		BFi_obj->levmar(gmean[iCh], tmean, nbins);
		*BFi = BFi_obj->getBFi();
		*beta = BFi_obj->getBeta();
		*err = BFi_obj->getErr();
	}



	// -----------------------------------------------------------------------------
	inline void my_pulse_xcorr_zero(unsigned long long* y, unsigned long long* y_shift, int n1, int n2)
	{
		memset((void*)g, 0, sizeof(double) * maxlag+1);
		double c1 = ((double)(y[n1-1] - y[0])) / ((double)n1 * (double)n2);
		int k;

		if(y == nullptr)
			return;

		// Calculate the correlation function 
		for(int ii=0; ii < n1; ii++)
		{
			for(int jj=ii+1; jj < n2-16; jj=jj+16)
			{
				k = y_shift[jj] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+1] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+2] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+3] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+4] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+5] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+6] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+7] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+8] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+9] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+10] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+11] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+12] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+13] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+14] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;

				k = y_shift[jj+15] - y[ii];
				if((k <= maxlag) && (k > 0))
					g[k] = g[k] + 1;
				else
					break;
			}
		}

		// Multiply the corr function by c1 
		for(int ii=0; ii<maxlag-16; ii=ii+16)
		{
			g[ii] = c1 * g[ii];
			g[ii+1] = c1 * g[ii+1];
			g[ii+2] = c1 * g[ii+2];
			g[ii+3] = c1 * g[ii+3];
			g[ii+4] = c1 * g[ii+4];
			g[ii+5] = c1 * g[ii+5];
			g[ii+6] = c1 * g[ii+6];
			g[ii+7] = c1 * g[ii+7];
			g[ii+8] = c1 * g[ii+8];
			g[ii+9] = c1 * g[ii+9];
			g[ii+10] = c1 * g[ii+10];
			g[ii+11] = c1 * g[ii+11];
			g[ii+12] = c1 * g[ii+12];
			g[ii+13] = c1 * g[ii+13];
			g[ii+14] = c1 * g[ii+14];
			g[ii+15] = c1 * g[ii+15];
		}
	}



	// --------------------------------------------------------------------------
	// This is a porting of the matlab function logspace
	void mylogspace(double d1, double d2, int n, double* y)
	{
		int n1 = n - 1;

		double c = (d2 - d1)*(n1 - 1);
		if(isinf(c))
			for(int ii=0; ii<n1; ii++)
				y[ii] = pow(10.0, d1 + (d2 / n1)*ii - (d1 / n1)*ii);
		else
			for(int ii=0; ii<n1; ii++)
				y[ii] = pow(10.0, (d1 + ii*(d2 - d1) / n1));

		if(y != nullptr)
		{
			y[0] = pow(10.0,d1);
			y[n-1] = pow(10.0,d2);
		}
	}



	// --------------------------------------------------------------------------
	void mylogbin2_precalc()
	{
		double  binedge_min;
		double 	i_binedge_min;
		int jj;

		//% precalculate:  Determine k intevals
		mylogspace(log10(t[1]), log10(t[maxlag-1]), nbins+1, t_binedge);

		//% precalculate: This is taking mean of k interval for new delay values
		//% tmean = (t_binedge(2:end) + t_binedge(1:end - 1)) / 2;
		for(int ii=0; ii < nbins; ii++)
			tmean[ii] = (t_binedge[ii+1] + t_binedge[ii]) / 2;

		//% precalculate: This for loop is instead of rounding of k edges.
		memset(i_binedge, 0, sizeof(double) * (nbins+1));
		for(int ii=0; ii < nbins+1; ii++)
		{
			//% [~, i_binedge(ii)] = min(abs(t - t_binedge(ii)));
			binedge_min = fabs(t[1] - t_binedge[ii]);
			i_binedge_min = 0;
			for(jj=1; jj<maxlag+1; jj++)
			{
				if(fabs(t[jj] - t_binedge[ii]) < binedge_min)
				{
					binedge_min = fabs(t[jj] - t_binedge[ii]);
					i_binedge_min = jj;
				}
			}
			i_binedge[ii] = i_binedge_min;
		}

		memset((void*)t_binmean, 0, sizeof(double) * nbins);
		for(int ii=0; ii < nbins; ii++)
			t_binmean[ii] = i_binedge[ii+1] - i_binedge[ii] + 1;
	}



	// --------------------------------------------------------------------------
	void mylogbin2(int iCh)
	{
		//% Calculating g mean.Can precalculate devisor
		//% (i_binedge(ii + 1) - i_binedge(ii) + 1))
		memset((void*)gbinsum, 0, sizeof(double) * nbins);
		for(int ii=0; ii < nbins; ii++)
		{
			//% gmean(ii) = sum(g(i_binedge(ii) :i_binedge(ii + 1)) / (i_binedge(ii + 1) - i_binedge(ii) + 1));
			for(int jj = i_binedge[ii]; jj <= i_binedge[ii+1]; jj++)
				gbinsum[ii] = gbinsum[ii] + g[jj];
			gmean[iCh][ii] = gbinsum[ii] / t_binmean[ii];
		}
	}


public:

	double* g;
	double* t;
	int elimi;
	int samplef;
	// maxlag in tick units. To convert to seconds multiply by samplef 
	// (ie, number of ticks per second).
	int maxlag;    

	int nbins; 
	double* i_binedge;
	double* t_binedge;
	double* t_binmean;
	double* gbinsum;
	double** gmean;
	double* tmean;
	int maxphotcount;

	int nCh;

	BloodFlowIndex*  BFi_obj;
};



