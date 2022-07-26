

#pragma once

#include <string.h>
#include <math.h>


//////////////////////////////////////////////////////////////////
struct RangeType
{
	double x;
	double y;
};


//////////////////////////////////////////////////////////////////
struct MyVector
{
	// --------------------------------------------------------------
	MyVector()
	{
		x = 0.0;
		y = 0.0;
	}

	// --------------------------------------------------------------
	MyVector(double a, double b)
	{
		x = a;
		y = b;
	}

	void Set(double a, double b)
	{
		x = a;
		y = b;
	}

	double x;
	double y;
};


//////////////////////////////////////////////////////////////////
enum Direction
{
	positive = 1,
	negative = 2
};



//////////////////////////////////////////////////////////////////
struct Hist
{
	// --------------------------------------------------------------
	Hist()
	{
		nBins = 20;
		outlierThreshold = 1e-2;
		bins = new int[nBins];
		memset(bins, 0, nBins * sizeof(bins[0]));
	}

	// --------------------------------------------------------------
	void SetRange(double min0, double max0)
	{
		range = max0 - min0;
		rangeElem = range / nBins;
	}


	// --------------------------------------------------------------
	void Calc(MyVector* data, int size, double* min_y, double* max_y)
	{
		double min_y0 = min_y[0];

		// Need to initialize the max and min in each bin to the first 
		// data point (rather than an arbitrary value like zero). 
		for(int kk=1; kk<nBins-1; kk++)
		{
			min_y[kk] = data[0].y;
			max_y[kk] = data[0].y;
		}

		for(int jj=0; jj<size; jj++)
		{
			for(int kk=0; kk<nBins; kk++)
			{
				double binmin = min_y0 + kk*rangeElem;
				double binmax = binmin + rangeElem;

				if((data[jj].y >= binmin) && (data[jj].y <= binmax))
				{
					bins[kk]++;

					// Keep track of max and min of data in each bin
					if(data[jj].y < min_y[kk])
						min_y[kk] = data[jj].y;
					if(data[jj].y > max_y[kk])
						max_y[kk] = data[jj].y;

					break;
				}
				// If plot[jj] didn't fall into any bin, it must be very slightly 
				// greater than the max bin because of rounding error. Check that 
				// this is the case and if it is, drop it into the last bin
				else if((kk == (nBins-1)) && (fabs(data[jj].y-binmax) < 1e-5))
				{
					bins[kk]++;

					// Keep track of max and min of data in each bin
					if(data[jj].y < min_y[kk])
						min_y[kk] = data[jj].y;
					if(data[jj].y > max_y[kk])
						max_y[kk] = data[jj].y;

					break;
				}
			}
		}
	}

	double range;
	double rangeElem;
	int nBins;
	int* bins;
	double outlierThreshold;
};



//////////////////////////////////////////////////////////////////
struct TimeSeries
{

	// --------------------------------------------------------------
	TimeSeries()
	{
		autozoom = true;
	}

	// --------------------------------------------------------------
	void SetAutozoom(bool val)
	{
		autozoom = val;
	}


	// --------------------------------------------------------------
	void Initialize(double rangex)
	{
		data = new MyVector[rangex];
		hist = new Hist();
		imax = hist->nBins-1;
		min_y = new double[imax];
		max_y = new double[imax];

		memset(min_y, 0, sizeof(double)*imax);
		memset(max_y, 0, sizeof(double)*imax);
	}


	// -----------------------------------------------------------
	void ResetStats()
	{
		memset(min_y, 0, imax * sizeof(min_y[0]));
		memset(max_y, 0, imax * sizeof(max_y[0]));
		memset(hist->bins, 0, hist->nBins * sizeof(hist->bins[0]));
	}


	// -----------------------------------------------------------
	void GetNewMinMax(int size)
	{
		// Get new plot min_y
		for(int kk=0; kk<hist->nBins; kk++)
		{
			if(hist->bins[kk] > hist->outlierThreshold)
			{
				min_y[0] = min_y[kk];
				break;
			}
		}

		// Get new plot max_y
		for(int kk=hist->nBins-1; kk>0; kk--)
		{
			if(hist->bins[kk] > hist->outlierThreshold)
			{
				max_y[imax] = max_y[kk];
				break;
			}
		}
	}



	// -----------------------------------------------------------
	void CalcStats(int jj)
	{
		if(jj<1)
		{
			min_y[0] = data[jj].y;
			max_y[imax] = data[jj].y;
		}

		// Get y max and min of current plot
	
		if(data[jj].y < min_y[0])
			min_y[0] = data[jj].y;
		if(data[jj].y > max_y[imax])
			max_y[imax] = data[jj].y;
	}

	MyVector* data;
	double* min_y;
	double* max_y;
	int imax;
	Hist*  hist;
	bool autozoom;
};



//////////////////////////////////////////////////////////////////
struct Markers
{
	// --------------------------------------
	Markers()
	{
		num_x = 5;
		num_y = 6;

		incr.x = 1 / num_x;
		incr.y = 1 / num_y;

		for(int ii=0; ii<num_x; ii++)
		{
			pos_x[ii] = ii*incr.x;
			text_x[ii] = pos_x[ii];
		}
		for(int jj=0; jj<num_y; jj++)
		{
			pos_y[jj] = jj*incr.y;
			text_x[jj] = pos_y[jj];
		}
	}


	// --------------------------------------
	Markers(MyVector min, MyVector max)
	{
		num_x = 5;
		num_y = 6;

		incr.x = (max.x - min.x) / num_x;
		incr.y = (max.y - min.y) / num_y;

		for(int ii=0; ii<num_x; ii++)
		{
			pos_x[ii] = min.x + ii*incr.x;
			text_x[ii] = pow(10.0, pos_x[ii]);
		}
		for(int jj=0; jj<num_y; jj++)
		{
			pos_y[jj] = min.y + jj*incr.y;
			text_y[jj] = pos_y[jj];
		}
	}



	// --------------------------------------
	Markers(MyVector min, MyVector max, double dataRate)
	{
		num_x = 5;
		num_y = 10;

		incr.x = (max.x - min.x) / num_x;
		incr.y = (max.y - min.y) / num_y;

		for(int ii=0; ii<num_x; ii++)
		{
			pos_x[ii] = min.x + ii*incr.x;
			text_x[ii] = pos_x[ii] / dataRate;
		}
		for(int jj=0; jj<num_y; jj++)
		{
			pos_y[jj] = min.y + jj*incr.y;
			text_y[jj] = pos_y[jj];
		}
	}


	// --------------------------------------
	Markers(const Markers& m)
	{
		num_x = m.num_x;
		num_y = m.num_y;
		incr = m.incr;

	}

	// --------------------------------------
	~Markers()
	{
	}


	// ---------------------------------------
	void ResetXY(MyVector min, MyVector max, double dataRate)
	{
		incr.x = (max.x - min.x) / num_x;
		incr.y = (max.y - min.y) / num_y;

		for(int ii=0; ii<num_x; ii++)
		{
			pos_x[ii] = min.x + ii*incr.x;
			text_x[ii] = pos_x[ii] / dataRate;
		}
		for(int jj=0; jj<num_y; jj++)
		{
			pos_y[jj] = min.y + jj*incr.y;
			text_y[jj] = pos_y[jj];
		}
	}


	// ---------------------------------------
	void ResetY(MyVector min, MyVector max)
	{
		incr.y = (max.y - min.y) / num_y;
		for(int jj=0; jj<num_y; jj++)
		{
			pos_y[jj] = min.y + jj*incr.y;
			text_y[jj] = pos_y[jj];
		}
	}



	// ---------------------------------------
	void ResetX(MyVector min, MyVector max)
	{
		incr.x = (max.x - min.x) / num_x;

		for(int ii=0; ii<num_x; ii++)
		{
			pos_x[ii] = min.x + ii*incr.x;
			text_x[ii] = pow(10.0, pos_x[ii]);
		}
	}


    // ---------------------------------------
    void ResetX(MyVector min, MyVector max, double dataRate)
    {
        incr.x = (max.x - min.x) / num_x;

        for(int ii=0; ii<num_x; ii++)
        {
            pos_x[ii] = min.x + ii*incr.x;
            text_x[ii] = pos_x[ii] / dataRate;
        }
    }


	// ---------------------------------------
	int MakeNewX(MyVector* data, int size, double dRate)
	{
		int incr_x = size / num_x;
		for(int ii=0; ii<num_x; ii++)
			text_x[ii] = data[ii*incr_x].x / dRate;

		return 0;
	}


    // ---------------------------------------
    void MakeNewX(double xmin, double xmax, double dataRate, double offset)
    {
        incr.x = (xmax - xmin) / num_x;
        for(int ii=0; ii<num_x; ii++)
        {
            pos_x[ii] = xmin + ii*incr.x;
            text_x[ii] = pos_x[ii] / dataRate + offset;
        }
    }


	int num_x;
	int num_y;
	MyVector incr;
	double pos_x[15];
	double text_x[15];
	double pos_y[15];
	double text_y[15];
};



//////////////////////////////////////////////////////////////////
struct MyCoord
{
	// --------------------------------------
	MyCoord(MyVector o, MyVector m1, MyVector m2, Direction d1, Direction d2, MyVector offs, int nbins)
	{
		curr = o;
		prev = o;
		min = m1;
		max = m2;
		range.x = max.x - min.x;
		range.y = max.y - min.y;
		xd = d1;
		yd = d2;
		offset.x = offs.x;
		offset.y = offs.y;

		markers = new Markers(min, max);

		plot.Initialize(nbins);
	}


	// --------------------------------------
	MyCoord(MyVector o, MyVector m1, MyVector m2, Direction d1, Direction d2, MyVector offs, double dataRate)
	{
		curr = o;
		prev = o;
		min = m1;
		max = m2;
		range.x = max.x - min.x;
		range.y = max.y - min.y;
		xd = d1;
		yd = d2;
		offset.x = offs.x;
		offset.y = offs.y;
		drate = dataRate;

		markers = new Markers(min, max, dataRate);

		plot.Initialize(range.x);
	}



	// --------------------------------------
	MyCoord(const MyCoord& p)
	{
		curr = p.curr;
		prev = p.prev;
		min = p.min;
		max = p.max;
		range = p.range;
		xd = p.xd;
		yd = p.yd;
		offset = p.offset;
		drate = p.drate;

		markers = new Markers(min, max, drate);
		*markers = *p.markers;

		plot = p.plot;
	}


	// -----------------------------------------------------------
	~MyCoord()
	{
		delete markers;
	}


	// -----------------------------------------------------------
    void ResetYMinMax(MyVector m1, MyVector m2)
	{
		min = m1;
		max = m2;
		range.y = max.y - min.y;
		markers->ResetY(min, max);
	}


	// -----------------------------------------------------------
    void ResetYMinMax(double ymin, double ymax)
    {
        min.y = ymin;
        max.y = ymax;
        range.y = max.y - min.y;
        markers->ResetY(min, max);
    }


    // -----------------------------------------------------------
	void ResetXMinMax(MyVector m1, MyVector m2)
	{
		min = m1;
		max = m2;
		range.x = max.x - min.x;
		markers->ResetX(min, max);
	}


	// -----------------------------------------------------------
    void ResetXMinMax(MyVector m1, MyVector m2, double dataRate)
    {
        min = m1;
        max = m2;
        range.x = max.x - min.x;
        markers->ResetX(min, max, dataRate);
    }


    // -----------------------------------------------------------
    void ResetXMinMax(double xmin, double xmax)
    {
        min.x = xmin;
        max.x = xmax;
        range.x = max.x - min.x;
        // markers->ResetX(min, max, offset, dataRate);
    }


    // -----------------------------------------------------------
    void ResetXYMinMax(MyVector m1, MyVector m2)
    {
        min = m1;
        max = m2;

        range.x = max.x - min.x;
        markers->ResetX(min, max);

        range.y = max.y - min.y;
        markers->ResetY(min, max);
    }


    // -----------------------------------------------------------
    void ResetXYMinMax(MyVector m1, MyVector m2, double dataRate)
    {
        min = m1;
        max = m2;

        range.x = max.x - min.x;
        markers->ResetX(min, max, dataRate);

        range.y = max.y - min.y;
        markers->ResetY(min, max);
    }


    // -----------------------------------------------------------
	void ResetAxes(double dRate)
	{
		markers->ResetXY(min, max, dRate);
	}



    // -----------------------------------------------------------
    void SetXMaxMin(int size, double dRate)
    {
        markers->MakeNewX(plot.data, size, dRate);
    }



    // -----------------------------------------------------------
    void SetXMaxMin(double xmin, double xmax, double dRate, double offset)
    {
        markers->MakeNewX(xmin, xmax, dRate, offset);
    }



    // -----------------------------------------------------------
    void SetXMaxMin(double dWidth, int size, double dRate, double offset)
    {
        if(size < dWidth) {
            markers->MakeNewX(0.0, dWidth, dRate, offset);
        }
        else {
            int idx = size-dWidth;
            MyVector* data = &plot.data[idx];
            markers->MakeNewX(data, dWidth, dRate);
        }
    }



    // -----------------------------------------------------------
	bool IsDataOutOfRange(MyVector* min0, MyVector* max0, int size)
	{
		bool flag = false;
		plot.hist->SetRange(plot.min_y[0], plot.max_y[plot.imax]);

		if(size < 1)
			return false;

		if(!plot.autozoom)
			return false;

		plot.hist->Calc(plot.data, size, plot.min_y, plot.max_y);
		plot.GetNewMinMax(size);

		double plotrange = plot.max_y[plot.imax] - plot.min_y[0];
		double r = (plotrange > 0) ? plotrange : 1;
		double padding = r/10;

		if(plot.max_y[plot.imax] > max.y)
		{
			max0->y = plot.max_y[plot.imax] + padding;
			flag = true;
		}

		// Zoom in from the top if axis max is way bigger than plot max
		else if((max.y - plot.max_y[plot.imax]) > 2 * r)
		{
			max0->y = plot.max_y[plot.imax] + padding;
			flag = true;
		}

		// If data is right at the upper border then zoom out a little from the top
		else if(fabs(max.y - plot.max_y[plot.imax]) < padding)
		{
			max0->y = (max.y > plot.max_y[plot.imax]) ? 
				      max.y + padding : 
				      plot.max_y[plot.imax] + padding;
			flag = true;
		}

		if(plot.min_y[0] < min.y)
		{
			min0->y = plot.min_y[0] - padding;
			flag = true;
		}
		// Zoom in from the bottom if axis min is way smaller than plot min
		else if((plot.min_y[0] - min.y) > 2 * r) 
		{
			min0->y = plot.min_y[0] - padding;
			flag = true;
		}

		// If data is right at the lower border then zoom out a little from the bottom
		else if(fabs(min.y - plot.min_y[0]) < padding)
		{
			min0->y = (min.y < plot.min_y[0]) ? 
				      min.y - padding :
				      plot.min_y[0] - padding;
			flag = true;
		}

		return flag;
	}


	TimeSeries plot;
	MyVector curr;
	MyVector prev;
	MyVector min;
	MyVector max;
	RangeType range;
	Direction xd;
	Direction yd;
	Markers* markers;
	MyVector offset;
	double drate;
	int physConn;
};

