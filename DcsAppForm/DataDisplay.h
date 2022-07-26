#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Threading;

#include <stdio.h>
#include <stdlib.h>
#include "DataStreamPlot.h"

#define     ANALOG_RAW_TO_VOLTS                 0.0006114-5.0

namespace DcsAppForm 
{

    enum PlotType
    {
        BFI         = 0x01,
        CORR        = 0x02,
        AUX         = 0x04,
        ALL      	= BFI | AUX, 
        ROLLING     = 0x08,
		STATIC      = 0x10,
		WATERFALL   = 0x20
	};

    public ref class DataDisplay
    {
    public:

        DataDisplay(PictureBox^ disp, MyCoord& p, int nplots, int plottype0, bool visible0)
        {
            pictureBox = disp;

            // Initialize nativeCoord
			double h = pictureBox->Bounds.Height;
			double w = pictureBox->Bounds.Width;
			MyVector origin(0, h);
			MyVector min_xy(0, 0);
			MyVector max_xy(w, h);
			MyVector originOffset(55, 20);
			nativeCoord = new MyCoord(origin, min_xy, max_xy, positive, negative, originOffset, 1.0);

			dataCoord = new MyCoord(p);

            nPens = 8;
            pens = gcnew array<Pen^>(nPens);

            pens[0] = gcnew Pen(Color::OrangeRed);
            pens[0]->Width = 2;
            pens[1] = gcnew Pen(Color::Yellow);
            pens[1]->Width = 2;
            pens[2] = gcnew Pen(Color::LightBlue);
            pens[2]->Width = 2;
            pens[3] = gcnew Pen(Color::Magenta);
            pens[3]->Width = 2;
            pens[4] = gcnew Pen(Color::Cyan);
            pens[4]->Width = 2;
            pens[5] = gcnew Pen(Color::Orange);
            pens[5]->Width = 2;
            pens[6] = gcnew Pen(Color::PaleVioletRed);
            pens[6]->Width = 2;
            pens[7] = gcnew Pen(Color::PaleGoldenrod);
            pens[7]->Width = 2;

            blackPen = gcnew Pen(Color::White);
            blackPen->Width = 1;

            blueBrush = gcnew SolidBrush(Color::WhiteSmoke);

			multplots = nplots>1? true : false;
			plottype = plottype0;
            visible = visible0;

            if(_critSect == nullptr) 
                _critSect = gcnew Object();
        }


        // -------------------------------------------------------------------
        inline void clear(Graphics^ g, System::Drawing::Color color)
        {
            Monitor::Enter(_critSect);
            g->Clear(color);
            Monitor::Exit(_critSect);
        }


        // -------------------------------------------------------------------
        inline void drawLine(Graphics^ g, System::Drawing::Pen^ pen, float x1, float y1, float x2, float y2)
        {
            Monitor::Enter(_critSect);
            g->DrawLine(pen, x1, y1, x2, y2);
            Monitor::Exit(_critSect);
        }


        // -------------------------------------------------------------------
        inline void drawLines(Graphics^ g, System::Drawing::Pen^ pen, array<System::Drawing::Point>^ points)
        {
            Monitor::Enter(_critSect);
            g->DrawLines(pen, points);
            Monitor::Exit(_critSect);
        }


        // -------------------------------------------------------------------
        inline void drawString(Graphics^ g, System::String^ s, System::Drawing::Font^ font, System::Drawing::Brush^ brush, System::Drawing::RectangleF layoutRectangle, System::Drawing::StringFormat^ format)
        {
            Monitor::Enter(_critSect);
            g->DrawString(s, font, brush, layoutRectangle, format);
            Monitor::Exit(_critSect);
        }
        

        
        // -------------------------------------------------------------------
		void setDataCoord(MyCoord* p)
        {
			dataCoord->min = p->min;
			dataCoord->max = p->max;
			dataCoord->range = p->range;
			dataCoord->markers->num_x = p->markers->num_x;
			dataCoord->markers->num_y = p->markers->num_y;
			
			for(int ii=0; ii<dataCoord->markers->num_x; ii++)
			{
				dataCoord->markers->pos_x[ii] = p->markers->pos_x[ii];
				dataCoord->markers->text_x[ii] = p->markers->text_x[ii];
			}
			for(int jj=0; jj<dataCoord->markers->num_y; jj++)
			{
				dataCoord->markers->pos_y[jj] = p->markers->pos_y[jj];
				dataCoord->markers->text_y[jj] = p->markers->text_y[jj];
			}
        }



        // ----------------------------------------------------------------------------------
        void setVisible(bool visible0)
        {
            visible = visible0;
        }


        // -------------------------------------------------------------------
        inline void convertDataToNativeCoord(MyCoord* p1)
        {
            MyCoord* p2 = nativeCoord;

            p2->curr.x = (((p1->curr.x - p1->min.x) * (p2->range.x - p2->offset.x)) / p1->range.x) + p2->min.x + p2->offset.x;
            p2->curr.y = p2->max.y - p2->offset.y - (((p1->curr.y - p1->min.y) * (p2->range.y - p2->offset.y)) / p1->range.y);

            p2->prev.x = (((p1->prev.x - p1->min.x) * (p2->range.x - p2->offset.x)) / p1->range.x) + p2->min.x + p2->offset.x;
            p2->prev.y = p2->max.y - p2->offset.y - (((p1->prev.y - p1->min.y) * (p2->range.y - p2->offset.y)) / p1->range.y);
        }



        // -------------------------------------------------------------------
        inline void convertStaticDataToNativeCoord(MyCoord* p1, int idx)
        {
            MyCoord* p2 = nativeCoord;

            p2->curr.x = (((p1->plot.data[idx].x - p1->min.x) * (p2->range.x - p2->offset.x)) / p1->range.x) + p2->min.x + p2->offset.x;
            p2->curr.y = p2->max.y - p2->offset.y - (((p1->plot.data[idx].y - p1->min.y) * (p2->range.y - p2->offset.y)) / p1->range.y);

            if(idx==0)
                idx = 1;

            p2->prev.x = (((p1->plot.data[idx-1].x - p1->min.x) * (p2->range.x - p2->offset.x)) / p1->range.x) + p2->min.x + p2->offset.x;
            p2->prev.y = p2->max.y - p2->offset.y - (((p1->plot.data[idx - 1].y - p1->min.y) * (p2->range.y - p2->offset.y)) / p1->range.y);
        }



        // -------------------------------------------------------------------
		inline void convertRollingDataToNativeCoord(MyCoord* p1, int idx)
        {
            MyCoord* p2 = nativeCoord;

			// Convert from data coord to native coord
            p2->curr.x = ((((p1->plot.data[idx].x - p1->plot.data[0].x) - p1->min.x) * (p2->range.x - p2->offset.x)) / p1->range.x) + p2->min.x + p2->offset.x;
            p2->curr.y = p2->max.y - p2->offset.y - (((p1->plot.data[idx].y - p1->min.y) * (p2->range.y - p2->offset.y)) / p1->range.y);

            if(idx==0)
                idx = 1;

            p2->prev.x = ((((p1->plot.data[idx-1].x - p1->plot.data[0].x) - p1->min.x) * (p2->range.x - p2->offset.x)) / p1->range.x) + p2->min.x + p2->offset.x;
            p2->prev.y = p2->max.y - p2->offset.y - (((p1->plot.data[idx - 1].y - p1->min.y) * (p2->range.y - p2->offset.y)) / p1->range.y);
        }



        // -------------------------------------------------------------------
		inline void convertRollingDataToNativeCoord(MyCoord* p1, MyCoord* p1new,  int idx, double order, double nplots)
        {
            MyCoord* p2 = nativeCoord;
			double p1new_max_y_new = order/nplots;
			double p1new_min_y_new = (order-1)/nplots;

			// Scaling to a portion of the canvas instead of the whole canvas
			p1->plot.data[idx].y = (p1->plot.data[idx].y-p1->min.y) * (p1new_max_y_new - p1new_min_y_new) / (p1->max.y - p1->min.y) + p1new_min_y_new;

			// Convert from data coord to native coord
            p2->curr.x = ((((p1->plot.data[idx].x - p1->plot.data[0].x) - p1new->min.x) * (p2->range.x - p2->offset.x)) / p1new->range.x) + p2->min.x + p2->offset.x;
            p2->curr.y = p2->max.y - p2->offset.y - (((p1->plot.data[idx].y - p1new->min.y) * (p2->range.y - p2->offset.y)) / p1new->range.y);

            if(idx==0)
                idx = 1;

            p2->prev.x = ((((p1->plot.data[idx-1].x - p1->plot.data[0].x) - p1new->min.x) * (p2->range.x - p2->offset.x)) / p1new->range.x) + p2->min.x + p2->offset.x;
            p2->prev.y = p2->max.y - p2->offset.y - (((p1->plot.data[idx-1].y - p1new->min.y) * (p2->range.y - p2->offset.y)) / p1new->range.y);
        }



		// -------------------------------------------------------------------
		void updateDisplay(MyCoord* p, int size, double drate)
		{
			Graphics^ g = pictureBox->CreateGraphics();

            if(visible)
                clear(g, pictureBox->BackColor);
			p->markers->ResetY(p->min, p->max);
			setDataCoord(p);
            if(visible)
                redrawGrid(g, pictureBox->Font);
		}




		// -------------------------------------------------------------------
		void updateDisplay(MyCoord* p, double drate)
		{
			Graphics^ g = pictureBox->CreateGraphics();

            if(visible)
                clear(g, pictureBox->BackColor);
			p->markers->ResetY(p->min, p->max);
			setDataCoord(p);
            if(visible)
                redrawGrid(g, pictureBox->Font);
		}




		// -------------------------------------------------------------------
		void clearDisplay(MyCoord* p, int size, double drate)
		{
			Graphics^ g = pictureBox->CreateGraphics();

            if(visible)
                clear(g, pictureBox->BackColor);
			p->markers->MakeNewX(p->plot.data, size, drate);
			setDataCoord(p);

            if(visible)
                redrawGrid(g, pictureBox->Font);
		}



		// -------------------------------------------------------------------
        void clearDisplay(MyCoord* p, double drate)
        {
            Graphics^ g = pictureBox->CreateGraphics();

            if(visible)
                clear(g, pictureBox->BackColor);
            p->markers->ResetY(p->min, p->max);

            setDataCoord(p);
            if(visible)
                redrawGrid(g, pictureBox->Font);
        }


        // -------------------------------------------------------------------
        void clearDisplay(MyCoord* p)
        {
            Graphics^ g = pictureBox->CreateGraphics();

            if(visible)
                clear(g, pictureBox->BackColor);
            p->markers->ResetY(p->min, p->max);

            setDataCoord(p);
            if(visible)
                redrawGrid(g, pictureBox->Font);
        }



        // -------------------------------------------------------------------
        inline void plotData(MyCoord* p, MyCoord* pnew, int size, double drate, int colorIdx, int order, int nplots)
        {
            Graphics^ g = pictureBox->CreateGraphics();
            bool drawflag;
            int ii, kk;

            if(size<2)
                return;

            array<Point>^ points = gcnew array<Point>(size);

            MyVector min0(p->min.x, p->min.y);
            MyVector max0(p->max.x, p->max.y);

            if(p->IsDataOutOfRange(&min0, &max0, size))
            {
                p->ResetYMinMax(min0, max0);
                clearDisplay(pnew, drate);
            }

            for(ii=0, kk=0; ii<size; ii++)
            {
                if(p->plot.data[ii].y > p->max.y)
                    p->plot.data[ii].y = p->max.y;
                else if(p->plot.data[ii].y < p->min.y)
                    p->plot.data[ii].y = p->min.y;

                drawflag = true;
                if((p->plot.data[ii].x - p->plot.data[0].x) < p->min.x)
                    drawflag = false;

                // Convert p to native coordinates
                convertRollingDataToNativeCoord(p, pnew, ii, (double)order, (double)nplots);

                if(drawflag)
                {
                    points[kk].X = (float)nativeCoord->curr.x;
                    points[kk].Y = (float)nativeCoord->curr.y;
                    kk++;
                }
            }
            if(visible)
                drawLines(g, pens[getColorIdx(colorIdx)], points);
        }



        // -------------------------------------------------------------------
        inline void plotData(MyCoord** p, MyCoord* pnew, int size, double drate, int* idx_active_plots, int nplots)
        {
            Graphics^ g = pictureBox->CreateGraphics();
            bool drawflag;
            int ii, jj, kk, ll;
            bool clear = false;

            if(size<2)
                return;

            array<Point>^ points = gcnew array<Point>(size);

            for(kk=0; kk<nplots; kk++)
            {
                jj = idx_active_plots[kk];

                MyVector min0(p[jj]->min.x, p[jj]->min.y);
                MyVector max0(p[jj]->max.x, p[jj]->max.y);

                if(p[jj]->IsDataOutOfRange(&min0, &max0, size))
                {
                    p[jj]->ResetYMinMax(min0, max0);
                    clear = true;
                }
            }    
            if(clear)
                clearDisplay(pnew, drate);

            for(kk=0; kk<nplots; kk++)
            {
                jj = idx_active_plots[kk];

                for(ii=0, ll=0; ii<size; ii++)
                {
                    if(p[jj]->plot.data[ii].y > p[jj]->max.y)
                        p[jj]->plot.data[ii].y = p[jj]->max.y;
                    else if(p[jj]->plot.data[ii].y < p[jj]->min.y)
                        p[jj]->plot.data[ii].y = p[jj]->min.y;

                    drawflag = true;
                    if((p[jj]->plot.data[ii].x - p[jj]->plot.data[0].x) < p[jj]->min.x)
                        drawflag = false;

                    // Convert p to native coordinates
					if(plottype & WATERFALL)
						convertRollingDataToNativeCoord(p[jj], pnew, ii, (double)kk+1, (double)nplots);
					else
						convertRollingDataToNativeCoord(p[jj], ii);

                    if(drawflag)
                    {
                        points[ll].X = (float)nativeCoord->curr.x;
                        points[ll].Y = (float)nativeCoord->curr.y;
                        ll++;
                    }
                }
                if(visible) {
                    if(plottype & AUX)
                        drawLines(g, pens[getColorIdxFromBottom(jj)], points);
                    else
                        drawLines(g, pens[getColorIdxFromTop(jj)], points);
                }
            }
        }



        // -------------------------------------------------------------------
		inline void plotData(MyCoord* p, int size, double drate, int colorIdx)
        {
            Graphics^ g = pictureBox->CreateGraphics();
            bool drawflag;
			MyVector min0(p->min.x, p->min.y);
			MyVector max0(p->max.x, p->max.y);
            int ii, kk;

            if(size<2)
                return;

            array<Point>^ points = gcnew array<Point>(size);

			if(p->IsDataOutOfRange(&min0, &max0, size))
			{
				p->ResetYMinMax(min0, max0);
				clearDisplay(p, drate);
			}

			for(ii=0, kk=0; ii<size; ii++)
			{
				if (p->plot.data[ii].y > p->max.y)
					p->plot.data[ii].y = p->max.y;
				else if (p->plot.data[ii].y < p->min.y)
					p->plot.data[ii].y = p->min.y;

                drawflag = true;
                if((p->plot.data[ii].x - p->plot.data[0].x) < p->min.x)
                    drawflag = false;
                
                // Convert p to native coordinates
				convertRollingDataToNativeCoord(p, ii);

                if(drawflag)
                {
                    points[kk].X = (float)nativeCoord->curr.x;
                    points[kk].Y = (float)nativeCoord->curr.y;
                    kk++;
                }
            }
            if(visible)
                drawLines(g, pens[getColorIdx(colorIdx)], points);
		}



		
        // -------------------------------------------------------------------
        inline void plotData(MyCoord* p, int size, int colorIdx)
        {
            Graphics^ g = pictureBox->CreateGraphics();
			bool drawflag;
            int ii, kk;

            if(size<2)
                return;

            array<Point>^ points = gcnew array<Point>(size);
            
            for(ii=0, kk=0; ii<size; ii++)
			{
				if(p->plot.data[ii].y > p->max.y)
					p->plot.data[ii].y = p->max.y;
				else if(p->plot.data[ii].y < p->min.y)
					p->plot.data[ii].y = p->min.y;

				if(p->plot.data[ii].x > p->max.x)
					p->plot.data[ii].x = p->max.x;
				else if(p->plot.data[ii].x < p->min.x)
					p->plot.data[ii].x = p->min.x;

				drawflag = true;
				if((p->plot.data[ii].x - p->plot.data[0].x) < p->min.x)
					drawflag = false;

				// Convert p to native coordinates
				convertStaticDataToNativeCoord(p, ii);

				if(drawflag)
				{
                    points[kk].X = (float)nativeCoord->curr.x;
                    points[kk].Y = (float)nativeCoord->curr.y;
                    kk++;
                }
            }
            if(visible)
                drawLines(g, pens[colorIdx], points);
        }


		// -------------------------------------------------------------------
		void turnPlotTypeOptionOn(enum PlotType option)
		{
			plottype |= option;
		}



		// --------------------------------------------------------------------
		void turnPlotTypeOptionOff(enum PlotType option)
		{
			plottype &= ~option;
		}


	private:

		// -------------------------------------------------------------------
		System::Void drawAxes(Graphics^ g, double x1, double y1, double x2, double y2)
		{
			drawLine(g, blackPen, (float)x1, (float)y1, (float)x1, (float)y2);
			drawLine(g, blackPen, (float)x1, (float)y1, (float)x2, (float)y1);
		}


        // -------------------------------------------------------------------
		void drawMarkers(Graphics^ g, Font^ font, double x1, double y1, double x2, double y2)
        {
			MyCoord p = *dataCoord;
            char*  s = new char[20]();
			System::Drawing::Rectangle textBox(0,0,70,20);
            int labelformat = 2;

            // ----------------- Draw y axis markers -----------------------
            p.curr.x = x1;
            for(int jj=0; jj<dataCoord->markers->num_y; jj++)
            {
                // Draw line marker in native space 
                p.curr.y = dataCoord->markers->pos_y[jj];
                convertDataToNativeCoord(&p);
                drawLine(g, blackPen, (float)x1-2, (float)nativeCoord->curr.y, (float)x1+4, (float)nativeCoord->curr.y);

				// Draw text markers to the right of line markers. Draw them only if we are plotting 
				// single plot in the textBox display.
				if((multplots==false) || (plottype & STATIC) || ((plottype & WATERFALL)==0))
				{
	                textBox.X = (int)(x1-70);
	                textBox.Y = (int)nativeCoord->curr.y - 8;
	
					if(plottype & STATIC) {
						sprintf(s, "%0.3g", dataCoord->markers->text_y[jj]);
						char* dotptr = strstr(s, ".");
						int lendiff = 3-strlen(dotptr);
						if(lendiff == 1)
							sprintf(s, "%s0", s);
						else if(lendiff == 2)
							sprintf(s, "%s00", s);
						else if(lendiff == 3)
							sprintf(s, "%s00", s);
					}
					else
						sprintf(s, "%0.2g", dataCoord->markers->text_y[jj]);

					String^ s2 = gcnew String(s);
	                String^ markerstr = "" + s2;
	                StringFormat^ format1 = gcnew StringFormat;
	                format1->Alignment = StringAlignment::Far;
	                drawString(g, markerstr, font, blueBrush, (RectangleF)textBox, format1);
	            }
			}

            // ----------------- Draw x axis markers -----------------------
            // Determine label display format 
            for(int ii=0; ii<dataCoord->markers->num_x; ii++)
            {
                if(fabs(dataCoord->markers->text_x[ii]) > 1)
                    labelformat = 1;
            }

            p.curr.y = p.min.y;
            for(int ii=0; ii<dataCoord->markers->num_x; ii++)
            {
                // Draw line marker in native space 
                p.curr.x = dataCoord->markers->pos_x[ii];
                convertDataToNativeCoord(&p);

                drawLine(g, blackPen, (float)nativeCoord->curr.x, (float)nativeCoord->curr.y+5, 
                            (float)nativeCoord->curr.x, (float)nativeCoord->curr.y-5);

                // Draw text markers under the line markers
                textBox.X = (int)(nativeCoord->curr.x-10);
                textBox.Y = (int)(nativeCoord->curr.y+5);

                if(labelformat == 1)
                    sprintf(s, "%0.1f", dataCoord->markers->text_x[ii]);
                else
                    sprintf(s, "%0.1e", dataCoord->markers->text_x[ii]);
                String^ s2 = gcnew String(s);
                String^ markerstr = "" + s2;
                StringFormat^ format1 = gcnew StringFormat;
                format1->Alignment = StringAlignment::Near;
                drawString(g, markerstr, font, blueBrush, (RectangleF)textBox, format1);
            }
			delete s; 
        }

	public:

	    // -------------------------------------------------------------------
        System::Void redrawGrid(Graphics^ g, System::Drawing::Font^ font)
        {
            double x1 = nativeCoord->offset.x;
            double y1 = nativeCoord->max.y - nativeCoord->offset.y;
            double x2 = nativeCoord->max.x;
            double y2 = nativeCoord->min.y;

            // ------------ Draw data display borders --------------------
            drawAxes(g, x1, y1, x2, y2);

            // ----------------- Draw x and y axis markers -----------------------
            drawMarkers(g, font, x1, y1, x2, y2);
        }


		// -------------------------------------------------------------------
		inline int getColorIdx(int idx)
		{
			return nPens-idx-1;
		}


		// -------------------------------------------------------------------
		inline Color getColorFromTop(int idx)
		{
			return pens[idx]->Color;
		}


		// -------------------------------------------------------------------
		inline Color getColorFromBottom(int idx)
		{
			return pens[getColorIdx(idx)]->Color;
		}


        // -------------------------------------------------------------------
        inline int getColorIdxFromTop(int idx)
        {
            return idx;
        }


        // -------------------------------------------------------------------
        inline int getColorIdxFromBottom(int idx)
        {
            return getColorIdx(idx);
        }


    public:

        PictureBox^ pictureBox;
        
        array<Pen^>^ pens;
        int nPens;

        Pen^ blackPen;
        SolidBrush^ blueBrush;
	
		// Defines the coordinate system by setting the parameters of 
		// the lower left corner point.  
		MyCoord* nativeCoord;
		MyCoord* dataCoord;

		bool multplots;
		int  plottype;
        bool visible;
        static Object^  _critSect;
	};
}
