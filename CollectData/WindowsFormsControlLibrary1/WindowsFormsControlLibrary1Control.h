#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace FPGAConfig;


#include <stdio.h>
#include <stdlib.h>
#include "Coord.h"

// Even though MyCoord is public it is inaccessible in a form unless we include 
// the following pragma (don't ask me why this is true).
#pragma make_public(MyCoord)

namespace WindowsFormsControlLibrary1 
{
	/// <summary>
	/// Summary for WindowsFormsControlLibrary1Control
	/// </summary>
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	public ref class WindowsFormsControlLibrary1Control : public System::Windows::Forms::UserControl
	{
	public:
		WindowsFormsControlLibrary1Control()
		{
			InitializeComponent();

			// Initialize nativeCoord
			double h = pictureBox1->Bounds.Height;
			double w = pictureBox1->Bounds.Width;
			MyVector origin(0,h);
			MyVector min(0,0);
			MyVector max(w,h);
			MyVector originOffset(70,20);
			nativeCoord = new MyCoord(origin, min, max, positive, negative, originOffset, 1.0);

			redPen = gcnew Pen(Color::Red);
			redPen->Width = 2;

			bluePen = gcnew Pen(Color::Blue);
			bluePen->Width = 2;

			cyanPen = gcnew Pen(Color::Cyan);
			cyanPen->Width = 2;

			magentaPen = gcnew Pen(Color::Magenta);
			magentaPen->Width = 2;

			greenPen = gcnew Pen(Color::Green);
			greenPen->Width = 2;

			brownPen = gcnew Pen(Color::Brown);
			brownPen->Width = 2;

			blackPen = gcnew Pen(Color::Black);
			blackPen->Width = 2;

			blueBrush = gcnew SolidBrush(Color::Blue);
		}

protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~WindowsFormsControlLibrary1Control()
		{
			if (components)
			{
				delete components;
			}
		}

	public: 

		void FPGAConfigEntry()
		{
			FPGAConfigForm^ form2 = gcnew FPGAConfigForm();
			Application::Run(form2);
		}


	public: System::Windows::Forms::PictureBox^  pictureBox1;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
			this->SuspendLayout();
			// 
			// pictureBox1
			// 
			this->pictureBox1->Location = System::Drawing::Point(23, 14);
			this->pictureBox1->Margin = System::Windows::Forms::Padding(4);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(599, 199);
			this->pictureBox1->TabIndex = 0;
			this->pictureBox1->TabStop = false;
			this->pictureBox1->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &WindowsFormsControlLibrary1Control::pictureBox1_Paint);
			// 
			// WindowsFormsControlLibrary1Control
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->Controls->Add(this->pictureBox1);
			this->Margin = System::Windows::Forms::Padding(4);
			this->Name = L"WindowsFormsControlLibrary1Control";
			this->Size = System::Drawing::Size(672, 360);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion

	public:

		// -------------------------------------------------------------------
		void setDataCoord(MyCoord& p)
		{
			dataCoord = new MyCoord(p);
		}


		// -------------------------------------------------------------------
		void setDataCoordMarkers(MyCoord* p)
		{
			for(int ii=0; ii<dataCoord->markers->num_x; ii++)
				dataCoord->markers->text_x[ii] = p->markers->text_x[ii];

			for(int jj=0; jj<dataCoord->markers->num_y; jj++)
				dataCoord->markers->text_y[jj] = p->markers->text_y[jj];
		}


		// -------------------------------------------------------------------
		void convertDataToNativeCoord(MyCoord* p1)
		{
			MyCoord* p2 = nativeCoord;

			p2->curr.x = (((p1->curr.x - p1->min.x) * (p2->range.x - p2->offset.x)) / p1->range.x) + p2->min.x + p2->offset.x;
			p2->curr.y = p2->max.y - p2->offset.y - (((p1->curr.y - p1->min.y) * (p2->range.y - p2->offset.y)) / p1->range.y); 

			p2->prev.x = (((p1->prev.x - p1->min.x) * (p2->range.x - p2->offset.x)) / p1->range.x) + p2->min.x + p2->offset.x;
			p2->prev.y = p2->max.y - p2->offset.y - (((p1->prev.y - p1->min.y) * (p2->range.y - p2->offset.y)) / p1->range.y); 
		}



		// -------------------------------------------------------------------
		void convertStaticDataToNativeCoord(MyCoord* p1, int idx)
		{
			MyCoord* p2 = nativeCoord;

			p2->curr.x = (((p1->plot.data[idx].x - p1->min.x) * (p2->range.x - p2->offset.x)) / p1->range.x) + p2->min.x + p2->offset.x;
			p2->curr.y = p2->max.y - p2->offset.y - (((p1->plot.data[idx].y - p1->min.y) * (p2->range.y - p2->offset.y)) / p1->range.y); 

			if(idx == 0)
				idx = 1;

			p2->prev.x = (((p1->plot.data[idx-1].x - p1->min.x) * (p2->range.x - p2->offset.x)) / p1->range.x) + p2->min.x + p2->offset.x;
			p2->prev.y = p2->max.y - p2->offset.y - (((p1->plot.data[idx-1].y - p1->min.y) * (p2->range.y - p2->offset.y)) / p1->range.y); 
		}



		// -------------------------------------------------------------------
		void convertRollingDataToNativeCoord(MyCoord* p1, int idx)
		{
			MyCoord* p2 = nativeCoord;

			p2->curr.x = ((((p1->plot.data[idx].x - p1->plot.data[0].x) - p1->min.x) * (p2->range.x - p2->offset.x)) / p1->range.x) + p2->min.x + p2->offset.x;
			p2->curr.y = p2->max.y - p2->offset.y - (((p1->plot.data[idx].y - p1->min.y) * (p2->range.y - p2->offset.y)) / p1->range.y); 

			if(idx==0)
				idx=1;

			p2->prev.x = ((((p1->plot.data[idx-1].x - p1->plot.data[0].x) - p1->min.x) * (p2->range.x - p2->offset.x)) / p1->range.x) + p2->min.x + p2->offset.x;
			p2->prev.y = p2->max.y - p2->offset.y - (((p1->plot.data[idx-1].y - p1->min.y) * (p2->range.y - p2->offset.y)) / p1->range.y); 
		}



		// -------------------------------------------------------------------
		void clearDisplay(MyCoord* p, int size, double drate)
		{
			Graphics^ g = pictureBox1->CreateGraphics();
			g->Clear(pictureBox1->BackColor);

			p->markers->MakeNewX(p->plot.data, size, drate);
			
			setDataCoordMarkers(p);
			redrawGrid(g);
		}



		// -------------------------------------------------------------------
		void clearDisplay(MyCoord* p, double drate)
		{
			Graphics^ g = pictureBox1->CreateGraphics();
			g->Clear(pictureBox1->BackColor);

			p->markers->ResetY(p->min, p->max);

			setDataCoordMarkers(p);
			redrawGrid(g);
		}


		// -------------------------------------------------------------------
		void clearDisplay(MyCoord* p)
		{
			Graphics^ g = pictureBox1->CreateGraphics();

			g->Clear(pictureBox1->BackColor);
			p->markers->ResetY(p->min, p->max);

			setDataCoordMarkers(p);
			redrawGrid(g);
		}



		// -------------------------------------------------------------------
		void plotData(MyCoord* p, int size, double drate)
		{
			Graphics^ g = pictureBox1->CreateGraphics();
			bool drawflag;
			MyVector min0(p->min.x, p->min.y);
			MyVector max0(p->max.x, p->max.y);

			if(p->IsDataOutOfRange(&min0, &max0, size))
			{
				p->ResetYMaxMin(min0, max0);
				clearDisplay(p, drate);
			}

			for(int ii=0; ii<size; ii++)
			{
				if(p->plot.data[ii].y  > p->max.y)
					p->plot.data[ii].y = p->max.y;
				else if(p->plot.data[ii].y  < p->min.y)
					p->plot.data[ii].y = p->min.y;

				drawflag = true;
				if((p->plot.data[ii].x-p->plot.data[0].x)  <= p->min.x)
					drawflag = false;

				// Convert p to native coordinates
				convertRollingDataToNativeCoord(p, ii);

				if(drawflag)
				{
					g->DrawLine(redPen, (float)nativeCoord->prev.x, (float)nativeCoord->prev.y, 
			                      		(float)nativeCoord->curr.x, (float)nativeCoord->curr.y);

				}
			}
		}


		// -------------------------------------------------------------------
		void plotData(MyCoord* p, int size, int color)
		{
			Graphics^ g = pictureBox1->CreateGraphics();
			bool drawflag;
			Pen^ pen;

			switch(color)
			{
			case(0):
				pen = redPen;
				break;
			case(1):
				pen = greenPen;
				break;
			case(2):
				pen = bluePen;
				break;
			case(3):
				pen = magentaPen;
				break;
			default:
				pen = redPen;
			};

			for(int ii=0; ii<size; ii++)
			{
				if(p->plot.data[ii].y  > p->max.y)
					p->plot.data[ii].y = p->max.y;
				else if(p->plot.data[ii].y  < p->min.y)
					p->plot.data[ii].y = p->min.y;

				if(p->plot.data[ii].x  > p->max.x)
					p->plot.data[ii].x = p->max.x;
				else if(p->plot.data[ii].x  < p->min.x)
					p->plot.data[ii].x = p->min.x;

				drawflag = true;
				if((p->plot.data[ii].x-p->plot.data[0].x)  <= p->min.x)
					drawflag = false;

				// Convert p to native coordinates
				convertStaticDataToNativeCoord(p, ii);

				if(drawflag)
				{
					g->DrawLine(pen, (float)nativeCoord->prev.x, (float)nativeCoord->prev.y, 
			                      	 (float)nativeCoord->curr.x, (float)nativeCoord->curr.y);

				}
			}
		}


	private: 

		// -------------------------------------------------------------------
		System::Void drawAxes(Graphics^ g, double x1, double y1, double x2, double y2)
		{
			g->DrawLine(blackPen,  (float)x1, (float)y1, (float)x1, (float)y2);
			g->DrawLine(blackPen,  (float)x2, (float)y1, (float)x2, (float)y2);
			g->DrawLine(blackPen,  (float)x1, (float)y1, (float)x2, (float)y1);
			g->DrawLine(blackPen,  (float)x1, (float)y2, (float)x2, (float)y2);
		}


		void drawMarkers(Graphics^ g, double x1, double y1, double x2, double y2)
		{
			MyCoord p = *dataCoord;
			char  s[20];
			String^ s2 = gcnew String(s);
			System::Drawing::Font^ font = this->Font;
			Rectangle textBox(0,0,70,20);
			int labelformat=2;

			// ----------------- Draw y axis markers -----------------------
			p.curr.x = x1;
			for(int jj=0; jj<dataCoord->markers->num_y; jj++)
			{
				// Draw line marker in native space 
				p.curr.y = dataCoord->markers->pos_y[jj];
				convertDataToNativeCoord(&p);
				g->DrawLine(blackPen, (float)x1-2, (float)nativeCoord->curr.y, (float)x1+4, (float)nativeCoord->curr.y);

				// Draw text markers to the right of line markers
				textBox.X = x1-70;
				textBox.Y = nativeCoord->curr.y-8;

				sprintf(s, "%0.3g", dataCoord->markers->text_y[jj]);
				String^ s2 = gcnew String(s);
				String^ markerstr = "" + s2;
				StringFormat^ format1 = gcnew StringFormat;
				format1->Alignment = StringAlignment::Far;
				g->DrawString(markerstr, font, blueBrush, (RectangleF)textBox, format1);
				delete s2;
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

				g->DrawLine(blackPen, (float)nativeCoord->curr.x, (float)nativeCoord->curr.y+5, (float)nativeCoord->curr.x, (float)nativeCoord->curr.y-5);

				// Draw text markers under the line markers
				textBox.X = nativeCoord->curr.x-10;
				textBox.Y = nativeCoord->curr.y+5;

				if(labelformat==1)
					sprintf(s, "%0.1f", dataCoord->markers->text_x[ii]);
				else
					sprintf(s, "%0.1e", dataCoord->markers->text_x[ii]);
				String^ s2 = gcnew String(s);
				String^ markerstr = "" + s2;
				StringFormat^ format1 = gcnew StringFormat;
				format1->Alignment = StringAlignment::Near;
				g->DrawString(markerstr, font, blueBrush, (RectangleF)textBox, format1);

				delete s2;
			}
		}



		// -------------------------------------------------------------------
		System::Void pictureBox1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
		{
			double x1 = nativeCoord->offset.x;
			double y1 = nativeCoord->max.y - nativeCoord->offset.y;
			double x2 = nativeCoord->max.x;
			double y2 = nativeCoord->min.y;

			// ------------ Draw data display borders --------------------
			//drawAxes(e->Graphics, x1, y1, x2, y2);

			redrawGrid(e->Graphics);
		}



		// -------------------------------------------------------------------
		System::Void redrawGrid(Graphics^ g)
		{
			double x1 = nativeCoord->offset.x;
			double y1 = nativeCoord->max.y - nativeCoord->offset.y;
			double x2 = nativeCoord->max.x;
			double y2 = nativeCoord->min.y;

			// ------------ Draw data display borders --------------------
			drawAxes(g, x1, y1, x2, y2);

			// ----------------- Draw x and y axis markers -----------------------
			drawMarkers(g, x1, y1, x2, y2);
		}

		
		Pen^ redPen;
		Pen^ greenPen;
		Pen^ brownPen;
		Pen^ bluePen;
		Pen^ cyanPen;
		Pen^ magentaPen;
		Pen^ blackPen;
		SolidBrush^ blueBrush;

		// Defines the coordinate system by setting the parameters of 
		// the lower left corner point.  
		MyCoord* nativeCoord;
		MyCoord* dataCoord;
	};
}
