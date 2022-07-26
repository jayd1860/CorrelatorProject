#pragma once

#include "DcsDeviceDataParser.h"
#include "Coord.h"
#include "CorrFunc.h"
#include "DataDisplay.h"

#define BUF_SIZE 2000000

namespace DcsAppForm 
{
	public ref class DataStreamPlot
	{

	public:
		// Data simulation params
        DataStreamPlot(PictureBox^ canvas,
                       double tWidth,
                       double dRateRaw,
                       double dRateGen,
                       double dRatePlot,
                       MyVector origin,
                       MyVector min0,
                       MyVector max0,
                       int nbins,
                       int nplots,
                       int plottype,
                       int toggleBFiAux)
		{
			m_data = new MyVector[BUF_SIZE];
			m_inext = 0;
			m_active = false;

			m_tWidth = tWidth;                  // Display window time width in seconds
			m_dRateRaw = dRateRaw;                    // The data generator speed in data points per second.
			m_dRateGen = dRateGen;                    // The data generator speed in data points per second.
			m_dRatePlot = dRatePlot;                    // The data generator speed in data points per second.
			m_dWidthRaw = m_tWidth * m_dRateRaw;  // Display window data width in number of data points
			m_dWidthGen = m_tWidth * m_dRateGen;  // Display window data width in number of data points
			m_dWidthPlot = m_tWidth * m_dRatePlot;  // Display window data width in number of data points
			m_updateRate = 1;       // Update rate in updates per seconds
			m_tStep = 1 / m_updateRate;                    // Step width in seconds
			m_dStep = (int)ceil(m_tStep * m_dRateGen);    // Step width in data points
			m_dIncrGen = floor(m_dRateRaw / m_dRateGen);
			m_dIncrDisp = floor(m_dRateGen / m_dRatePlot);
            m_plottype  = plottype;

			MyVector offset(0, 0);

			if(nbins > 0) {
				m_coord = new MyCoord(origin, min0, max0, positive, positive, offset, nbins);
			}
			else {
				m_coord = new MyCoord(origin, min0, max0, positive, positive, offset, m_dRateRaw);
			}

			// Initialize data output to GUI objects
            if(canvas != nullptr)
            {
                bool visible;

                if((toggleBFiAux == BFI) && (plottype & BFI))
                    visible = true;
                else if((toggleBFiAux == AUX) && (plottype & AUX))
                    visible = true;
                else if((toggleBFiAux == BFI) && (plottype & AUX))
                    visible = false;
                else if((toggleBFiAux == AUX) && (plottype & BFI))
                    visible = false;
                else if(plottype & CORR)
                    visible = true;

                disp = gcnew DataDisplay(canvas, *m_coord, nplots, m_plottype, visible);
            }

    		active = 1;
		}



		// ---------------------------------------------------------------------------------
		~DataStreamPlot()
		{
			delete m_data;
		}



		// ---------------------------------------------------------------------------------
		void AddDataItem(double y)
		{
			if(m_inext < BUF_SIZE)
				m_inext++;
			else
				m_inext=0;

			m_data[m_inext].x = m_inext;
			m_data[m_inext].y = y;
		}



		// ----------------------------------------------------------------------------------
		void Reset()
		{
			m_inext=0;
		}


        // ----------------------------------------------------------------------------------
		void UpdateParams(double tWidthNew, double ymin, double ymax, int size, bool multiplot, bool waterfall)
        {
            // Reset Y min/max
			if(!multiplot | !waterfall)
				m_coord->ResetYMinMax(ymin, ymax);
			else if(multiplot & waterfall)
				m_coord->ResetYMinMax(0.0, 1.0);


			if(waterfall)
				disp->turnPlotTypeOptionOn(WATERFALL);
			else
				disp->turnPlotTypeOptionOff(WATERFALL);
		}



		// ----------------------------------------------------------------------------------
		void ClearBuffers()
		{
			memset(m_coord->plot.data, 0, m_coord->range.x * sizeof(MyVector));
			memset(m_data, 0, BUF_SIZE * sizeof(MyVector));
		}


        // ----------------------------------------------------------------------------------
        void SetVisible(int toggleBFiAux)
        {
            bool visible;

            if((toggleBFiAux == BFI) && (m_plottype & BFI))
                visible = true;
            else if((toggleBFiAux == AUX) && (m_plottype & AUX))
                visible = true;
            else if((toggleBFiAux == BFI) && (m_plottype & AUX))
                visible = false;
            else if((toggleBFiAux == AUX) && (m_plottype & BFI))
                visible = false;
            else if(m_plottype & CORR)
                visible = true;

            disp->setVisible(visible);
        }


		MyVector* m_data;
		int    m_inext;
		double m_tWidth;
		double m_dRateRaw;                    // The data generator speed in data points per second.
		double m_dRateGen;                    // The data generator speed in data points per second.
		double m_dRatePlot;                    // The data generator speed in data points per second.
		double m_dWidthRaw;
		double m_dWidthGen;
		double m_dWidthPlot;
		double m_tStep;
		int   m_dStep;
		double m_nPlotPts;
		int   m_dStepInBytes;
		double m_updateRate;
		double m_dIncrGen;
		double m_dIncrDisp;
		int   m_physConn;
		bool  m_active;
		MyCoord* m_coord;
		DataDisplay^ disp;
		int active;
        int m_plottype;
	};
}
