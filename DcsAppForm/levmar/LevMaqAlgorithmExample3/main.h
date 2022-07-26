#pragma once

#include <stdio.h>
#include <string.h>

#define PI 3.1415926535897



/////////////////////////////////////////////////////////////////////////
class BloodFlowIndex
{

public:

    // -------------------------------------------------------------
    BloodFlowIndex(double* time)
    {
		b = 6;
		d = .5;
		e = 8;
		t = time;
    }

    inline double g2(double* par, int x, void* fdata);
    inline void BloodFlowIndex::gradient(double *g, double *par, int x, void *fdata);

public:

    double b;
    double d;
    double e;
	double* t;
};

