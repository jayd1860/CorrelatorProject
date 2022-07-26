#pragma once

// NOTE: Putting #include <windows.h> before the using any namespaces. If you 
// have it after then you'll get all kinds of compile errors in the unmanaged 
// portions of your code that uses WINAPI.
#include <windows.h>
#include <shlwapi.h> 
#include <stdio.h>
#include <stdlib.h>
#include <limits>
#include <float.h>
#include "Utils.h"
#include "Coord.h"
#include "DcsDeviceDataParser.h"
#include "CorrFunc.h"
#include "DataStreamPlot.h"
#include "Fx3Device.h"
#include "MyFile.h"
#include "LightAndTissPropertiesForm.h"

typedef struct _CY_DATA_BUFFER {
    UCHAR *buffer;                      /*Pointer to the buffer from where the data is read/written */
    UINT32 length;                      /*Length of the buffer */
    int    count;
    _CY_DATA_BUFFER *next;
} CY_DATA_BUFFER, *PCY_DATA_BUFFER;


enum InputSrc
{
    DeviceSuperSpeed  = 1,
    DeviceNormalSpeed = 2,
    DeviceAny = DeviceSuperSpeed | DeviceNormalSpeed,
    File_ = 4,
    DeviceSuperspeedOrFile = DeviceSuperSpeed | File_,
    None = 8
};


enum SaveOptions
{
    NoSaveNoReplay = 0,
    SaveRaw		   = 1,
    SaveCorr	   = 2,
    SaveAux		   = 4,
    Replay		   = 8
};


namespace DcsAppForm {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;
    using namespace System::Threading;
    using namespace System::IO;
    using namespace FPGAConfig;


    /// <summary>
    /// Summary for DcsAppForm
    /// </summary>
    public ref class DcsAppForm : public System::Windows::Forms::Form
    {

    public:
        DcsAppForm(void)
        {
            InitializeComponent();

            double tWidth = 40;                  // Display window time width in seconds
            double dRateRaw;                    // The data generator speed in data points per second.
            double dRateGen;                    // The data generator speed in data points per second.
            double dRatePlot;                    // The data generator speed in data points per second.
            MyVector origindata(0.0, 0.0);
            MyVector mindata;
            MyVector maxdata;
            int iLast;
            wchar_t datetime[13];
            wchar_t dirname[100];
            wchar_t filename[100];
            wchar_t* wstr = new wchar_t[20]();
            char* str = new char[20]();
            wchar_t dirnameroot[512] = L"C:\\Users\\Public\\DCS";
            wchar_t dirnameroot_sim[512];

            memset(datetime, 0, sizeof(datetime));
            getCurrDateTimeString(datetime);
            wsprintf(dirname, L"dcs_%s", datetime);
            wsprintf(filename, L"dcs_run001");

            // Initialize the save folders and files
            memset(dirnameroot_sim, 0, sizeof(dirnameroot_sim));			
            if(GetCurrentDirectory(sizeof(dirnameroot_sim), dirnameroot_sim) == 0)
                memcpy(dirnameroot_sim, dirnameroot, sizeof(dirnameroot));
            wsprintf(dirnameroot_sim, L"%s\\..\\SimData", dirnameroot_sim);

            m_saveFileDataRaw = gcnew MyFile(dirnameroot, dirname, filename, L"raw");
            m_saveFileDataCorr = gcnew MyFile(dirnameroot, dirname, filename, L"corr");
            m_saveFileDataAux = gcnew MyFile(dirnameroot, dirname, filename, L"aux");
            m_replayFileData = gcnew MyFile();
            m_simulateData = gcnew MyFile(dirnameroot_sim, L"", L"simData", L"bin");
            
            this->textBoxSessionFolder->Text = gcnew String(m_saveFileDataRaw->GetDirnameAcqFull());
            this->textBoxBaseFileName->Text = gcnew String(m_saveFileDataRaw->GetFilenameBase());

            m_MaxFiles = 100;
            m_MaxNameSize = m_saveFileDataRaw->GetMaxNameSize();
            m_nRun = 0;

            AddDevicesToDropdownMenu();

            usbdevice = new Fx3Device();
            m_toggleBFiAux = BFI;
            m_BFi = new double[m_parser->dataDCS->nCh];
            m_beta = new double[m_parser->dataDCS->nCh];
            m_err = new double[m_parser->dataDCS->nCh];
            m_chDlg_rate = new int[m_parser->dataDCS->nCh];
            m_chDlg_BFi = new int[m_parser->dataDCS->nCh];
            m_enableDet = new int[m_parser->dataDCS->nCh];

            m_waterfall = false;

            // Initialize data input objects
            nplots_aux = m_parser->dataADC->nCh;
            nplots_corr = m_parser->dataDCS->nCh;
            nplots_bfi = m_parser->dataDCS->nCh;

            aux = gcnew array<DataStreamPlot^>(nplots_aux);
            corr = gcnew array<DataStreamPlot^>(nplots_corr);
            bfi = gcnew array<DataStreamPlot^>(nplots_bfi);

            array<DataDisplay^>^ dataDispAuxSingle = gcnew array<DataDisplay^>(nplots_aux);

            // Create data display for single-plot aux data
            dRateRaw = usbdevice->GetDataRate();                    // The data generator speed in data points per second.
            dRateGen = dRateRaw/100.0;                    // The data generator speed in data points per second.
            dRatePlot = dRateGen/2.0;                    // The data generator speed in data points per second.
            mindata.Set(0.0, 0.0);
            maxdata.Set(tWidth*dRateRaw, 20.0);
            for(int ii=0; ii<nplots_corr; ii++) {
                aux[ii] = gcnew DataStreamPlot(this->pictureBox1, tWidth, dRateRaw, dRateGen, dRatePlot,
                    origindata, mindata, maxdata, 0, 1, ROLLING | AUX, m_toggleBFiAux);
            }

            // Create data display for mutiple-plot aux data
            maxdata.Set(tWidth*dRateRaw, 1.0);
            auxmult = gcnew DataStreamPlot(this->pictureBox1, tWidth, dRateRaw, dRateGen, dRatePlot, origindata,
                mindata, maxdata, 0, nplots_aux, ROLLING | AUX | WATERFALL, m_toggleBFiAux);

            // Create data display for single-plot BFi data
            dRateRaw = 1.0;                    // The data generator speed in data points per second.
            dRateGen = dRateRaw;                    // The data generator speed in data points per second.
            dRatePlot = dRateGen;                    // The data generator speed in data points per second.
            mindata.Set(0.0, 0.0);
            maxdata.Set(tWidth*dRateRaw, 1e5);
            for(int ii=0; ii<nplots_bfi; ii++) {
                bfi[ii] = gcnew DataStreamPlot(this->pictureBox1, tWidth, dRateRaw, dRateGen, dRatePlot,
                    origindata, mindata, maxdata, 0, 1, ROLLING | BFI, m_toggleBFiAux);
                bfi[ii]->m_coord->plot.SetAutozoom(false);
            }

            // Create data display for mutiple-plot bfi data
            int plottype = m_waterfall ? ROLLING | BFI | WATERFALL : ROLLING | BFI;
            if(plottype & WATERFALL)
                maxdata.Set(tWidth*dRateRaw, 1.0);
            bfimult = gcnew DataStreamPlot(this->pictureBox1, tWidth, dRateRaw, dRateGen, dRatePlot, origindata,
                mindata, maxdata, 0, nplots_bfi, plottype, m_toggleBFiAux);

            tWidth = 0.0;                    // This field isn't used for this display
            dRateRaw = m_corrdata[0]->nbins;  // The data generator speed in data points per second.
            dRateGen = m_corrdata[0]->nbins;  // The data generator speed in data points per second.
            dRatePlot = m_corrdata[0]->nbins; // The data generator speed in data points per second.
            double min_x_corr = 4e-7;
            double max_x_corr = 1e-2;
            mindata.Set(log10(min_x_corr), 0.99);
            maxdata.Set(log10(max_x_corr), 1.70);
            for(int ii=0; ii<nplots_corr; ii++) {
                corr[ii] = gcnew DataStreamPlot(this->pictureBox3, tWidth, dRateRaw, dRateGen, dRatePlot,
                    origindata, mindata, maxdata, m_corrdata[0]->nbins, 1, STATIC | CORR, m_toggleBFiAux);
            }

            // Create data display for mutiple-plot corr data
            corrmult = gcnew DataStreamPlot(this->pictureBox3, tWidth, dRateRaw, dRateGen, dRatePlot, origindata,
                mindata, maxdata, m_corrdata[0]->nbins, nplots_corr, STATIC | CORR, m_toggleBFiAux);

            m_nDeviceIndex = comboBoxDevices->SelectedIndex;
            m_maxPktSizeDefault = (long)pow(2.0, 14.0);

            m_save = 0;

            checkBoxAux1->Checked = aux[0]->active;
            checkBoxAux2->Checked = aux[1]->active;
            checkBoxAux3->Checked = aux[2]->active;
            checkBoxAux4->Checked = aux[3]->active;

            checkBoxBFi1->Checked = bfi[0]->active;
            checkBoxBFi2->Checked = bfi[0]->active;
            checkBoxBFi3->Checked = bfi[0]->active;
            checkBoxBFi4->Checked = bfi[0]->active;

            checkBoxCorr1->Checked = corr[0]->active;
            checkBoxCorr2->Checked = corr[1]->active;
            checkBoxCorr3->Checked = corr[2]->active;
            checkBoxCorr4->Checked = corr[3]->active;

            checkedplots_aux = 0;
            checkedplots_bfi = 0;
            checkedplots_corr = 0;

            for(int ii=0; ii<nplots_aux; ii++)
                checkedplots_aux |= aux[ii]->active << ii;
            for(int ii=0; ii<nplots_bfi; ii++)
                checkedplots_bfi |= bfi[ii]->active << ii;
            for(int ii=0; ii<nplots_corr; ii++)
                checkedplots_corr |= corr[ii]->active << ii;

            numbitson_init();

            idx_active_plots_aux = new int[nplots_aux]();
            idx_active_plots_bfi = new int[nplots_bfi]();
            idx_active_plots_corr = new int[nplots_corr]();

            m_acquisitionPaused = false;

            acqElapsedTime = 0;

            this->checkBoxAux1->BackColor = aux[0]->disp->getColorFromBottom(0);
            this->checkBoxAux2->BackColor = aux[1]->disp->getColorFromBottom(1);
            this->checkBoxAux3->BackColor = aux[2]->disp->getColorFromBottom(2);
            this->checkBoxAux4->BackColor = aux[3]->disp->getColorFromBottom(3);

            this->labelDet1->BackColor = corr[0]->disp->getColorFromTop(0);
            this->labelDet2->BackColor = corr[1]->disp->getColorFromTop(1);
            this->labelDet3->BackColor = corr[2]->disp->getColorFromTop(2);
            this->labelDet4->BackColor = corr[3]->disp->getColorFromTop(3);

            this->checkBoxAutoZoom->Checked = bfimult->m_coord->plot.autozoom;

            delete usbdevice;
            usbdevice = NULL;

            countTextBoxes = gcnew array<Control^>(4);
            bfiTextBoxes = gcnew array<Control^>(4);

            countTextBoxes[0] = gcnew Control(this->textBoxDet1Count, gcnew String(L"0"));
            countTextBoxes[1] = gcnew Control(this->textBoxDet2Count, gcnew String(L"0"));
            countTextBoxes[2] = gcnew Control(this->textBoxDet3Count, gcnew String(L"0"));
            countTextBoxes[3] = gcnew Control(this->textBoxDet4Count, gcnew String(L"0"));

            bfiTextBoxes[0]   = gcnew Control(this->textBoxDet1BFi, gcnew String(L"0.0,  0.0"));
            bfiTextBoxes[1]   = gcnew Control(this->textBoxDet2BFi, gcnew String(L"0.0,  0.0"));
            bfiTextBoxes[2]   = gcnew Control(this->textBoxDet3BFi, gcnew String(L"0.0,  0.0"));
            bfiTextBoxes[3]   = gcnew Control(this->textBoxDet4BFi, gcnew String(L"0.0,  0.0"));

            for(int ii=0; ii<4; ii++) {
                countTextBoxes[ii]->Text = gcnew String(L"0");
                bfiTextBoxes[ii]->Text = gcnew String(L"0.0,  0.0");
            }

            swprintf(wstr, L"%0.1f", bfi[0]->m_tWidth);
            this->textBoxXWidth->Text = gcnew String(wstr);

            swprintf(wstr, L"%0.2g", bfi[0]->m_coord->min.y);
            this->textBoxYMin->Text = gcnew String(wstr);

            swprintf(wstr, L"%0.2g", bfi[0]->m_coord->max.y);
            this->textBoxYMax->Text = gcnew String(wstr);

            convertStringToChar(this->textBoxXWidth->Text, str);
            pictureBox1_twidth = atof(str);

            convertStringToChar(this->textBoxYMin->Text, str);
            pictureBox1_ymin = atof(str);

            convertStringToChar(this->textBoxYMax->Text, str);
            pictureBox1_ymax = atof(str);

            checkBoxWaterfall->Checked = m_waterfall;

            // Set the correlation display X and Y min/max text boxes
            iLast = corr[0]->m_coord->markers->num_x-1;
            sprintf(str, "%0.2g", corr[0]->m_coord->markers->text_x[0]);
            this->textBoxCorrXMin->Text = gcnew String(str);
            sprintf(str, "%0.2g", corr[0]->m_coord->markers->text_x[iLast]);
            this->textBoxCorrXMax->Text = gcnew String(str);

            iLast = corr[0]->m_coord->markers->num_y-1;
            sprintf(str, "%0.2f", corr[0]->m_coord->min.y);
            this->textBoxCorrYMin->Text = gcnew String(str);
            sprintf(str, "%0.2f", corr[0]->m_coord->max.y);
            this->textBoxCorrYMax->Text = gcnew String(str);

            buttonStartForeColor = Color::FromArgb(255, 0, 0);
            buttonStartBackColor = Color::FromArgb(50, 200, 40);
            buttonStopForeColor = Color::FromArgb(255, 255, 0);
            buttonStopBackColor = Color::FromArgb(200, 50, 40);
            textBoxElapsedTimeColor_AcqStart = buttonStopBackColor;
            textBoxElapsedTimeColor_AcqStop = Color::FromArgb(0, 0, 0);

            this->buttonStartAcquisition->ForeColor = buttonStartForeColor;
            this->buttonStartAcquisition->BackColor = buttonStartBackColor;
            this->textBoxElapsedTime->ForeColor = textBoxElapsedTimeColor_AcqStop;

            laserpower = 0;

            qHead = NULL;
            qTail = NULL;
            _critSect = gcnew Object();

            // Set iniial acquisition expiration time
            comboBoxExpirationTime->Items->Add("10.0");
            comboBoxExpirationTime->Items->Add("20.0");
            comboBoxExpirationTime->Items->Add("30.0");
            comboBoxExpirationTime->Items->Add("40.0");
            comboBoxExpirationTime->Items->Add("50.0");
            comboBoxExpirationTime->Items->Add("60.0");
            comboBoxExpirationTime->Items->Add("20000.0");
            comboBoxExpirationTime->SelectedIndex = 6;
            convertStringToChar(comboBoxExpirationTime->Text, str);
            if(str[0] != 0)
                m_expTime = atof(str);

            checkBoxShowBFi->Checked = true;
            checkBoxWaterfall->Checked = false;

            errstatus = ERROR_SUCCESS;

            delete str;
            delete wstr;
        }

    protected:
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        ~DcsAppForm()
        {
            if(components)
            {
                delete components;
            }
        }
    private: System::Windows::Forms::GroupBox^  groupBoxSaveData;
    protected:
    private: System::Windows::Forms::Button^  buttonBrowseFolders;
    private: System::Windows::Forms::Button^  buttonBrowseFiles;
    private: System::Windows::Forms::TextBox^  textBoxBaseFileName;
    private: System::Windows::Forms::TextBox^  textBoxSessionFolder;
    private: System::Windows::Forms::ComboBox^  comboBoxDevices;
    private: System::Windows::Forms::Button^  buttonFpgaConfig;
    private: System::Windows::Forms::Button^  buttonStartAcquisition;
    private: System::Windows::Forms::PictureBox^  pictureBox1;

    private: System::Windows::Forms::PictureBox^  pictureBox3;
    private: System::Windows::Forms::RadioButton^  radioButtonReplaySavedData;

    private: System::Windows::Forms::RadioButton^  radioButtonSaveCorrData;

    private: System::Windows::Forms::CheckBox^  checkBoxAux1;
    private: System::Windows::Forms::CheckBox^  checkBoxAux2;
    private: System::Windows::Forms::CheckBox^  checkBoxAux3;
    private: System::Windows::Forms::CheckBox^  checkBoxAux4;

    private: System::Windows::Forms::Button^  buttonPauseAcquisition;
    private: System::Windows::Forms::CheckBox^  checkBoxAutoZoom;
    private: System::Windows::Forms::CheckBox^  checkBoxBFi4;
    private: System::Windows::Forms::CheckBox^  checkBoxBFi3;
    private: System::Windows::Forms::CheckBox^  checkBoxBFi2;
    private: System::Windows::Forms::CheckBox^  checkBoxBFi1;
    private: System::Windows::Forms::TextBox^  textBoxCorrEapsedTime;
    private: System::Windows::Forms::Label^  label1;
    private: System::Windows::Forms::CheckBox^  checkBoxCorr4;
    private: System::Windows::Forms::CheckBox^  checkBoxCorr3;
    private: System::Windows::Forms::CheckBox^  checkBoxCorr2;
    private: System::Windows::Forms::CheckBox^  checkBoxCorr1;
    private: System::Windows::Forms::Label^  labelDet4;
    private: System::Windows::Forms::Label^  labelDet3;
    private: System::Windows::Forms::Label^  labelDet2;
    private: System::Windows::Forms::Label^  labelDet1;
    private: System::Windows::Forms::Label^  labelCorr;

    private: System::Windows::Forms::TextBox^  textBoxDet1Count;
    private: System::Windows::Forms::TextBox^  textBoxDet2Count;
    private: System::Windows::Forms::TextBox^  textBoxDet3Count;
    private: System::Windows::Forms::TextBox^  textBoxDet4Count;
    private: System::Windows::Forms::TextBox^  textBoxDet4BFi;
    private: System::Windows::Forms::TextBox^  textBoxDet3BFi;
    private: System::Windows::Forms::TextBox^  textBoxDet2BFi;
    private: System::Windows::Forms::TextBox^  textBoxDet1BFi;
    private: System::Windows::Forms::Label^  label2;
    private: System::Windows::Forms::Label^  label3;
    private: System::Windows::Forms::GroupBox^  groupBox1;
    private: System::Windows::Forms::TextBox^  textBoxXWidth;
    private: System::Windows::Forms::Label^  label4;
    private: System::Windows::Forms::Label^  label5;
    private: System::Windows::Forms::Label^  label7;
    private: System::Windows::Forms::TextBox^  textBoxYMin;
    private: System::Windows::Forms::TextBox^  textBoxYMax;
private: System::Windows::Forms::CheckBox^  checkBoxWaterfall;

    private: System::Windows::Forms::TextBox^  textBoxCorrXMax;
    private: System::Windows::Forms::TextBox^  textBoxCorrXMin;
    private: System::Windows::Forms::Label^  labelCorrXMinMax;
    private: System::Windows::Forms::Label^  labelCorrYMinMax;
    private: System::Windows::Forms::TextBox^  textBoxCorrYMin;
    private: System::Windows::Forms::TextBox^  textBoxCorrYMax;
    private: System::Windows::Forms::GroupBox^  groupBoxLaserPower;
    private: System::Windows::Forms::Label^  labelLaserPower;

    private: System::Windows::Forms::ListBox^  listBoxLaserPowerStatus;
    private: System::Windows::Forms::Label^  labelLaserPowerMillAmps;
    private: System::Windows::Forms::TextBox^  textBoxSetLaserPower;



    private: System::Windows::Forms::ComboBox^  comboBoxExpirationTime;
    private: System::Windows::Forms::RadioButton^  radioButtonSaveRawData;
    private: System::Windows::Forms::Label^  label10;
    private: System::Windows::Forms::Label^  label9;
    private: System::Windows::Forms::Label^  label8;
    private: System::Windows::Forms::Label^  label6;
    private: System::Windows::Forms::Label^  labelElapsedTime;

    private: System::Windows::Forms::TextBox^  textBoxElapsedTime;
    private: System::Windows::Forms::CheckBox^  checkBoxShowBFi;


    private: System::Windows::Forms::MenuStrip^  menuStrip1;
    private: System::Windows::Forms::ToolStripMenuItem^  propertiesToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  lightTissuePropertiesToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  laserPowerToolStripMenuItem;
private: System::Windows::Forms::CheckBox^  checkBoxShowAux;



    private:
        /// <summary>
        /// Required designer variable.
        /// </summary>
        System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        void InitializeComponent(void)
        {
            this->groupBoxSaveData = (gcnew System::Windows::Forms::GroupBox());
            this->label10 = (gcnew System::Windows::Forms::Label());
            this->label9 = (gcnew System::Windows::Forms::Label());
            this->label8 = (gcnew System::Windows::Forms::Label());
            this->label6 = (gcnew System::Windows::Forms::Label());
            this->radioButtonReplaySavedData = (gcnew System::Windows::Forms::RadioButton());
            this->comboBoxExpirationTime = (gcnew System::Windows::Forms::ComboBox());
            this->radioButtonSaveCorrData = (gcnew System::Windows::Forms::RadioButton());
            this->buttonBrowseFolders = (gcnew System::Windows::Forms::Button());
            this->radioButtonSaveRawData = (gcnew System::Windows::Forms::RadioButton());
            this->buttonBrowseFiles = (gcnew System::Windows::Forms::Button());
            this->textBoxBaseFileName = (gcnew System::Windows::Forms::TextBox());
            this->textBoxSessionFolder = (gcnew System::Windows::Forms::TextBox());
            this->comboBoxDevices = (gcnew System::Windows::Forms::ComboBox());
            this->buttonFpgaConfig = (gcnew System::Windows::Forms::Button());
            this->buttonStartAcquisition = (gcnew System::Windows::Forms::Button());
            this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
            this->pictureBox3 = (gcnew System::Windows::Forms::PictureBox());
            this->checkBoxAux1 = (gcnew System::Windows::Forms::CheckBox());
            this->checkBoxAux2 = (gcnew System::Windows::Forms::CheckBox());
            this->checkBoxAux3 = (gcnew System::Windows::Forms::CheckBox());
            this->checkBoxAux4 = (gcnew System::Windows::Forms::CheckBox());
            this->buttonPauseAcquisition = (gcnew System::Windows::Forms::Button());
            this->checkBoxAutoZoom = (gcnew System::Windows::Forms::CheckBox());
            this->checkBoxBFi4 = (gcnew System::Windows::Forms::CheckBox());
            this->checkBoxBFi3 = (gcnew System::Windows::Forms::CheckBox());
            this->checkBoxBFi2 = (gcnew System::Windows::Forms::CheckBox());
            this->checkBoxBFi1 = (gcnew System::Windows::Forms::CheckBox());
            this->textBoxCorrEapsedTime = (gcnew System::Windows::Forms::TextBox());
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->checkBoxCorr4 = (gcnew System::Windows::Forms::CheckBox());
            this->checkBoxCorr3 = (gcnew System::Windows::Forms::CheckBox());
            this->checkBoxCorr2 = (gcnew System::Windows::Forms::CheckBox());
            this->checkBoxCorr1 = (gcnew System::Windows::Forms::CheckBox());
            this->labelDet4 = (gcnew System::Windows::Forms::Label());
            this->labelDet3 = (gcnew System::Windows::Forms::Label());
            this->labelDet2 = (gcnew System::Windows::Forms::Label());
            this->labelDet1 = (gcnew System::Windows::Forms::Label());
            this->labelCorr = (gcnew System::Windows::Forms::Label());
            this->textBoxDet1Count = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDet2Count = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDet3Count = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDet4Count = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDet4BFi = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDet3BFi = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDet2BFi = (gcnew System::Windows::Forms::TextBox());
            this->textBoxDet1BFi = (gcnew System::Windows::Forms::TextBox());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->label3 = (gcnew System::Windows::Forms::Label());
            this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
            this->textBoxXWidth = (gcnew System::Windows::Forms::TextBox());
            this->label4 = (gcnew System::Windows::Forms::Label());
            this->label5 = (gcnew System::Windows::Forms::Label());
            this->label7 = (gcnew System::Windows::Forms::Label());
            this->textBoxYMin = (gcnew System::Windows::Forms::TextBox());
            this->textBoxYMax = (gcnew System::Windows::Forms::TextBox());
            this->checkBoxWaterfall = (gcnew System::Windows::Forms::CheckBox());
            this->textBoxCorrXMax = (gcnew System::Windows::Forms::TextBox());
            this->textBoxCorrXMin = (gcnew System::Windows::Forms::TextBox());
            this->labelCorrXMinMax = (gcnew System::Windows::Forms::Label());
            this->labelCorrYMinMax = (gcnew System::Windows::Forms::Label());
            this->textBoxCorrYMin = (gcnew System::Windows::Forms::TextBox());
            this->textBoxCorrYMax = (gcnew System::Windows::Forms::TextBox());
            this->groupBoxLaserPower = (gcnew System::Windows::Forms::GroupBox());
            this->labelLaserPowerMillAmps = (gcnew System::Windows::Forms::Label());
            this->textBoxSetLaserPower = (gcnew System::Windows::Forms::TextBox());
            this->labelLaserPower = (gcnew System::Windows::Forms::Label());
            this->listBoxLaserPowerStatus = (gcnew System::Windows::Forms::ListBox());
            this->labelElapsedTime = (gcnew System::Windows::Forms::Label());
            this->textBoxElapsedTime = (gcnew System::Windows::Forms::TextBox());
            this->checkBoxShowBFi = (gcnew System::Windows::Forms::CheckBox());
            this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
            this->propertiesToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->lightTissuePropertiesToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->laserPowerToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->checkBoxShowAux = (gcnew System::Windows::Forms::CheckBox());
            this->groupBoxSaveData->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox3))->BeginInit();
            this->groupBox1->SuspendLayout();
            this->groupBoxLaserPower->SuspendLayout();
            this->menuStrip1->SuspendLayout();
            this->SuspendLayout();
            // 
            // groupBoxSaveData
            // 
            this->groupBoxSaveData->Controls->Add(this->label10);
            this->groupBoxSaveData->Controls->Add(this->label9);
            this->groupBoxSaveData->Controls->Add(this->label8);
            this->groupBoxSaveData->Controls->Add(this->label6);
            this->groupBoxSaveData->Controls->Add(this->radioButtonReplaySavedData);
            this->groupBoxSaveData->Controls->Add(this->comboBoxExpirationTime);
            this->groupBoxSaveData->Controls->Add(this->radioButtonSaveCorrData);
            this->groupBoxSaveData->Controls->Add(this->buttonBrowseFolders);
            this->groupBoxSaveData->Controls->Add(this->radioButtonSaveRawData);
            this->groupBoxSaveData->Controls->Add(this->buttonBrowseFiles);
            this->groupBoxSaveData->Controls->Add(this->textBoxBaseFileName);
            this->groupBoxSaveData->Controls->Add(this->textBoxSessionFolder);
            this->groupBoxSaveData->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->groupBoxSaveData->Location = System::Drawing::Point(12, 31);
            this->groupBoxSaveData->Margin = System::Windows::Forms::Padding(2);
            this->groupBoxSaveData->Name = L"groupBoxSaveData";
            this->groupBoxSaveData->Padding = System::Windows::Forms::Padding(2);
            this->groupBoxSaveData->Size = System::Drawing::Size(526, 164);
            this->groupBoxSaveData->TabIndex = 0;
            this->groupBoxSaveData->TabStop = false;
            this->groupBoxSaveData->Text = L"Save Data";
            // 
            // label10
            // 
            this->label10->AutoSize = true;
            this->label10->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label10->Location = System::Drawing::Point(180, 130);
            this->label10->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->label10->Name = L"label10";
            this->label10->Size = System::Drawing::Size(60, 15);
            this->label10->TabIndex = 15;
            this->label10->Text = L"seconds";
            // 
            // label9
            // 
            this->label9->AutoSize = true;
            this->label9->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label9->Location = System::Drawing::Point(18, 128);
            this->label9->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->label9->Name = L"label9";
            this->label9->Size = System::Drawing::Size(66, 15);
            this->label9->TabIndex = 14;
            this->label9->Text = L"Acq Time";
            this->label9->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
            // 
            // label8
            // 
            this->label8->AutoSize = true;
            this->label8->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label8->Location = System::Drawing::Point(50, 95);
            this->label8->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->label8->Name = L"label8";
            this->label8->Size = System::Drawing::Size(31, 15);
            this->label8->TabIndex = 13;
            this->label8->Text = L"File";
            this->label8->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
            // 
            // label6
            // 
            this->label6->AutoSize = true;
            this->label6->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label6->Location = System::Drawing::Point(35, 59);
            this->label6->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->label6->Name = L"label6";
            this->label6->Size = System::Drawing::Size(48, 15);
            this->label6->TabIndex = 12;
            this->label6->Text = L"Folder";
            this->label6->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
            // 
            // radioButtonReplaySavedData
            // 
            this->radioButtonReplaySavedData->AutoCheck = false;
            this->radioButtonReplaySavedData->AutoSize = true;
            this->radioButtonReplaySavedData->Enabled = false;
            this->radioButtonReplaySavedData->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->radioButtonReplaySavedData->Location = System::Drawing::Point(363, 23);
            this->radioButtonReplaySavedData->Margin = System::Windows::Forms::Padding(2);
            this->radioButtonReplaySavedData->Name = L"radioButtonReplaySavedData";
            this->radioButtonReplaySavedData->Size = System::Drawing::Size(146, 19);
            this->radioButtonReplaySavedData->TabIndex = 7;
            this->radioButtonReplaySavedData->TabStop = true;
            this->radioButtonReplaySavedData->Text = L"Replay Saved Data";
            this->radioButtonReplaySavedData->UseVisualStyleBackColor = true;
            this->radioButtonReplaySavedData->Click += gcnew System::EventHandler(this, &DcsAppForm::radioButtonReplaySavedData_Click);
            // 
            // comboBoxExpirationTime
            // 
            this->comboBoxExpirationTime->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->comboBoxExpirationTime->FormattingEnabled = true;
            this->comboBoxExpirationTime->Location = System::Drawing::Point(85, 126);
            this->comboBoxExpirationTime->Margin = System::Windows::Forms::Padding(2);
            this->comboBoxExpirationTime->Name = L"comboBoxExpirationTime";
            this->comboBoxExpirationTime->Size = System::Drawing::Size(92, 23);
            this->comboBoxExpirationTime->TabIndex = 11;
            this->comboBoxExpirationTime->DisplayMemberChanged += gcnew System::EventHandler(this, &DcsAppForm::comboBoxExpirationTime_DisplayMemberChanged);
            this->comboBoxExpirationTime->Leave += gcnew System::EventHandler(this, &DcsAppForm::comboBoxExpirationTime_Leave);
            // 
            // radioButtonSaveCorrData
            // 
            this->radioButtonSaveCorrData->AutoCheck = false;
            this->radioButtonSaveCorrData->AutoSize = true;
            this->radioButtonSaveCorrData->Enabled = false;
            this->radioButtonSaveCorrData->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->radioButtonSaveCorrData->Location = System::Drawing::Point(175, 23);
            this->radioButtonSaveCorrData->Margin = System::Windows::Forms::Padding(2);
            this->radioButtonSaveCorrData->Name = L"radioButtonSaveCorrData";
            this->radioButtonSaveCorrData->Size = System::Drawing::Size(165, 19);
            this->radioButtonSaveCorrData->TabIndex = 6;
            this->radioButtonSaveCorrData->TabStop = true;
            this->radioButtonSaveCorrData->Text = L"Save Correlation Data";
            this->radioButtonSaveCorrData->UseVisualStyleBackColor = true;
            this->radioButtonSaveCorrData->Click += gcnew System::EventHandler(this, &DcsAppForm::radioButtonSaveCorrData_Click);
            // 
            // buttonBrowseFolders
            // 
            this->buttonBrowseFolders->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->buttonBrowseFolders->Location = System::Drawing::Point(380, 54);
            this->buttonBrowseFolders->Margin = System::Windows::Forms::Padding(2);
            this->buttonBrowseFolders->Name = L"buttonBrowseFolders";
            this->buttonBrowseFolders->Size = System::Drawing::Size(77, 27);
            this->buttonBrowseFolders->TabIndex = 3;
            this->buttonBrowseFolders->Text = L"Browse";
            this->buttonBrowseFolders->UseVisualStyleBackColor = true;
            this->buttonBrowseFolders->Click += gcnew System::EventHandler(this, &DcsAppForm::buttonBrowseFolders_Click);
            // 
            // radioButtonSaveRawData
            // 
            this->radioButtonSaveRawData->AutoCheck = false;
            this->radioButtonSaveRawData->AutoSize = true;
            this->radioButtonSaveRawData->Enabled = false;
            this->radioButtonSaveRawData->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->radioButtonSaveRawData->Location = System::Drawing::Point(43, 23);
            this->radioButtonSaveRawData->Margin = System::Windows::Forms::Padding(2);
            this->radioButtonSaveRawData->Name = L"radioButtonSaveRawData";
            this->radioButtonSaveRawData->Size = System::Drawing::Size(122, 19);
            this->radioButtonSaveRawData->TabIndex = 5;
            this->radioButtonSaveRawData->TabStop = true;
            this->radioButtonSaveRawData->Text = L"Save Raw Data";
            this->radioButtonSaveRawData->UseVisualStyleBackColor = true;
            this->radioButtonSaveRawData->Click += gcnew System::EventHandler(this, &DcsAppForm::radioButtonSaveRawData_Click);
            // 
            // buttonBrowseFiles
            // 
            this->buttonBrowseFiles->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->buttonBrowseFiles->Location = System::Drawing::Point(380, 89);
            this->buttonBrowseFiles->Margin = System::Windows::Forms::Padding(2);
            this->buttonBrowseFiles->Name = L"buttonBrowseFiles";
            this->buttonBrowseFiles->Size = System::Drawing::Size(77, 27);
            this->buttonBrowseFiles->TabIndex = 4;
            this->buttonBrowseFiles->Text = L"Browse";
            this->buttonBrowseFiles->UseVisualStyleBackColor = true;
            this->buttonBrowseFiles->Click += gcnew System::EventHandler(this, &DcsAppForm::buttonBrowseFiles_Click);
            // 
            // textBoxBaseFileName
            // 
            this->textBoxBaseFileName->Location = System::Drawing::Point(85, 93);
            this->textBoxBaseFileName->Margin = System::Windows::Forms::Padding(2);
            this->textBoxBaseFileName->Name = L"textBoxBaseFileName";
            this->textBoxBaseFileName->Size = System::Drawing::Size(280, 21);
            this->textBoxBaseFileName->TabIndex = 1;
            this->textBoxBaseFileName->Leave += gcnew System::EventHandler(this, &DcsAppForm::textBoxBaseFileName_Leave);
            // 
            // textBoxSessionFolder
            // 
            this->textBoxSessionFolder->Location = System::Drawing::Point(85, 57);
            this->textBoxSessionFolder->Margin = System::Windows::Forms::Padding(2);
            this->textBoxSessionFolder->Name = L"textBoxSessionFolder";
            this->textBoxSessionFolder->Size = System::Drawing::Size(280, 21);
            this->textBoxSessionFolder->TabIndex = 0;
            this->textBoxSessionFolder->Leave += gcnew System::EventHandler(this, &DcsAppForm::textBoxSessionFolder_Leave);
            // 
            // comboBoxDevices
            // 
            this->comboBoxDevices->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->comboBoxDevices->FormattingEnabled = true;
            this->comboBoxDevices->Location = System::Drawing::Point(589, 35);
            this->comboBoxDevices->Margin = System::Windows::Forms::Padding(2);
            this->comboBoxDevices->Name = L"comboBoxDevices";
            this->comboBoxDevices->Size = System::Drawing::Size(151, 25);
            this->comboBoxDevices->TabIndex = 1;
            this->comboBoxDevices->Click += gcnew System::EventHandler(this, &DcsAppForm::comboBoxDevices_Click);
            // 
            // buttonFpgaConfig
            // 
            this->buttonFpgaConfig->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->buttonFpgaConfig->Location = System::Drawing::Point(756, 30);
            this->buttonFpgaConfig->Margin = System::Windows::Forms::Padding(2);
            this->buttonFpgaConfig->Name = L"buttonFpgaConfig";
            this->buttonFpgaConfig->Size = System::Drawing::Size(89, 31);
            this->buttonFpgaConfig->TabIndex = 2;
            this->buttonFpgaConfig->Text = L"FPGA Config";
            this->buttonFpgaConfig->UseVisualStyleBackColor = true;
            this->buttonFpgaConfig->Click += gcnew System::EventHandler(this, &DcsAppForm::buttonFpgaConfig_Click);
            // 
            // buttonStartAcquisition
            // 
            this->buttonStartAcquisition->Enabled = false;
            this->buttonStartAcquisition->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->buttonStartAcquisition->Location = System::Drawing::Point(691, 252);
            this->buttonStartAcquisition->Margin = System::Windows::Forms::Padding(2);
            this->buttonStartAcquisition->Name = L"buttonStartAcquisition";
            this->buttonStartAcquisition->Size = System::Drawing::Size(91, 39);
            this->buttonStartAcquisition->TabIndex = 3;
            this->buttonStartAcquisition->Text = L"START";
            this->buttonStartAcquisition->UseVisualStyleBackColor = true;
            this->buttonStartAcquisition->Click += gcnew System::EventHandler(this, &DcsAppForm::buttonStartAcquisition_Click);
            // 
            // pictureBox1
            // 
            this->pictureBox1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(32)), static_cast<System::Int32>(static_cast<System::Byte>(64)),
                static_cast<System::Int32>(static_cast<System::Byte>(16)));
            this->pictureBox1->Enabled = false;
            this->pictureBox1->Location = System::Drawing::Point(11, 255);
            this->pictureBox1->Margin = System::Windows::Forms::Padding(2);
            this->pictureBox1->Name = L"pictureBox1";
            this->pictureBox1->Size = System::Drawing::Size(659, 448);
            this->pictureBox1->TabIndex = 4;
            this->pictureBox1->TabStop = false;
            this->pictureBox1->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &DcsAppForm::pictureBox1_Paint);
            // 
            // pictureBox3
            // 
            this->pictureBox3->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(32)), static_cast<System::Int32>(static_cast<System::Byte>(64)),
                static_cast<System::Int32>(static_cast<System::Byte>(16)));
            this->pictureBox3->Location = System::Drawing::Point(803, 252);
            this->pictureBox3->Margin = System::Windows::Forms::Padding(2);
            this->pictureBox3->Name = L"pictureBox3";
            this->pictureBox3->Size = System::Drawing::Size(346, 257);
            this->pictureBox3->TabIndex = 6;
            this->pictureBox3->TabStop = false;
            this->pictureBox3->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &DcsAppForm::pictureBox3_Paint);
            // 
            // checkBoxAux1
            // 
            this->checkBoxAux1->AutoSize = true;
            this->checkBoxAux1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->checkBoxAux1->Location = System::Drawing::Point(358, 230);
            this->checkBoxAux1->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxAux1->Name = L"checkBoxAux1";
            this->checkBoxAux1->Size = System::Drawing::Size(58, 17);
            this->checkBoxAux1->TabIndex = 7;
            this->checkBoxAux1->Text = L"Aux 1";
            this->checkBoxAux1->UseVisualStyleBackColor = true;
            this->checkBoxAux1->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxAux1_Click);
            // 
            // checkBoxAux2
            // 
            this->checkBoxAux2->AutoSize = true;
            this->checkBoxAux2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->checkBoxAux2->Location = System::Drawing::Point(431, 230);
            this->checkBoxAux2->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxAux2->Name = L"checkBoxAux2";
            this->checkBoxAux2->Size = System::Drawing::Size(58, 17);
            this->checkBoxAux2->TabIndex = 8;
            this->checkBoxAux2->Text = L"Aux 2";
            this->checkBoxAux2->UseVisualStyleBackColor = true;
            this->checkBoxAux2->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxAux2_Click);
            // 
            // checkBoxAux3
            // 
            this->checkBoxAux3->AutoSize = true;
            this->checkBoxAux3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->checkBoxAux3->Location = System::Drawing::Point(505, 231);
            this->checkBoxAux3->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxAux3->Name = L"checkBoxAux3";
            this->checkBoxAux3->Size = System::Drawing::Size(58, 17);
            this->checkBoxAux3->TabIndex = 9;
            this->checkBoxAux3->Text = L"Aux 3";
            this->checkBoxAux3->UseVisualStyleBackColor = true;
            this->checkBoxAux3->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxAux3_Click);
            // 
            // checkBoxAux4
            // 
            this->checkBoxAux4->AutoSize = true;
            this->checkBoxAux4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->checkBoxAux4->Location = System::Drawing::Point(577, 230);
            this->checkBoxAux4->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxAux4->Name = L"checkBoxAux4";
            this->checkBoxAux4->Size = System::Drawing::Size(58, 17);
            this->checkBoxAux4->TabIndex = 10;
            this->checkBoxAux4->Text = L"Aux 4";
            this->checkBoxAux4->UseVisualStyleBackColor = true;
            this->checkBoxAux4->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxAux4_Click);
            // 
            // buttonPauseAcquisition
            // 
            this->buttonPauseAcquisition->Enabled = false;
            this->buttonPauseAcquisition->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->buttonPauseAcquisition->Location = System::Drawing::Point(691, 304);
            this->buttonPauseAcquisition->Margin = System::Windows::Forms::Padding(2);
            this->buttonPauseAcquisition->Name = L"buttonPauseAcquisition";
            this->buttonPauseAcquisition->Size = System::Drawing::Size(91, 39);
            this->buttonPauseAcquisition->TabIndex = 12;
            this->buttonPauseAcquisition->Text = L"PAUSE";
            this->buttonPauseAcquisition->UseVisualStyleBackColor = true;
            this->buttonPauseAcquisition->Visible = false;
            this->buttonPauseAcquisition->Click += gcnew System::EventHandler(this, &DcsAppForm::buttonPauseAcquisition_Click);
            // 
            // checkBoxAutoZoom
            // 
            this->checkBoxAutoZoom->AutoSize = true;
            this->checkBoxAutoZoom->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->checkBoxAutoZoom->Location = System::Drawing::Point(14, 717);
            this->checkBoxAutoZoom->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxAutoZoom->Name = L"checkBoxAutoZoom";
            this->checkBoxAutoZoom->Size = System::Drawing::Size(87, 17);
            this->checkBoxAutoZoom->TabIndex = 13;
            this->checkBoxAutoZoom->Text = L"Auto-Zoom";
            this->checkBoxAutoZoom->UseVisualStyleBackColor = true;
            this->checkBoxAutoZoom->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxAutoZoom_Click);
            // 
            // checkBoxBFi4
            // 
            this->checkBoxBFi4->AutoSize = true;
            this->checkBoxBFi4->Location = System::Drawing::Point(100, 147);
            this->checkBoxBFi4->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxBFi4->Name = L"checkBoxBFi4";
            this->checkBoxBFi4->Size = System::Drawing::Size(15, 14);
            this->checkBoxBFi4->TabIndex = 17;
            this->checkBoxBFi4->UseVisualStyleBackColor = true;
            this->checkBoxBFi4->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxBFi4_Click);
            // 
            // checkBoxBFi3
            // 
            this->checkBoxBFi3->AutoSize = true;
            this->checkBoxBFi3->Location = System::Drawing::Point(100, 113);
            this->checkBoxBFi3->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxBFi3->Name = L"checkBoxBFi3";
            this->checkBoxBFi3->Size = System::Drawing::Size(15, 14);
            this->checkBoxBFi3->TabIndex = 16;
            this->checkBoxBFi3->UseVisualStyleBackColor = true;
            this->checkBoxBFi3->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxBFi3_Click);
            // 
            // checkBoxBFi2
            // 
            this->checkBoxBFi2->AutoSize = true;
            this->checkBoxBFi2->Location = System::Drawing::Point(100, 79);
            this->checkBoxBFi2->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxBFi2->Name = L"checkBoxBFi2";
            this->checkBoxBFi2->Size = System::Drawing::Size(15, 14);
            this->checkBoxBFi2->TabIndex = 15;
            this->checkBoxBFi2->UseVisualStyleBackColor = true;
            this->checkBoxBFi2->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxBFi2_Click);
            // 
            // checkBoxBFi1
            // 
            this->checkBoxBFi1->AutoSize = true;
            this->checkBoxBFi1->Location = System::Drawing::Point(100, 46);
            this->checkBoxBFi1->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxBFi1->Name = L"checkBoxBFi1";
            this->checkBoxBFi1->Size = System::Drawing::Size(15, 14);
            this->checkBoxBFi1->TabIndex = 14;
            this->checkBoxBFi1->UseVisualStyleBackColor = true;
            this->checkBoxBFi1->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxBFi1_Click);
            // 
            // textBoxCorrEapsedTime
            // 
            this->textBoxCorrEapsedTime->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->textBoxCorrEapsedTime->Location = System::Drawing::Point(1072, 229);
            this->textBoxCorrEapsedTime->Margin = System::Windows::Forms::Padding(2);
            this->textBoxCorrEapsedTime->Name = L"textBoxCorrEapsedTime";
            this->textBoxCorrEapsedTime->Size = System::Drawing::Size(76, 19);
            this->textBoxCorrEapsedTime->TabIndex = 18;
            this->textBoxCorrEapsedTime->Text = L"0";
            this->textBoxCorrEapsedTime->Visible = false;
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label1->Location = System::Drawing::Point(916, 230);
            this->label1->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(152, 13);
            this->label1->TabIndex = 19;
            this->label1->Text = L"Correlation Elapsed Time ";
            this->label1->Visible = false;
            // 
            // checkBoxCorr4
            // 
            this->checkBoxCorr4->AutoSize = true;
            this->checkBoxCorr4->Location = System::Drawing::Point(309, 150);
            this->checkBoxCorr4->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxCorr4->Name = L"checkBoxCorr4";
            this->checkBoxCorr4->Size = System::Drawing::Size(15, 14);
            this->checkBoxCorr4->TabIndex = 23;
            this->checkBoxCorr4->UseVisualStyleBackColor = true;
            this->checkBoxCorr4->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxCorr4_Click);
            // 
            // checkBoxCorr3
            // 
            this->checkBoxCorr3->AutoSize = true;
            this->checkBoxCorr3->Location = System::Drawing::Point(309, 116);
            this->checkBoxCorr3->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxCorr3->Name = L"checkBoxCorr3";
            this->checkBoxCorr3->Size = System::Drawing::Size(15, 14);
            this->checkBoxCorr3->TabIndex = 22;
            this->checkBoxCorr3->UseVisualStyleBackColor = true;
            this->checkBoxCorr3->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxCorr3_Click);
            // 
            // checkBoxCorr2
            // 
            this->checkBoxCorr2->AutoSize = true;
            this->checkBoxCorr2->Location = System::Drawing::Point(309, 81);
            this->checkBoxCorr2->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxCorr2->Name = L"checkBoxCorr2";
            this->checkBoxCorr2->Size = System::Drawing::Size(15, 14);
            this->checkBoxCorr2->TabIndex = 21;
            this->checkBoxCorr2->UseVisualStyleBackColor = true;
            this->checkBoxCorr2->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxCorr2_Click);
            // 
            // checkBoxCorr1
            // 
            this->checkBoxCorr1->AutoSize = true;
            this->checkBoxCorr1->Location = System::Drawing::Point(309, 48);
            this->checkBoxCorr1->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxCorr1->Name = L"checkBoxCorr1";
            this->checkBoxCorr1->Size = System::Drawing::Size(15, 14);
            this->checkBoxCorr1->TabIndex = 20;
            this->checkBoxCorr1->UseVisualStyleBackColor = true;
            this->checkBoxCorr1->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxCorr1_Click);
            // 
            // labelDet4
            // 
            this->labelDet4->AutoSize = true;
            this->labelDet4->Location = System::Drawing::Point(26, 145);
            this->labelDet4->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->labelDet4->Name = L"labelDet4";
            this->labelDet4->Size = System::Drawing::Size(48, 17);
            this->labelDet4->TabIndex = 24;
            this->labelDet4->Text = L"DET 4";
            // 
            // labelDet3
            // 
            this->labelDet3->AutoSize = true;
            this->labelDet3->Location = System::Drawing::Point(26, 110);
            this->labelDet3->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->labelDet3->Name = L"labelDet3";
            this->labelDet3->Size = System::Drawing::Size(48, 17);
            this->labelDet3->TabIndex = 25;
            this->labelDet3->Text = L"DET 3";
            // 
            // labelDet2
            // 
            this->labelDet2->AutoSize = true;
            this->labelDet2->Location = System::Drawing::Point(26, 78);
            this->labelDet2->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->labelDet2->Name = L"labelDet2";
            this->labelDet2->Size = System::Drawing::Size(48, 17);
            this->labelDet2->TabIndex = 26;
            this->labelDet2->Text = L"DET 2";
            // 
            // labelDet1
            // 
            this->labelDet1->AutoSize = true;
            this->labelDet1->Location = System::Drawing::Point(26, 43);
            this->labelDet1->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->labelDet1->Name = L"labelDet1";
            this->labelDet1->Size = System::Drawing::Size(48, 17);
            this->labelDet1->TabIndex = 27;
            this->labelDet1->Text = L"DET 1";
            // 
            // labelCorr
            // 
            this->labelCorr->AutoSize = true;
            this->labelCorr->Location = System::Drawing::Point(273, 18);
            this->labelCorr->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->labelCorr->Name = L"labelCorr";
            this->labelCorr->Size = System::Drawing::Size(77, 17);
            this->labelCorr->TabIndex = 28;
            this->labelCorr->Text = L"Correlation";
            // 
            // textBoxDet1Count
            // 
            this->textBoxDet1Count->Location = System::Drawing::Point(354, 43);
            this->textBoxDet1Count->Margin = System::Windows::Forms::Padding(2);
            this->textBoxDet1Count->Name = L"textBoxDet1Count";
            this->textBoxDet1Count->Size = System::Drawing::Size(65, 23);
            this->textBoxDet1Count->TabIndex = 30;
            this->textBoxDet1Count->Text = L"0";
            // 
            // textBoxDet2Count
            // 
            this->textBoxDet2Count->Location = System::Drawing::Point(354, 77);
            this->textBoxDet2Count->Margin = System::Windows::Forms::Padding(2);
            this->textBoxDet2Count->Name = L"textBoxDet2Count";
            this->textBoxDet2Count->Size = System::Drawing::Size(65, 23);
            this->textBoxDet2Count->TabIndex = 31;
            this->textBoxDet2Count->Text = L"0";
            // 
            // textBoxDet3Count
            // 
            this->textBoxDet3Count->Location = System::Drawing::Point(354, 111);
            this->textBoxDet3Count->Margin = System::Windows::Forms::Padding(2);
            this->textBoxDet3Count->Name = L"textBoxDet3Count";
            this->textBoxDet3Count->Size = System::Drawing::Size(65, 23);
            this->textBoxDet3Count->TabIndex = 32;
            this->textBoxDet3Count->Text = L"0";
            // 
            // textBoxDet4Count
            // 
            this->textBoxDet4Count->Location = System::Drawing::Point(354, 145);
            this->textBoxDet4Count->Margin = System::Windows::Forms::Padding(2);
            this->textBoxDet4Count->Name = L"textBoxDet4Count";
            this->textBoxDet4Count->Size = System::Drawing::Size(65, 23);
            this->textBoxDet4Count->TabIndex = 33;
            this->textBoxDet4Count->Text = L"0";
            // 
            // textBoxDet4BFi
            // 
            this->textBoxDet4BFi->Location = System::Drawing::Point(142, 144);
            this->textBoxDet4BFi->Margin = System::Windows::Forms::Padding(2);
            this->textBoxDet4BFi->Name = L"textBoxDet4BFi";
            this->textBoxDet4BFi->Size = System::Drawing::Size(108, 23);
            this->textBoxDet4BFi->TabIndex = 37;
            this->textBoxDet4BFi->Text = L"0";
            // 
            // textBoxDet3BFi
            // 
            this->textBoxDet3BFi->Location = System::Drawing::Point(142, 110);
            this->textBoxDet3BFi->Margin = System::Windows::Forms::Padding(2);
            this->textBoxDet3BFi->Name = L"textBoxDet3BFi";
            this->textBoxDet3BFi->Size = System::Drawing::Size(108, 23);
            this->textBoxDet3BFi->TabIndex = 36;
            this->textBoxDet3BFi->Text = L"0";
            // 
            // textBoxDet2BFi
            // 
            this->textBoxDet2BFi->Location = System::Drawing::Point(142, 76);
            this->textBoxDet2BFi->Margin = System::Windows::Forms::Padding(2);
            this->textBoxDet2BFi->Name = L"textBoxDet2BFi";
            this->textBoxDet2BFi->Size = System::Drawing::Size(108, 23);
            this->textBoxDet2BFi->TabIndex = 35;
            this->textBoxDet2BFi->Text = L"0";
            // 
            // textBoxDet1BFi
            // 
            this->textBoxDet1BFi->Location = System::Drawing::Point(142, 42);
            this->textBoxDet1BFi->Margin = System::Windows::Forms::Padding(2);
            this->textBoxDet1BFi->Name = L"textBoxDet1BFi";
            this->textBoxDet1BFi->Size = System::Drawing::Size(108, 23);
            this->textBoxDet1BFi->TabIndex = 34;
            this->textBoxDet1BFi->Text = L"0";
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->Location = System::Drawing::Point(361, 18);
            this->label2->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(52, 17);
            this->label2->TabIndex = 38;
            this->label2->Text = L"Counts";
            // 
            // label3
            // 
            this->label3->AutoSize = true;
            this->label3->Location = System::Drawing::Point(175, 18);
            this->label3->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->label3->Name = L"label3";
            this->label3->Size = System::Drawing::Size(28, 17);
            this->label3->TabIndex = 39;
            this->label3->Text = L"BFi";
            // 
            // groupBox1
            // 
            this->groupBox1->Controls->Add(this->label3);
            this->groupBox1->Controls->Add(this->label2);
            this->groupBox1->Controls->Add(this->textBoxDet4BFi);
            this->groupBox1->Controls->Add(this->textBoxDet3BFi);
            this->groupBox1->Controls->Add(this->textBoxDet2BFi);
            this->groupBox1->Controls->Add(this->textBoxDet1BFi);
            this->groupBox1->Controls->Add(this->labelDet1);
            this->groupBox1->Controls->Add(this->textBoxDet4Count);
            this->groupBox1->Controls->Add(this->labelDet2);
            this->groupBox1->Controls->Add(this->textBoxDet3Count);
            this->groupBox1->Controls->Add(this->labelDet3);
            this->groupBox1->Controls->Add(this->textBoxDet2Count);
            this->groupBox1->Controls->Add(this->labelDet4);
            this->groupBox1->Controls->Add(this->checkBoxBFi4);
            this->groupBox1->Controls->Add(this->textBoxDet1Count);
            this->groupBox1->Controls->Add(this->checkBoxBFi3);
            this->groupBox1->Controls->Add(this->checkBoxBFi2);
            this->groupBox1->Controls->Add(this->checkBoxBFi1);
            this->groupBox1->Controls->Add(this->labelCorr);
            this->groupBox1->Controls->Add(this->checkBoxCorr2);
            this->groupBox1->Controls->Add(this->checkBoxCorr3);
            this->groupBox1->Controls->Add(this->checkBoxCorr1);
            this->groupBox1->Controls->Add(this->checkBoxCorr4);
            this->groupBox1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->groupBox1->Location = System::Drawing::Point(704, 566);
            this->groupBox1->Name = L"groupBox1";
            this->groupBox1->Size = System::Drawing::Size(446, 179);
            this->groupBox1->TabIndex = 40;
            this->groupBox1->TabStop = false;
            this->groupBox1->Text = L"Detectors";
            // 
            // textBoxXWidth
            // 
            this->textBoxXWidth->Enabled = false;
            this->textBoxXWidth->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxXWidth->Location = System::Drawing::Point(316, 717);
            this->textBoxXWidth->Margin = System::Windows::Forms::Padding(2);
            this->textBoxXWidth->Name = L"textBoxXWidth";
            this->textBoxXWidth->Size = System::Drawing::Size(65, 19);
            this->textBoxXWidth->TabIndex = 40;
            this->textBoxXWidth->Text = L"10";
            this->textBoxXWidth->Leave += gcnew System::EventHandler(this, &DcsAppForm::textBoxXWidth_Leave);
            // 
            // label4
            // 
            this->label4->AutoSize = true;
            this->label4->Enabled = false;
            this->label4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label4->Location = System::Drawing::Point(260, 719);
            this->label4->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->label4->Name = L"label4";
            this->label4->Size = System::Drawing::Size(52, 13);
            this->label4->TabIndex = 40;
            this->label4->Text = L"X Width";
            // 
            // label5
            // 
            this->label5->AutoSize = true;
            this->label5->Enabled = false;
            this->label5->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label5->Location = System::Drawing::Point(385, 719);
            this->label5->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->label5->Name = L"label5";
            this->label5->Size = System::Drawing::Size(54, 13);
            this->label5->TabIndex = 41;
            this->label5->Text = L"seconds";
            // 
            // label7
            // 
            this->label7->AutoSize = true;
            this->label7->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label7->Location = System::Drawing::Point(470, 719);
            this->label7->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->label7->Name = L"label7";
            this->label7->Size = System::Drawing::Size(68, 13);
            this->label7->TabIndex = 42;
            this->label7->Text = L"Y Min/Max";
            // 
            // textBoxYMin
            // 
            this->textBoxYMin->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxYMin->Location = System::Drawing::Point(540, 717);
            this->textBoxYMin->Margin = System::Windows::Forms::Padding(2);
            this->textBoxYMin->Name = L"textBoxYMin";
            this->textBoxYMin->Size = System::Drawing::Size(65, 19);
            this->textBoxYMin->TabIndex = 43;
            this->textBoxYMin->Text = L"0";
            this->textBoxYMin->Leave += gcnew System::EventHandler(this, &DcsAppForm::textBoxYMin_Leave);
            // 
            // textBoxYMax
            // 
            this->textBoxYMax->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxYMax->Location = System::Drawing::Point(605, 717);
            this->textBoxYMax->Margin = System::Windows::Forms::Padding(2);
            this->textBoxYMax->Name = L"textBoxYMax";
            this->textBoxYMax->Size = System::Drawing::Size(65, 19);
            this->textBoxYMax->TabIndex = 44;
            this->textBoxYMax->Text = L"1";
            this->textBoxYMax->Leave += gcnew System::EventHandler(this, &DcsAppForm::textBoxYMax_Leave);
            // 
            // checkBoxWaterfall
            // 
            this->checkBoxWaterfall->AutoSize = true;
            this->checkBoxWaterfall->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->checkBoxWaterfall->Location = System::Drawing::Point(14, 738);
            this->checkBoxWaterfall->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxWaterfall->Name = L"checkBoxWaterfall";
            this->checkBoxWaterfall->Size = System::Drawing::Size(77, 17);
            this->checkBoxWaterfall->TabIndex = 40;
            this->checkBoxWaterfall->Text = L"Waterfall";
            this->checkBoxWaterfall->UseVisualStyleBackColor = true;
            this->checkBoxWaterfall->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxWaterfall_Click);
            // 
            // textBoxCorrXMax
            // 
            this->textBoxCorrXMax->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxCorrXMax->Location = System::Drawing::Point(1073, 517);
            this->textBoxCorrXMax->Name = L"textBoxCorrXMax";
            this->textBoxCorrXMax->Size = System::Drawing::Size(70, 19);
            this->textBoxCorrXMax->TabIndex = 45;
            this->textBoxCorrXMax->Text = L"1";
            this->textBoxCorrXMax->Leave += gcnew System::EventHandler(this, &DcsAppForm::textBoxCorrXMax_Leave);
            // 
            // textBoxCorrXMin
            // 
            this->textBoxCorrXMin->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxCorrXMin->Location = System::Drawing::Point(1001, 517);
            this->textBoxCorrXMin->Name = L"textBoxCorrXMin";
            this->textBoxCorrXMin->Size = System::Drawing::Size(71, 19);
            this->textBoxCorrXMin->TabIndex = 46;
            this->textBoxCorrXMin->Text = L"0";
            this->textBoxCorrXMin->Leave += gcnew System::EventHandler(this, &DcsAppForm::textBoxCorrXMin_Leave);
            // 
            // labelCorrXMinMax
            // 
            this->labelCorrXMinMax->AutoSize = true;
            this->labelCorrXMinMax->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->labelCorrXMinMax->Location = System::Drawing::Point(932, 520);
            this->labelCorrXMinMax->Name = L"labelCorrXMinMax";
            this->labelCorrXMinMax->Size = System::Drawing::Size(68, 13);
            this->labelCorrXMinMax->TabIndex = 47;
            this->labelCorrXMinMax->Text = L"X Min/Max";
            // 
            // labelCorrYMinMax
            // 
            this->labelCorrYMinMax->AutoSize = true;
            this->labelCorrYMinMax->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->labelCorrYMinMax->Location = System::Drawing::Point(932, 546);
            this->labelCorrYMinMax->Name = L"labelCorrYMinMax";
            this->labelCorrYMinMax->Size = System::Drawing::Size(68, 13);
            this->labelCorrYMinMax->TabIndex = 50;
            this->labelCorrYMinMax->Text = L"Y Min/Max";
            // 
            // textBoxCorrYMin
            // 
            this->textBoxCorrYMin->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxCorrYMin->Location = System::Drawing::Point(1000, 543);
            this->textBoxCorrYMin->Name = L"textBoxCorrYMin";
            this->textBoxCorrYMin->Size = System::Drawing::Size(71, 19);
            this->textBoxCorrYMin->TabIndex = 49;
            this->textBoxCorrYMin->Text = L"0";
            this->textBoxCorrYMin->Leave += gcnew System::EventHandler(this, &DcsAppForm::textBoxCorrYMin_Leave);
            // 
            // textBoxCorrYMax
            // 
            this->textBoxCorrYMax->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxCorrYMax->Location = System::Drawing::Point(1073, 543);
            this->textBoxCorrYMax->Name = L"textBoxCorrYMax";
            this->textBoxCorrYMax->Size = System::Drawing::Size(70, 19);
            this->textBoxCorrYMax->TabIndex = 48;
            this->textBoxCorrYMax->Text = L"1";
            this->textBoxCorrYMax->Leave += gcnew System::EventHandler(this, &DcsAppForm::textBoxCorrYMax_Leave);
            // 
            // groupBoxLaserPower
            // 
            this->groupBoxLaserPower->Controls->Add(this->labelLaserPowerMillAmps);
            this->groupBoxLaserPower->Controls->Add(this->textBoxSetLaserPower);
            this->groupBoxLaserPower->Controls->Add(this->labelLaserPower);
            this->groupBoxLaserPower->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->groupBoxLaserPower->Location = System::Drawing::Point(931, 35);
            this->groupBoxLaserPower->Name = L"groupBoxLaserPower";
            this->groupBoxLaserPower->Size = System::Drawing::Size(216, 67);
            this->groupBoxLaserPower->TabIndex = 51;
            this->groupBoxLaserPower->TabStop = false;
            this->groupBoxLaserPower->Text = L"Laser Power";
            // 
            // labelLaserPowerMillAmps
            // 
            this->labelLaserPowerMillAmps->AutoSize = true;
            this->labelLaserPowerMillAmps->Location = System::Drawing::Point(180, 33);
            this->labelLaserPowerMillAmps->Name = L"labelLaserPowerMillAmps";
            this->labelLaserPowerMillAmps->Size = System::Drawing::Size(27, 15);
            this->labelLaserPowerMillAmps->TabIndex = 2;
            this->labelLaserPowerMillAmps->Text = L"mA";
            // 
            // textBoxSetLaserPower
            // 
            this->textBoxSetLaserPower->Location = System::Drawing::Point(105, 29);
            this->textBoxSetLaserPower->Name = L"textBoxSetLaserPower";
            this->textBoxSetLaserPower->Size = System::Drawing::Size(71, 21);
            this->textBoxSetLaserPower->TabIndex = 1;
            this->textBoxSetLaserPower->Leave += gcnew System::EventHandler(this, &DcsAppForm::textBoxSetLaserPower_Leave);
            // 
            // labelLaserPower
            // 
            this->labelLaserPower->AutoSize = true;
            this->labelLaserPower->Location = System::Drawing::Point(27, 32);
            this->labelLaserPower->Name = L"labelLaserPower";
            this->labelLaserPower->Size = System::Drawing::Size(79, 15);
            this->labelLaserPower->TabIndex = 0;
            this->labelLaserPower->Text = L"Set Current";
            // 
            // listBoxLaserPowerStatus
            // 
            this->listBoxLaserPowerStatus->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->listBoxLaserPowerStatus->FormattingEnabled = true;
            this->listBoxLaserPowerStatus->ItemHeight = 20;
            this->listBoxLaserPowerStatus->Location = System::Drawing::Point(931, 106);
            this->listBoxLaserPowerStatus->Name = L"listBoxLaserPowerStatus";
            this->listBoxLaserPowerStatus->Size = System::Drawing::Size(217, 44);
            this->listBoxLaserPowerStatus->TabIndex = 3;
            // 
            // labelElapsedTime
            // 
            this->labelElapsedTime->AutoSize = true;
            this->labelElapsedTime->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 18, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->labelElapsedTime->Location = System::Drawing::Point(584, 84);
            this->labelElapsedTime->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
            this->labelElapsedTime->Name = L"labelElapsedTime";
            this->labelElapsedTime->Size = System::Drawing::Size(190, 29);
            this->labelElapsedTime->TabIndex = 53;
            this->labelElapsedTime->Text = L"Elapsed Time: ";
            this->labelElapsedTime->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
            // 
            // textBoxElapsedTime
            // 
            this->textBoxElapsedTime->BackColor = System::Drawing::SystemColors::ButtonFace;
            this->textBoxElapsedTime->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->textBoxElapsedTime->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 18, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxElapsedTime->Location = System::Drawing::Point(782, 85);
            this->textBoxElapsedTime->Margin = System::Windows::Forms::Padding(2);
            this->textBoxElapsedTime->Name = L"textBoxElapsedTime";
            this->textBoxElapsedTime->RightToLeft = System::Windows::Forms::RightToLeft::No;
            this->textBoxElapsedTime->Size = System::Drawing::Size(112, 28);
            this->textBoxElapsedTime->TabIndex = 52;
            this->textBoxElapsedTime->Text = L"0.0";
            // 
            // checkBoxShowBFi
            // 
            this->checkBoxShowBFi->AutoSize = true;
            this->checkBoxShowBFi->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->checkBoxShowBFi->Location = System::Drawing::Point(22, 210);
            this->checkBoxShowBFi->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxShowBFi->Name = L"checkBoxShowBFi";
            this->checkBoxShowBFi->Size = System::Drawing::Size(79, 17);
            this->checkBoxShowBFi->TabIndex = 54;
            this->checkBoxShowBFi->Text = L"Show BFi";
            this->checkBoxShowBFi->UseVisualStyleBackColor = true;
            this->checkBoxShowBFi->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxShowBFi_Click);
            // 
            // menuStrip1
            // 
            this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->propertiesToolStripMenuItem });
            this->menuStrip1->Location = System::Drawing::Point(0, 0);
            this->menuStrip1->Name = L"menuStrip1";
            this->menuStrip1->Size = System::Drawing::Size(1169, 25);
            this->menuStrip1->TabIndex = 55;
            this->menuStrip1->Text = L"menuStrip1";
            // 
            // propertiesToolStripMenuItem
            // 
            this->propertiesToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
                this->lightTissuePropertiesToolStripMenuItem,
                    this->laserPowerToolStripMenuItem
            });
            this->propertiesToolStripMenuItem->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->propertiesToolStripMenuItem->Name = L"propertiesToolStripMenuItem";
            this->propertiesToolStripMenuItem->Size = System::Drawing::Size(66, 21);
            this->propertiesToolStripMenuItem->Text = L"Settings";
            // 
            // lightTissuePropertiesToolStripMenuItem
            // 
            this->lightTissuePropertiesToolStripMenuItem->Name = L"lightTissuePropertiesToolStripMenuItem";
            this->lightTissuePropertiesToolStripMenuItem->Size = System::Drawing::Size(209, 22);
            this->lightTissuePropertiesToolStripMenuItem->Text = L"Light/Tissue Properties";
            this->lightTissuePropertiesToolStripMenuItem->Click += gcnew System::EventHandler(this, &DcsAppForm::lightTissuePropertiesToolStripMenuItem_Click);
            // 
            // laserPowerToolStripMenuItem
            // 
            this->laserPowerToolStripMenuItem->Name = L"laserPowerToolStripMenuItem";
            this->laserPowerToolStripMenuItem->Size = System::Drawing::Size(209, 22);
            this->laserPowerToolStripMenuItem->Text = L"Laser Power";
            // 
            // checkBoxShowAux
            // 
            this->checkBoxShowAux->AutoSize = true;
            this->checkBoxShowAux->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 7.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->checkBoxShowAux->Location = System::Drawing::Point(22, 234);
            this->checkBoxShowAux->Margin = System::Windows::Forms::Padding(2);
            this->checkBoxShowAux->Name = L"checkBoxShowAux";
            this->checkBoxShowAux->Size = System::Drawing::Size(82, 17);
            this->checkBoxShowAux->TabIndex = 56;
            this->checkBoxShowAux->Text = L"Show Aux";
            this->checkBoxShowAux->UseVisualStyleBackColor = true;
            this->checkBoxShowAux->Click += gcnew System::EventHandler(this, &DcsAppForm::checkBoxShowAux_Click);
            // 
            // DcsAppForm
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(1169, 757);
            this->Controls->Add(this->checkBoxShowAux);
            this->Controls->Add(this->listBoxLaserPowerStatus);
            this->Controls->Add(this->checkBoxShowBFi);
            this->Controls->Add(this->labelElapsedTime);
            this->Controls->Add(this->textBoxElapsedTime);
            this->Controls->Add(this->groupBoxLaserPower);
            this->Controls->Add(this->labelCorrYMinMax);
            this->Controls->Add(this->textBoxCorrYMin);
            this->Controls->Add(this->textBoxCorrYMax);
            this->Controls->Add(this->labelCorrXMinMax);
            this->Controls->Add(this->textBoxCorrXMin);
            this->Controls->Add(this->textBoxCorrXMax);
            this->Controls->Add(this->checkBoxWaterfall);
            this->Controls->Add(this->textBoxYMax);
            this->Controls->Add(this->label7);
            this->Controls->Add(this->textBoxYMin);
            this->Controls->Add(this->label5);
            this->Controls->Add(this->label4);
            this->Controls->Add(this->textBoxXWidth);
            this->Controls->Add(this->groupBox1);
            this->Controls->Add(this->label1);
            this->Controls->Add(this->textBoxCorrEapsedTime);
            this->Controls->Add(this->checkBoxAutoZoom);
            this->Controls->Add(this->buttonPauseAcquisition);
            this->Controls->Add(this->checkBoxAux4);
            this->Controls->Add(this->checkBoxAux3);
            this->Controls->Add(this->checkBoxAux2);
            this->Controls->Add(this->checkBoxAux1);
            this->Controls->Add(this->pictureBox3);
            this->Controls->Add(this->pictureBox1);
            this->Controls->Add(this->buttonStartAcquisition);
            this->Controls->Add(this->buttonFpgaConfig);
            this->Controls->Add(this->comboBoxDevices);
            this->Controls->Add(this->groupBoxSaveData);
            this->Controls->Add(this->menuStrip1);
            this->MainMenuStrip = this->menuStrip1;
            this->Margin = System::Windows::Forms::Padding(2);
            this->Name = L"DcsAppForm";
            this->Text = L"DcsAppForm";
            this->groupBoxSaveData->ResumeLayout(false);
            this->groupBoxSaveData->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox3))->EndInit();
            this->groupBox1->ResumeLayout(false);
            this->groupBox1->PerformLayout();
            this->groupBoxLaserPower->ResumeLayout(false);
            this->groupBoxLaserPower->PerformLayout();
            this->menuStrip1->ResumeLayout(false);
            this->menuStrip1->PerformLayout();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion
    private:

        // ------------------------------------------------------------------------------------------------
        System::Void pictureBox1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
        {
            bfimult->disp->redrawGrid(e->Graphics, this->Font);
        }


        // ------------------------------------------------------------------------------------------------
        System::Void pictureBox3_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
        {
            corrmult->disp->redrawGrid(e->Graphics, this->Font);
        }


        // ------------------------------------------------------------------------------------------------
        System::Void comboBoxDevices_Click(System::Object^  sender, System::EventArgs^  e)
        {
            m_nDeviceIndex = comboBoxDevices->SelectedIndex;
        }


        // ------------------------------------------------------------------------------------------------
        System::Void buttonFpgaConfig_Click(System::Object^  sender, System::EventArgs^  e)
        {

            int nDeviceIndex = comboBoxDevices->SelectedIndex;
            if(nDeviceIndex == -1)
                return;

            usbdevice = new Fx3Device(nDeviceIndex, true);

            if(!usbdevice->IsConnected()) {
                MessageBox::Show(L"No device is connected for configuration");
                return;
            }

            if(usbdevice->IsStreamer()) {
                MessageBox::Show(L"Device already configured Streamer");
                return;
            }

            FPGAConfigForm^ form = gcnew FPGAConfigForm();
            form->ShowDialog();

            Cleanup(L"", usbdevice, NULL);
            AddDevicesToDropdownMenu();
        }



        // ------------------------------------------------------------------------------------------------
        System::Void radioButtonSaveRawData_Click(System::Object^  sender, System::EventArgs^  e)
        {
            // Turn Aucheck property off (to false) and toggle button manually
            if(radioButtonSaveRawData->Checked == false)
                radioButtonSaveRawData->Checked = true;
            else
                radioButtonSaveRawData->Checked = false;

            // State transition 1
            if((m_save == NoSaveNoReplay) && (radioButtonSaveRawData->Checked == true))
                m_save |= SaveRaw;
            // State transition 2
            else if((m_save == SaveRaw) && (radioButtonSaveRawData->Checked == false))
                m_save &= ~SaveRaw;
            // State transition 7
            else if((m_save == (SaveCorr|SaveAux)) && (radioButtonSaveRawData->Checked == true)) {
                m_save |= SaveRaw;
                m_save &= ~SaveAux;
            }
            // State transition 8
            else if((m_save == (SaveRaw|SaveCorr)) && (radioButtonSaveRawData->Checked == false)) {
                m_save &= ~SaveRaw;
                m_save |= SaveAux;
            }
            // State transition 9
            else if((m_save == Replay) && (radioButtonSaveRawData->Checked == true)) {
                m_save |= SaveRaw;
            }

            if((m_save & SaveRaw || m_save & SaveCorr || m_save & SaveAux) && (m_save & Replay)) {
                this->textBoxSessionFolder->Text = gcnew String(m_saveFileDataRaw->GetDirnameAcqFull());
                this->textBoxBaseFileName->Text = gcnew String(m_saveFileDataRaw->GetFilenameBase());
            }

            m_save &= ~Replay;
            radioButtonReplaySavedData->Checked = false;
            this->buttonPauseAcquisition->Visible = false;
        }




        // ------------------------------------------------------------------------------------------------
        System::Void radioButtonSaveCorrData_Click(System::Object^  sender, System::EventArgs^  e)
        {
            // Turn Aucheck property off (to false) and toggle button manually
            if(radioButtonSaveCorrData->Checked == false)
                radioButtonSaveCorrData->Checked = true;
            else
                radioButtonSaveCorrData->Checked = false;


            // State transition 3
            if((m_save == NoSaveNoReplay) && (radioButtonSaveCorrData->Checked == true)) {
                m_save |= SaveCorr;
                m_save |= SaveAux;
            }
            // State transition 4
            else if((m_save == (SaveCorr|SaveAux)) && (radioButtonSaveCorrData->Checked == false)) {
                m_save &= ~SaveCorr;
                m_save &= ~SaveAux;
            }
            // State transition 5
            else if((m_save == SaveRaw) && (radioButtonSaveCorrData->Checked == true)) {
                m_save |= SaveCorr;
                m_save &= ~SaveAux;
            }
            // State transition 6
            else if((m_save == (SaveCorr|SaveRaw)) && (radioButtonSaveCorrData->Checked == false)) {
                m_save &= ~SaveCorr;
                m_save &= ~SaveAux;
            }
            // State transition 12
            else if((m_save == Replay) && (radioButtonSaveCorrData->Checked == true)) {
                m_save |= SaveCorr;
                m_save |= SaveAux;
            }

            if((m_save & SaveRaw || m_save & SaveCorr || m_save & SaveAux) && (m_save & Replay)) {
                this->textBoxSessionFolder->Text = gcnew String(m_saveFileDataCorr->GetDirnameAcqFull());
                this->textBoxBaseFileName->Text = gcnew String(m_saveFileDataCorr->GetFilenameBase());
            }

            m_save &= ~Replay;
            radioButtonReplaySavedData->Checked = false;
            this->buttonPauseAcquisition->Visible = false;
        }


        // ------------------------------------------------------------------------------------------------
        System::Void radioButtonReplaySavedData_Click(System::Object^  sender, System::EventArgs^  e)
        {
            // Turn Aucheck property off (to false) and toggle button manually
            if(radioButtonReplaySavedData->Checked == false)
                radioButtonReplaySavedData->Checked = true;
            else
                radioButtonReplaySavedData->Checked = false;

            // State transition 13
            if((m_save == NoSaveNoReplay) && (radioButtonReplaySavedData->Checked == true)) {
                m_save |= Replay;
            }
            // State transition 14
            else if((m_save == Replay) && (radioButtonReplaySavedData->Checked == false)) {
                m_save &= ~Replay;
            }
            // State transition 10
            else if((m_save == SaveRaw) && (radioButtonReplaySavedData->Checked == true)) {
                m_save |= Replay;
            }
            // State transition 11
            else if((m_save == (SaveCorr|SaveAux)) && (radioButtonReplaySavedData->Checked == true)) {
                m_save |= Replay;
            }
            // State transition 15
            else if((m_save == (SaveRaw|SaveCorr)) && (radioButtonReplaySavedData->Checked == true)) {
                m_save |= Replay;
            }

            m_save &= ~SaveRaw;
            m_save &= ~SaveCorr;
            m_save &= ~SaveAux;
            radioButtonSaveRawData->Checked = false;
            radioButtonSaveCorrData->Checked = false;

            if(m_save & Replay) {
                this->textBoxSessionFolder->Text = gcnew String(m_replayFileData->GetDirnameAcqFull());
                this->textBoxBaseFileName->Text = gcnew String(m_replayFileData->GetFilenameBaseFull());
                this->buttonPauseAcquisition->Visible = true;
            }
            else
                this->buttonPauseAcquisition->Visible = false;
        }



        // ------------------------------------------------------------------------------------------------
        System::Void buttonBrowseFolders_Click(System::Object^  sender, System::EventArgs^  e)
        {
            FolderBrowserDialog^ folderDialog = gcnew FolderBrowserDialog();
            wchar_t*    dirname = new wchar_t[512]();

            folderDialog->SelectedPath = this->textBoxSessionFolder->Text;

            ::DialogResult result = folderDialog->ShowDialog();
            if(result == ::DialogResult::OK) {
                this->textBoxSessionFolder->Text = folderDialog->SelectedPath;

                convertStringToWchar(this->textBoxSessionFolder->Text, dirname);

                if(m_save & Replay) {
                    // Change replay folder ONLY if the replay option is selected.
                    m_replayFileData->SetDirnameAcqFull(dirname);
                }
                else {
                    // If no save/replay or the save options are selected then 
                    // change all save file folders to same dirname/filename
                    m_saveFileDataRaw->SetDirnameAcqFull(dirname);
                    m_saveFileDataCorr->SetDirnameAcqFull(dirname);
                    m_saveFileDataAux->SetDirnameAcqFull(dirname);
                }
            }
            delete dirname;
        }



        // ------------------------------------------------------------------------------------------------
        System::Void buttonBrowseFiles_Click(System::Object^  sender, System::EventArgs^  e)
        {
            OpenFileDialog^ fileDialog = gcnew OpenFileDialog;
            wchar_t* filename = new wchar_t[100]();
            wchar_t* dirname = new wchar_t[512]();

            fileDialog->InitialDirectory = this->textBoxSessionFolder->Text;
            fileDialog->Filter = "All files|*.*";
            fileDialog->FilterIndex = 2;
            fileDialog->RestoreDirectory = true;
            if(fileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
                String^ dirnameStr = Path::GetDirectoryName(fileDialog->FileName);

                this->textBoxBaseFileName->Text = fileDialog->SafeFileName;
                this->textBoxSessionFolder->Text = dirnameStr;

                convertStringToWchar(textBoxBaseFileName->Text, filename);
                convertStringToWchar(dirnameStr, dirname);

                if(m_save & Replay) {
                    // Change replay folder ONLY if the replay option is selected.
                    // Also since the replay file must already exist (unlike for
                    // the save options), we also set the replay folder where the file 
                    // is found.
                    m_replayFileData->SetDirnameAcqFull(dirname);
                    m_replayFileData->SetFilenameFull(filename);
                }
                else {
                    // If no save/replay or the save options are selected then 
                    // change all save file objects to same dirname/filename
                    m_saveFileDataRaw->SetDirnameAcqFull(dirname);
                    m_saveFileDataRaw->SetFilenameFull(filename);

                    m_replayFileData->SetDirnameAcqFull(dirname);
                    m_replayFileData->SetFilenameFull(filename);

                    m_saveFileDataAux->SetDirnameAcqFull(dirname);
                    m_saveFileDataAux->SetFilenameFull(filename);
                }
            }

            delete filename;
            delete dirname;
        }



        // ------------------------------------------------------------------------------------------------
        System::Void buttonStartAcquisition_Click(System::Object^  sender, System::EventArgs^  e)
        {

            if(this->buttonStartAcquisition->Text == L"START")
            {
                if((m_save & SaveRaw) | (m_save & SaveCorr) | (m_save & SaveAux))
                {
                    DriveInfo^ drive = gcnew DriveInfo(gcnew String(m_saveFileDataRaw->GetFilenameFull()));
                    if(drive->AvailableFreeSpace < 1000000)
                    {
                        MessageBoxShow(String::Format("ERROR {0}: Disk is full. Cannot save data. Please free up space and try again.", ERROR_DISK_FULL));
                        return;
                    }
                }

                // Start the data transfer.
                m_acquisitionInProgress = true;
                setAcquisitionGUIControls(L"START");

                m_count = 0;
                m_bufferWrap = false;

                // This is a technique to create thread safe threads with non static function.
                // Infor from MSDN site "How to: Make Thread-Safe Calls to Windows Forms Controls"
                Thread^ workerThread = gcnew Thread(gcnew ThreadStart(this, &DcsAppForm::CollectData));
                workerThread->Start();
            }
            else
            {
                // Stop the data transfer.
                setAcquisitionGUIControls(L"STOP");
                m_acquisitionInProgress = false;
            }
        }



        // ------------------------------------------------------------------------------------------------
        System::Void buttonPauseAcquisition_Click(System::Object^  sender, System::EventArgs^  e)
        {
            if(this->buttonPauseAcquisition->Text == L"PAUSE")
            {
                this->buttonPauseAcquisition->Text = L"RESUME";
                m_acquisitionPaused = true;
            }
            else
            {
                // Stop the data transfer.
                this->buttonPauseAcquisition->Text = L"PAUSE";
                m_acquisitionPaused = false;
            }

        }


        // ------------------------------------------------------------------------------------------------
        System::Void checkBoxAux1_Click(System::Object^  sender, System::EventArgs^  e)
        {
            aux[0]->active = checkBoxAux1->Checked;
            if(aux[0]->active)
                checkedplots_aux |= 0x00000001;
            else
                checkedplots_aux &= ~0x00000001;
        }


        // ------------------------------------------------------------------------------------------------
        System::Void checkBoxAux2_Click(System::Object^  sender, System::EventArgs^  e)
        {
            aux[1]->active = checkBoxAux2->Checked;
            if(aux[1]->active)
                checkedplots_aux |= 0x00000002;
            else
                checkedplots_aux &= ~0x00000002;
        }


        // ------------------------------------------------------------------------------------------------
        System::Void checkBoxAux3_Click(System::Object^  sender, System::EventArgs^  e)
        {
            aux[2]->active = checkBoxAux3->Checked;
            if(aux[2]->active)
                checkedplots_aux |= 0x00000004;
            else
                checkedplots_aux &= ~0x00000004;
        }


        // ------------------------------------------------------------------------------------------------
        System::Void checkBoxAux4_Click(System::Object^  sender, System::EventArgs^  e)
        {
            aux[3]->active = checkBoxAux4->Checked;
            if(aux[3]->active)
                checkedplots_aux |= 0x00000008;
            else
                checkedplots_aux &= ~0x00000008;
        }



        // ------------------------------------------------------------------------------------------------
        System::Void checkBoxBFi1_Click(System::Object^  sender, System::EventArgs^  e)
        {
            bfi[0]->active = checkBoxBFi1->Checked;
            if(bfi[0]->active)
                checkedplots_bfi |= 0x00000001;
            else
                checkedplots_bfi &= ~0x00000001;
        }

        // ------------------------------------------------------------------------------------------------
        System::Void checkBoxBFi2_Click(System::Object^  sender, System::EventArgs^  e)
        {
            bfi[1]->active = checkBoxBFi2->Checked;
            if(bfi[1]->active)
                checkedplots_bfi |= 0x00000002;
            else
                checkedplots_bfi &= ~0x00000002;
        }


        // ------------------------------------------------------------------------------------------------
        System::Void checkBoxBFi3_Click(System::Object^  sender, System::EventArgs^  e)
        {
            bfi[2]->active = checkBoxBFi3->Checked;
            if(bfi[2]->active)
                checkedplots_bfi |= 0x00000004;
            else
                checkedplots_bfi &= ~0x00000004;
        }


        // ------------------------------------------------------------------------------------------------
        System::Void checkBoxBFi4_Click(System::Object^  sender, System::EventArgs^  e)
        {
            bfi[3]->active = checkBoxBFi4->Checked;
            if(bfi[3]->active)
                checkedplots_bfi |= 0x00000008;
            else
                checkedplots_bfi &= ~0x00000008;
        }



        // ------------------------------------------------------------------------------------------------
        System::Void checkBoxCorr1_Click(System::Object^  sender, System::EventArgs^  e)
        {
            corr[0]->active = checkBoxCorr1->Checked;
            if(corr[0]->active)
                checkedplots_corr |= 0x00000001;
            else
                checkedplots_corr &= ~0x00000001;
        }



        // ------------------------------------------------------------------------------------------------
        System::Void checkBoxCorr2_Click(System::Object^  sender, System::EventArgs^  e)
        {
            corr[1]->active = checkBoxCorr2->Checked;
            if(corr[1]->active)
                checkedplots_corr |= 0x00000002;
            else
                checkedplots_corr &= ~0x00000002;
        }


        // ------------------------------------------------------------------------------------------------
        System::Void checkBoxCorr3_Click(System::Object^  sender, System::EventArgs^  e)
        {
            corr[2]->active = checkBoxCorr3->Checked;
            if(corr[2]->active)
                checkedplots_corr |= 0x00000004;
            else
                checkedplots_corr &= ~0x00000004;
        }


        // ------------------------------------------------------------------------------------------------
        System::Void checkBoxCorr4_Click(System::Object^  sender, System::EventArgs^  e)
        {
            corr[3]->active = checkBoxCorr4->Checked;
            if(corr[3]->active)
                checkedplots_corr |= 0x00000008;
            else
                checkedplots_corr &= ~0x00000008;
        }



        // --------------------------------------------------------------------------------------------
        System::Void textBoxSetLaserPower_Leave(System::Object^  sender, System::EventArgs^  e)
        {
            char           curr_old[20];
            char           curr_new[20];
            unsigned char  curr_new_u[20];
            wchar_t        statusmsg[100];

            memset(curr_old, 0, sizeof(curr_old));
            memset(curr_new, 0, sizeof(curr_new));
            memset(curr_new_u, 0, sizeof(curr_new_u));
            memset(statusmsg, 0, sizeof(statusmsg));

            if(String::IsNullOrEmpty(textBoxSetLaserPower->Text))
                return;

            // Get old and new laser power settings
            sprintf_s(curr_old, "%d", laserpower);
            convertStringToChar(textBoxSetLaserPower->Text, curr_new);
            for(int i=0; i<strlen(curr_new); i++)
                curr_new_u[i] = curr_new[i];

            // Get the USB Device Instance Going.
            Fx3Device* USBDevice = new Fx3Device(m_nDeviceIndex, false);
            if(!(USBDevice->IsConnected())) {
                listBoxLaserPowerStatus->Items->Add(gcnew String(statusmsg));
                return;
            }

            if(USBDevice->SetCurrent(curr_new_u, strlen_u(curr_new_u)) == 0)
                swprintf(statusmsg, L"Success setting laser power to %s mA", curr_new_u);
            else
                swprintf(statusmsg, L"Error: Failed to set laser power to %s mA", curr_new_u);
            listBoxLaserPowerStatus->Items->Insert(0, gcnew String(statusmsg));
        }



        // ------------------------------------------------------------------------------------------------
        int getActivePlots(enum PlotType type)
        {
            int kk=0;
            int nplots;
            int checkedplots;
            int* idx_active_plots;

            switch(type)
            {
                case AUX:
                    nplots = nplots_aux;
                    checkedplots = checkedplots_aux;
                    idx_active_plots = idx_active_plots_aux;
                    break;

                case BFI:
                    nplots = nplots_bfi;
                    checkedplots = checkedplots_bfi;
                    idx_active_plots = idx_active_plots_bfi;
                    break;

                case CORR:
                    nplots = nplots_corr;
                    checkedplots = checkedplots_corr;
                    idx_active_plots = idx_active_plots_corr;
                    break;

                    // case ALL:
            }

            for(int ii=0; ii<(nplots); ii++) {
                if((checkedplots & (1<<ii)) > 0) {
                    idx_active_plots[kk] = ii;
                    kk++;
                }
            }
            return kk;
        }



        // ------------------------------------------------------------------------------------------------
        System::Void checkBoxAutoZoom_Click(System::Object^  sender, System::EventArgs^  e)
        {
            if(checkBoxAutoZoom->Checked)
            {
                for(int ii=0; ii<nplots_aux; ii++)
                    aux[ii]->m_coord->plot.autozoom = true;
                for(int ii=0; ii<nplots_bfi; ii++)
                    bfi[ii]->m_coord->plot.autozoom = true;
            }
            else
            {
                for(int ii=0; ii<nplots_aux; ii++)
                    aux[ii]->m_coord->plot.autozoom = false;
                for(int ii=0; ii<nplots_bfi; ii++)
                    bfi[ii]->m_coord->plot.autozoom = false;
            }
        }



        // ------------------------------------------------------------------------------------------------
        bool CheckDeviceSpeed(String^ message)
        {
            String^ caption = "DCS Device might not be able receive data.";
            MessageBoxButtons buttons = MessageBoxButtons::OK;

            if(!usbdevice->IsConnected())
                return true;

            if(usbdevice->IsBootLoader())
                return true;

            if(!usbdevice->IsSuperSpeed())
            {
                if(this->InvokeRequired)
                {
                    MessageBoxDelegate^ d = gcnew MessageBoxDelegate(this, &DcsAppForm::CheckDeviceSpeed);
                    this->Invoke(d, gcnew array<Object^> { message });
                }
                else
                {
                    MessageBox::Show(this, message, caption, buttons, MessageBoxIcon::Warning,
                        MessageBoxDefaultButton::Button1, MessageBoxOptions::RtlReading);
                }
                return false;
            }
            return true;
        }



        // ------------------------------------------------------------------------------------------------
        void EnableGUIControls()
        {
            this->radioButtonReplaySavedData->Enabled = true;
            this->radioButtonSaveCorrData->Enabled = true;
            this->radioButtonSaveRawData->Enabled = true;
            this->buttonStartAcquisition->Enabled = true;
            this->buttonPauseAcquisition->Enabled = true;
        }



        // ------------------------------------------------------------------------------------------------
        bool AddDevicesToDropdownMenu()
        {
            usbdevice = new Fx3Device();
            int nDeviceCount = usbdevice->GetDeviceCount();
            long totalTransferSize = usbdevice->GetTotalTransferSize();
            wchar_t** devNames = usbdevice->GetDeviceNames();
            int nCount;
            array<String^>^ s = gcnew array<String^>(nDeviceCount);
            bool r = true;
            String^ message = "WARNING: Might not be able to acquire data from DCS device. This may be because the USB port it's connected to is not USB 3.0. Please make sure the USB port you're using is 3.0 or higher then restart GUI and DCS device.";

            for(nCount=0; nCount < nDeviceCount; nCount++) {
                s[nCount] = gcnew String(devNames[nCount]);
                comboBoxDevices->Items->Insert(nCount, s[nCount]);
            }
            if(nDeviceCount>0)
                comboBoxDevices->SelectedIndex = 0;

            if(!CheckDeviceSpeed(message))
                r == false;

            if(usbdevice->IsStreamer())
                EnableGUIControls();
            else if(!usbdevice->IsBootLoader())
                EnableGUIControls();

            m_parser = new DcsDeviceDataParser(totalTransferSize*PACKETS_PER_TRANSFER);
            m_corrdata = new CorrFunc*[m_parser->dataDCS->nCh];
            for(int ii=0; ii<m_parser->dataDCS->nCh; ii++)
                m_corrdata[ii] = new CorrFunc;

            delete usbdevice;
            usbdevice = NULL;

            return r;
        }



        // --------------------------------------------------------------------------------------------
        void textBoxDet1BFi_SetText(String^ text)
        {
            if(textBoxDet1BFi->InvokeRequired) {
                SetTextBoxDetDelegate^ d =
                    gcnew SetTextBoxDetDelegate(this, &DcsAppForm::textBoxDet1BFi_SetText);
                this->Invoke(d, gcnew array<String^> { text });
            }
            else
                textBoxDet1BFi->Text = text;
        }



        // --------------------------------------------------------------------------------------------
        void textBoxDet2BFi_SetText(String^ text)
        {
            if(textBoxDet2BFi->InvokeRequired) {
                SetTextBoxDetDelegate^ d =
                    gcnew SetTextBoxDetDelegate(this, &DcsAppForm::textBoxDet2BFi_SetText);
                this->Invoke(d, gcnew array<String^> { text });
            }
            else
                textBoxDet2BFi->Text = text;
        }



        // --------------------------------------------------------------------------------------------
        void textBoxDet3BFi_SetText(String^ text)
        {
            if(textBoxDet3BFi->InvokeRequired) {
                SetTextBoxDetDelegate^ d =
                    gcnew SetTextBoxDetDelegate(this, &DcsAppForm::textBoxDet3BFi_SetText);
                this->Invoke(d, gcnew array<String^> { text });
            }
            else
                textBoxDet3BFi->Text = text;
        }



        // --------------------------------------------------------------------------------------------
        void textBoxDet4BFi_SetText(String^ text)
        {
            if(textBoxDet4BFi->InvokeRequired) {
                SetTextBoxDetDelegate^ d =
                    gcnew SetTextBoxDetDelegate(this, &DcsAppForm::textBoxDet4BFi_SetText);
                this->Invoke(d, gcnew array<String^> { text });
            }
            else
                textBoxDet4BFi->Text = text;
        }


        // --------------------------------------------------------------------------------------------
        void textBoxDet1Count_SetText(String^ text)
        {
            if(textBoxDet1Count->InvokeRequired) {
                SetTextBoxDetDelegate^ d =
                    gcnew SetTextBoxDetDelegate(this, &DcsAppForm::textBoxDet1Count_SetText);
                this->Invoke(d, gcnew array<String^> { text });
            }
            else
                textBoxDet1Count->Text = text;
        }



        // --------------------------------------------------------------------------------------------
        void textBoxDet2Count_SetText(String^ text)
        {
            if(textBoxDet2Count->InvokeRequired) {
                SetTextBoxDetDelegate^ d =
                    gcnew SetTextBoxDetDelegate(this, &DcsAppForm::textBoxDet2Count_SetText);
                this->Invoke(d, gcnew array<String^> { text });
            }
            else
                textBoxDet2Count->Text = text;
        }



        // --------------------------------------------------------------------------------------------
        void textBoxDet3Count_SetText(String^ text)
        {
            if(textBoxDet3Count->InvokeRequired) {
                SetTextBoxDetDelegate^ d =
                    gcnew SetTextBoxDetDelegate(this, &DcsAppForm::textBoxDet3Count_SetText);
                this->Invoke(d, gcnew array<String^> { text });
            }
            else
                textBoxDet3Count->Text = text;
        }



        // --------------------------------------------------------------------------------------------
        void textBoxDet4Count_SetText(String^ text)
        {
            if(textBoxDet4Count->InvokeRequired) {
                SetTextBoxDetDelegate^ d =
                    gcnew SetTextBoxDetDelegate(this, &DcsAppForm::textBoxDet4Count_SetText);
                this->Invoke(d, gcnew array<String^> { text });
            }
            else
                textBoxDet4Count->Text = text;
        }



        // --------------------------------------------------------------------------------------------
        System::Void textBoxCorrXMin_Leave(System::Object^  sender, System::EventArgs^  e)
        {
            char str0_min[20];
            char str_minNew[20];
            char str_maxNew[20];
            LightIntensityData* dataDCS = m_parser->dataDCS;
            bool errflag = false;

            memset(str0_min, 0, sizeof(str0_min));
            memset(str_minNew, 0, sizeof(str_minNew));
            memset(str_maxNew, 0, sizeof(str_maxNew));

            lock.acquire(5000);
            convertStringToChar(this->textBoxCorrXMin->Text, str_minNew);
            double min_x = atof(str_minNew);
            convertStringToChar(this->textBoxCorrXMax->Text, str_maxNew);
            double max_x = atof(str_maxNew);
            lock.release();

            // Get current max value 
            sprintf(str0_min, "%0.2g", pow(10.0, corr[0]->m_coord->min.x));

            // Error check 
            if(str_minNew[0]==0)
                errflag = true;
            if(min_x >= max_x)
                errflag = true;

            if(errflag==true) {
                this->textBoxCorrXMin->Text = gcnew String(str0_min);
                return;
            }

            // Set the x axis min in the correlation display to the new time bin 
            MyVector min0(log10(min_x), corrmult->m_coord->min.y);
            MyVector max0(log10(max_x), corrmult->m_coord->max.y);
            for(int kk=0; kk<dataDCS->nCh; kk++)
            {
                corr[kk]->m_coord->ResetXMinMax(min0, max0);
                corr[kk]->disp->setDataCoord(corr[kk]->m_coord);
            }
            corrmult->m_coord->ResetXMinMax(min0, max0);
            corrmult->disp->clearDisplay(corrmult->m_coord);
        }



        // --------------------------------------------------------------------------------------------
        System::Void textBoxCorrXMax_Leave(System::Object^  sender, System::EventArgs^  e)
        {
            char str0_max[20];
            char str_minNew[20];
            char str_maxNew[20];
            LightIntensityData* dataDCS = m_parser->dataDCS;
            bool errflag = false;

            memset(str0_max, 0, sizeof(str0_max));
            memset(str_minNew, 0, sizeof(str_minNew));
            memset(str_maxNew, 0, sizeof(str_maxNew));

            lock.acquire(5000);
            convertStringToChar(this->textBoxCorrXMin->Text, str_minNew);
            double min_x = atof(str_minNew);
            convertStringToChar(this->textBoxCorrXMax->Text, str_maxNew);
            double max_x = atof(str_maxNew);
            lock.release();

            // Get current max value 
            sprintf(str0_max, "%0.2g", pow(10.0, corr[0]->m_coord->max.x));

            // Error check 
            if(str_maxNew[0]==0)
                errflag = true;
            if(max_x < m_corrdata[0]->tmean[1])
                errflag = true;
            if(min_x >= max_x)
                errflag = true;

            if(errflag==true) {
                this->textBoxCorrXMax->Text = gcnew String(str0_max);
                return;
            }

            // Set the x axis min in the correlation display to the new time bin 
            MyVector min0(log10(min_x), corrmult->m_coord->min.y);
            MyVector max0(log10(max_x), corrmult->m_coord->max.y);
            for(int kk=0; kk<dataDCS->nCh; kk++)
            {
                corr[kk]->m_coord->ResetXMinMax(min0, max0);
                corr[kk]->disp->setDataCoord(corr[kk]->m_coord);
            }
            corrmult->m_coord->ResetXMinMax(min0, max0);
            corrmult->disp->clearDisplay(corrmult->m_coord);
        }


        // --------------------------------------------------------------------------------------------
        System::Void textBoxCorrYMin_Leave(System::Object^  sender, System::EventArgs^  e)
        {
            char str0_min[20];
            char str_minNew[20];
            char str_maxNew[20];
            LightIntensityData* dataDCS = m_parser->dataDCS;
            bool errflag = false;

            memset(str0_min, 0, sizeof(str0_min));
            memset(str_minNew, 0, sizeof(str_minNew));
            memset(str_maxNew, 0, sizeof(str_maxNew));

            lock.acquire(5000);
            convertStringToChar(this->textBoxCorrYMin->Text, str_minNew);
            double min_y = atof(str_minNew);
            convertStringToChar(this->textBoxCorrYMax->Text, str_maxNew);
            double max_y = atof(str_maxNew);
            lock.release();

            // Get current max value 
            sprintf(str0_min, "%0.2f", corr[0]->m_coord->min.y);

            // Error check 
            if(str_minNew[0]==0)
                errflag = true;
            if(min_y >= max_y)
                errflag = true;

            if(errflag==true) {
                this->textBoxCorrYMin->Text = gcnew String(str0_min);
                return;
            }

            // Set the y axis min in the correlation display 
            MyVector min0(corrmult->m_coord->min.x, min_y);
            MyVector max0(corrmult->m_coord->max.x, max_y);
            for(int kk=0; kk<dataDCS->nCh; kk++)
            {
                corr[kk]->m_coord->ResetYMinMax(min0, max0);
                corr[kk]->disp->setDataCoord(corr[kk]->m_coord);
            }
            corrmult->m_coord->ResetYMinMax(min0, max0);
            corrmult->disp->clearDisplay(corrmult->m_coord);
        }


        // --------------------------------------------------------------------------------------------
        System::Void textBoxCorrYMax_Leave(System::Object^  sender, System::EventArgs^  e)
        {
            char str0_max[20];
            char str_minNew[20];
            char str_maxNew[20];
            LightIntensityData* dataDCS = m_parser->dataDCS;
            bool errflag = false;

            memset(str0_max, 0, sizeof(str0_max));
            memset(str_minNew, 0, sizeof(str_minNew));
            memset(str_maxNew, 0, sizeof(str_maxNew));

            lock.acquire(5000);
            convertStringToChar(this->textBoxCorrYMin->Text, str_minNew);
            double min_y = atof(str_minNew);
            convertStringToChar(this->textBoxCorrYMax->Text, str_maxNew);
            double max_y = atof(str_maxNew);
            lock.release();

            // Get current max value 
            sprintf(str0_max, "%0.2f", corr[0]->m_coord->max.y);

            // Error check 
            if(str_maxNew[0]==0)
                errflag = true;
            if(min_y >= max_y)
                errflag = true;

            if(errflag==true) {
                this->textBoxCorrYMax->Text = gcnew String(str0_max);
                return;
            }

            // Set the y axis min in the correlation display 
            MyVector min0(corrmult->m_coord->min.x, min_y);
            MyVector max0(corrmult->m_coord->max.x, max_y);
            for(int kk=0; kk<dataDCS->nCh; kk++)
            {
                corr[kk]->m_coord->ResetYMinMax(min0, max0);
                corr[kk]->disp->setDataCoord(corr[kk]->m_coord);
            }
            corrmult->m_coord->ResetYMinMax(min0, max0);
            corrmult->disp->clearDisplay(corrmult->m_coord);
        }


        // --------------------------------------------------------------------------------------------
        System::Void checkBoxWaterfall_Click(System::Object^  sender, System::EventArgs^  e)
        {
            m_waterfall = checkBoxWaterfall->Checked;
        }


        // --------------------------------------------------------------------------------------------
        System::Void textBoxYMin_Leave(System::Object^  sender, System::EventArgs^  e)
        {
            char* str = new char[20]();
            lock.acquire(5000);
            convertStringToChar(this->textBoxYMin->Text, str);
            pictureBox1_ymin = atof(str);
            lock.release();
        }


        // --------------------------------------------------------------------------------------------
        System::Void textBoxYMax_Leave(System::Object^  sender, System::EventArgs^  e)
        {
            char* str = new char[20]();
            lock.acquire(5000);
            convertStringToChar(this->textBoxYMax->Text, str);
            pictureBox1_ymax = atof(str);
            lock.release();
        }

        // --------------------------------------------------------------------------------------------
        System::Void textBoxXWidth_Leave(System::Object^  sender, System::EventArgs^  e)
        {
            lock.acquire(5000);
            char* str = new char[20]();

            convertStringToChar(this->textBoxXWidth->Text, str);
            pictureBox1_twidth = atof(str);
            lock.release();
        }



        // --------------------------------------------------------------------------------------------
        System::Void lightTissuePropertiesToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
        {
            double  mua = m_corrdata[0]->BFi_obj->getMua();
            double  musp = m_corrdata[0]->BFi_obj->getMusp();
            double  lamda = m_corrdata[0]->BFi_obj->getLamda();
            double* rho = new double[m_parser->dataDCS->nCh]();

            for(int ii=0; ii<m_parser->dataDCS->nCh; ii++)
                rho[ii] = m_corrdata[ii]->BFi_obj->getSeparation();

            form_lightTissProp = gcnew LightAndTissPropertiesForm::LightAndTissPropertiesForm(mua, musp, lamda, rho, m_parser->dataDCS->nCh);
            form_lightTissProp->ShowDialog();

            if(form_lightTissProp->cancelled())
                return;


            for(int ii=0; ii<m_parser->dataDCS->nCh; ii++)
            {
                m_corrdata[ii]->BFi_obj->setMua(form_lightTissProp->getMua());
                m_corrdata[ii]->BFi_obj->setMusp(form_lightTissProp->getMusp());
                m_corrdata[ii]->BFi_obj->setLamda(form_lightTissProp->getLamda());
                m_corrdata[ii]->BFi_obj->setSeparation(form_lightTissProp->getSeparation(ii));
            }
        }



        // --------------------------------------------------------------------------------------------
        void setAcquisitionGUIControls(String^ text)
        {
            // This function is used to set the graphics textBox from another thread

            // InvokeRequired required compares the thread ID of the  
            // calling thread to the thread ID of the creating thread.  
            // If these threads are different, it returns true.  
            if(this->buttonStartAcquisition->InvokeRequired)
            {
                StringArgReturningVoidDelegate^ d =
                    gcnew StringArgReturningVoidDelegate(this, &DcsAppForm::setAcquisitionGUIControls);
                this->Invoke(d, gcnew array<Object^> { text });
            }
            else
            {
                if(!String::Compare(text, L"START"))
                {
                    this->buttonStartAcquisition->Text = L"STOP";
                    this->buttonStartAcquisition->ForeColor = buttonStopForeColor;
                    this->buttonStartAcquisition->BackColor = buttonStopBackColor;
                    this->textBoxElapsedTime->ForeColor = textBoxElapsedTimeColor_AcqStart;
                    this->textBoxElapsedTime->Text = gcnew String(L"0.0");
                    this->labelElapsedTime->ForeColor = textBoxElapsedTimeColor_AcqStart;
                }
                else if(!String::Compare(text, L"STOP"))
                {
                    this->buttonStartAcquisition->Text = L"START";
                    this->buttonStartAcquisition->ForeColor = buttonStartForeColor;
                    this->buttonStartAcquisition->BackColor = buttonStartBackColor;
                    this->textBoxElapsedTime->ForeColor = textBoxElapsedTimeColor_AcqStop;
                    this->labelElapsedTime->ForeColor = textBoxElapsedTimeColor_AcqStop;
                }

            }
        }



        // --------------------------------------------------------------------------------------------
        void textBoxBaseFileName_SetText(String^ text)
        {
            // InvokeRequired required compares the thread ID of the  
            // calling thread to the thread ID of the creating thread.  
            // If these threads are different, it returns true.  
            if(this->textBoxBaseFileName->InvokeRequired)
            {
                StringArgReturningVoidDelegate^ d =
                    gcnew StringArgReturningVoidDelegate(this, &DcsAppForm::textBoxBaseFileName_SetText);
                this->Invoke(d, gcnew array<Object^> { text });
            }
            else
            {
                this->textBoxBaseFileName->Text = text;
            }
        }



        // --------------------------------------------------------------------------------------------
        void comboBoxDevices_GetIdx(int^ idx)
        {
            // This function is used to set the graphics textBox from another thread

            // InvokeRequired required compares the thread ID of the  
            // calling thread to the thread ID of the creating thread.  
            // If these threads are different, it returns true.  
            if(this->comboBoxDevices->InvokeRequired)
            {
                IntArgReturningVoidDelegate^ d =
                    gcnew IntArgReturningVoidDelegate(this, &DcsAppForm::comboBoxDevices_GetIdx);
                this->Invoke(d, gcnew array<Object^> { idx });
            }
            else
            {
                *idx = this->comboBoxDevices->SelectedIndex;
            }
        }



        // --------------------------------------------------------------------------------------------
        System::Void textBoxSessionFolder_Leave(System::Object^  sender, System::EventArgs^  e)
        {
            wchar_t* dirname = new wchar_t[512]();

            convertStringToWchar(textBoxSessionFolder->Text, dirname);

            if(m_save & Replay) {
                m_replayFileData->SetDirnameAcqFull(dirname);
            }
            else {
                m_saveFileDataRaw->SetDirnameAcqFull(dirname);
                m_saveFileDataCorr->SetDirnameAcqFull(dirname);
                m_saveFileDataAux->SetDirnameAcqFull(dirname);
            }
            delete  dirname;
        }



        // --------------------------------------------------------------------------------------------
        System::Void textBoxBaseFileName_Leave(System::Object^  sender, System::EventArgs^  e)
        {
            wchar_t* filename = new wchar_t[50]();
            wchar_t* ext = new wchar_t[20]();

            convertStringToWchar(textBoxBaseFileName->Text, filename);

            if(m_save & Replay)
                m_replayFileData->SetFilenameFull(filename);
            else if((m_save & SaveRaw) || (m_save & SaveCorr) || (m_save & SaveAux)) {
                m_saveFileDataRaw->SetFilenameBase(filename);
                m_saveFileDataCorr->SetFilenameBase(filename);
                m_saveFileDataAux->SetFilenameBase(filename);
            }
            delete  filename;
            delete  ext;
        }



        // --------------------------------------------------------------------------------------------
        System::Void comboBoxExpirationTime_DisplayMemberChanged(System::Object^  sender, System::EventArgs^  e)
        {
            char*   str = new char[10]();
            convertStringToChar(comboBoxExpirationTime->Text, str);
            if(str[0] != 0)
                m_expTime = atof(str);
            delete str;
        }



        // --------------------------------------------------------------------------------------------
        System::Void comboBoxExpirationTime_Leave(System::Object^  sender, System::EventArgs^  e)
        {
            // Don't allow user to change the expiration time during acquisition. 
            // Not yet anyway, until we figure out why it crashes when you do that
            if(m_acquisitionInProgress || m_acquisitionPaused)
                return;

            char*   str = new char[10]();
            convertStringToChar(comboBoxExpirationTime->Text, str);
            if(str[0] != 0)
                m_expTime = atof(str);
            delete str;
        }




        // --------------------------------------------------------------------------------------------            
        System::Void checkBoxShowBFi_Click(System::Object^  sender, System::EventArgs^  e)
        {
            if(checkBoxShowBFi->Checked) {
                m_toggleBFiAux = BFI;
                m_waterfall = false;
                checkBoxWaterfall->Checked = 0;
                checkBoxShowAux->Checked = 0;

                for(int ii=0; ii<m_parser->dataDCS->nCh; ii++)
                    bfi[ii]->SetVisible(m_toggleBFiAux);
                bfimult->SetVisible(m_toggleBFiAux);

                for(int ii=0; ii<m_parser->dataADC->nCh; ii++)
                    aux[ii]->SetVisible(m_toggleBFiAux);
                auxmult->SetVisible(m_toggleBFiAux);

                bfimult->disp->clearDisplay(bfimult->m_coord);
            }
        }



        // --------------------------------------------------------------------------------------------            
        System::Void checkBoxShowAux_Click(System::Object^  sender, System::EventArgs^  e) 
        {
            if(checkBoxShowAux->Checked) {
                m_toggleBFiAux = AUX;
                m_waterfall = true;
                checkBoxWaterfall->Checked = 1;
                checkBoxShowBFi->Checked = 0;

                for(int ii=0; ii<m_parser->dataADC->nCh; ii++)
                    aux[ii]->SetVisible(m_toggleBFiAux);
                auxmult->SetVisible(m_toggleBFiAux);

                for(int ii=0; ii<m_parser->dataDCS->nCh; ii++)
                    bfi[ii]->SetVisible(m_toggleBFiAux);
                bfimult->SetVisible(m_toggleBFiAux);

                auxmult->disp->clearDisplay(bfimult->m_coord);
            }
        }


        // --------------------------------------------------------------------------------------------
        void Cleanup(wchar_t* msg, void* dev, PUCHAR buffers)
        {
            String^ s = gcnew String(msg);
            if(!String::IsNullOrEmpty(s))
                MessageBox::Show(s);

            if(errstatus != ERROR_SUCCESS)
            {
                String^ s;
                if(errstatus == ERROR_DISK_FULL)
                    s = String::Format("ERROR {0}: Cannot save any more data. Hard drive is full. Free up disk space and try again.", errstatus);
                else
                    s = String::Format("ERROR {0}: Cannot save any more data", errstatus);

                MessageBoxShow(s);
                errstatus = ERROR_SUCCESS;
            }

            m_acquisitionInProgress = false;
            m_acquisitionPaused = false;

            if(buffers != NULL)
                delete buffers;

            // m_hFileDataRaw cleanup is handles in DataWriteRoutine

            if(m_replayFileData->IsValid())
                m_replayFileData->Close();

            if(m_saveFileDataRaw->IsValid())
                m_saveFileDataRaw->Close();

            if(m_saveFileDataCorr->IsValid())
                m_saveFileDataCorr->Close();

            if(m_saveFileDataAux->IsValid())
                m_saveFileDataAux->Close();

            if(m_simulateData->IsValid())
                m_simulateData->Close();


            setAcquisitionGUIControls(L"STOP");
        }


        // --------------------------------------------------------------------------------------------
        int GetFirstFilename(wchar_t* filename)
        {
            wchar_t** names = new wchar_t*[m_MaxFiles];
            wchar_t* search_path = new wchar_t[m_MaxNameSize];
            WIN32_FIND_DATA fd;
            HANDLE hFind;
            int nFiles = 0;

            convertStringToWchar(textBoxBaseFileName->Text, filename);

            wsprintf(search_path, L"%s/*.*", filename);
            hFind = FindFirstFile(search_path, &fd);

            for(int ii=0; ii<m_MaxFiles; ii++)
                names[ii] = new wchar_t[50]();

            if(hFind != INVALID_HANDLE_VALUE) {
                do {
                    // read all (real) files in current folder
                    // , delete '!' read other 2 default folder . and ..
                    if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        wsprintf(names[nFiles], fd.cFileName);
                        nFiles++;

                        if(nFiles>m_MaxFiles)
                            break;
                    }
                } while(FindNextFile(hFind, &fd));
                FindClose(hFind);
            }

            filename = names[0];

            // Free the memory allocated for local variables
            for(int ii=0; ii<m_MaxFiles; ii++)
                delete names[ii];
            delete names;
            delete search_path;

            return 0;
        }



        // --------------------------------------------------------------------------------------------
        int GetFilenamePrefixFromGui(wchar_t* prefix)
        {
            const wchar_t* r;
            wchar_t s[100];
            wchar_t* editstr = new wchar_t[m_MaxNameSize]();
            wchar_t* editstrP = editstr;

            convertStringToWchar(textBoxBaseFileName->Text, editstr);

            if(editstr[0]=='\0') {
                wsprintf(prefix, L"dcs");
                return 0;
            }

            memset(s, 0, sizeof(s));

            wsprintf(prefix, editstrP);

            if((r = wcsstr(editstrP, L"_run")) != NULL) {

                if(r[4]==0 || r[5]==0 || r[6]==0)
                    return 0;

                if(!isdigit(r[4]) || !isdigit(r[5]) || !isdigit(r[6]))
                    return 0;

                for(int ii=0; &editstrP[ii] != r; ii++)
                    s[ii] = editstrP[ii];

                wsprintf(prefix, s);
            }

            // Free the memory allocated for local variables
            delete editstr;

            return 0;
        }


        // --------------------------------------------------------------------------------------------
        int GetRunNumberFromGui()
        {
            wchar_t* r;
            char  nRunStr[4];
            int nRun=0;
            wchar_t* filename = new wchar_t[m_MaxNameSize]();

            convertStringToWchar(textBoxBaseFileName->Text, filename);

            if((r = wcsstr(filename, L"_run")) != NULL)
            {
                if(r[4]==0 || r[5]==0 || r[6]==0)
                    return 0;

                if(!isdigit(r[4]) || !isdigit(r[5]) || !isdigit(r[6]))
                    return 0;

                nRunStr[0] = r[4];
                nRunStr[1] = r[5];
                nRunStr[2] = r[6];
                nRun = atoi(nRunStr);

                return nRun;
            }

            // Free the memory allocated for local variables
            delete  filename;

            return 0;
        }



        // --------------------------------------------------------------------------------------------
        int GetNewRunNumber()
        {
            wchar_t* dirname = new wchar_t[m_MaxNameSize]();
            convertStringToWchar(textBoxSessionFolder->Text, dirname);

            if(!PathFileExists(dirname)) {
                m_nRun = 1;
            }
            else {
                wchar_t** names = new wchar_t*[m_MaxFiles];
                wchar_t* search_path = new wchar_t[m_MaxNameSize];
                WIN32_FIND_DATA fd;
                HANDLE hFind;
                int nFiles = 0;
                const wchar_t* r;
                wchar_t* s = new wchar_t[100]();
                char  nRunStr[4];
                int nRun = 0;

                wsprintf(search_path, L"%s/*.*", dirname);
                hFind = FindFirstFile(search_path, &fd);

                memset(nRunStr, 0, sizeof(nRunStr));

                for(int ii=0; ii<m_MaxFiles; ii++)
                    names[ii] = new wchar_t[50]();

                if(hFind != INVALID_HANDLE_VALUE) {
                    do {
                        // read all (real) files in current folder
                        // , delete '!' read other 2 default folder . and ..
                        if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                            wsprintf(names[nFiles], fd.cFileName);
                            nFiles++;

                            if(nFiles>=m_MaxFiles)
                                break;
                        }
                    } while(FindNextFile(hFind, &fd));
                    FindClose(hFind);
                }

                m_nRun = 1;

                // Check all the found files to determine run number
                for(int ii=0; ii<nFiles; ii++)
                {
                    wsprintf(s, names[ii]);
                    if((r = wcsstr(s, L"_run")) != NULL)
                    {
                        if((r[4]!=0) || (isdigit(r[4])))
                            nRunStr[0] = r[4];

                        if((r[5]!=0) || (isdigit(r[5])))
                            nRunStr[1] = r[5];

                        if((r[6]!=0) || (isdigit(r[6])))
                            nRunStr[2] = r[6];

                        if(nRunStr[0]==0)
                            continue;

                        nRun = atoi(nRunStr);
                        if(nRun >= m_nRun)
                            m_nRun = nRun + 1;
                    }
                }

                // Free the memory allocated for local variables
                for(int ii=0; ii<m_MaxFiles; ii++)
                    delete names[ii];
                delete names;
                delete search_path;
            }

            // Free the memory allocated for local variables
            delete  dirname;

            return 0;
        }



        // --------------------------------------------------------------------------------------------
        int GetNewFilename()
        {
            int nRun;
            wchar_t* prefix = new wchar_t[100]();
            wchar_t* filename = new wchar_t[m_MaxNameSize]();

            // Dtermine which run this is
            GetNewRunNumber();
            nRun = GetRunNumberFromGui();

            if(nRun==m_nRun) {
                // Convert GUI filename String to wide char
                convertStringToWchar(textBoxBaseFileName->Text, filename);
            }
            else {
                GetFilenamePrefixFromGui(prefix);
                if(m_nRun < 10)
                    wsprintf(filename, L"%s_run00%d", prefix, m_nRun);
                else if(m_nRun < 100)
                    wsprintf(filename, L"%s_run0%d", prefix, m_nRun);
                else
                    wsprintf(filename, L"%s_run%d", prefix, m_nRun);
            }

            m_saveFileDataRaw->SetFilenameBase(filename);
            m_saveFileDataCorr->SetFilenameBase(filename);
            m_saveFileDataAux->SetFilenameBase(filename);

            // Set the GUI text boxes 
            textBoxBaseFileName_SetText(gcnew String(filename));

            // Free the memory allocated for local variables
            delete prefix;
            delete filename;

            return 0;
        }



        // ---------------------------------------------------------------------------------------------
        void pause(int millsec)
        {
            // Give user chance to pause replay of data 
            if(m_acquisitionPaused && (m_save & Replay))
                while(m_acquisitionPaused==true)
                    Sleep(10);

            Sleep(millsec);
        }




        // ------------------------------------------------------------------------------------------------
        bool MessageBoxShow(String^ message)
        {
            String^ caption = message;
            MessageBoxButtons buttons = MessageBoxButtons::OK;

            if(this->InvokeRequired)
            {
                MessageBoxDelegate^ d = gcnew MessageBoxDelegate(this, &DcsAppForm::MessageBoxShow);
                this->Invoke(d, gcnew array<Object^> { message });
            }
            else
            {
                MessageBox::Show(this, message, caption, buttons, MessageBoxIcon::Warning,
                    MessageBoxDefaultButton::Button1, MessageBoxOptions::RtlReading);
            }
            return true;
        }



        // --------------------------------------------------------------------------------------------
        void DataWriteRoutine()
        {
            PCY_DATA_BUFFER qPtr = qHead;
            PCY_DATA_BUFFER qPtrPrev;
            DWORD dwWritten = 0;

            if(qPtr == NULL)
            {
                // Make sure link list head is initialized...........
                while(qHead == NULL)
                    m_hDataQueueEvent->WaitOne();

                // Wait till the two datas are available to write.
                if(qHead->next == NULL)
                    m_hDataQueueEvent->WaitOne();

                if(qPtr == NULL)
                    qPtr = qHead;
            }

            /////////////// Let's start the data write loop /////////////////////////////////
            while(m_acquisitionInProgress == true)
            {
                m_hDataQueueEvent->WaitOne();

                while(qPtr != qTail)
                {
                    Monitor::Enter(_critSect);
                    if(qPtr != NULL)
                    {
                        // Write the device data to the file
                        if(!WriteFile(m_saveFileDataRaw->GetHandle(), qPtr->buffer, qPtr->length, &dwWritten, NULL))
                        {
                            errstatus = GetLastError();
                            m_acquisitionInProgress = false;
                            Monitor::Exit(_critSect);
                            break;
                        }

                        // Pop queue
                        qPtrPrev = qPtr;
                        qPtr = qPtr->next;
                        qHead = qPtr;
                        delete qPtrPrev;
                    }
                    Monitor::Exit(_critSect);
                }
            }

            Monitor::Enter(_critSect);
            while(qHead != NULL)
            {
                qPtrPrev = qHead;
                qHead = qHead->next;
                delete qPtrPrev;
            }
            Monitor::Exit(_critSect);

            Sleep(2*1000);
            m_saveFileDataRaw->Close();
            qHead = NULL;
            qTail = NULL;
        }



        // --------------------------------------------------------------------------------------------
        void CollectData()
        {
            // Allocate the arrays needed for queueing
            PUCHAR			    buffers = NULL;
            unsigned short*     buffersPtr;
            int                 nCount = 0;
            long                BytesXferred = 0;
            int                 incr = (int)aux[0]->m_dIncrGen;
            int                 jj;
            double              elapsedTimePrev = 0;
            wchar_t*            errMsg = NULL;
            long                totalTransferSize;
            enum InputSrc       inputsrc = None;
            double				expTime = FLT_MAX;
            wchar_t*            str = new wchar_t[50]();
            String^             message = "ERROR: Not able to acquire data from DCS device. This may be because the USB port it's connected to is not USB 3.0. Please make sure the USB port you're using is 3\.0 or higher then restart GUI and DCS device\.";
            int                 tFactor;
            DWORD				nRead;
            MyFile^             inputFileData;
            struct tm           timeinfo;
            unsigned short*     datetime = new unsigned short[8]();
            DWORD               dwWrite = 0;

            qHead = NULL;
            qTail = NULL;

            if((m_save & SaveRaw) || (m_save & SaveCorr) || (m_save & SaveAux))
                expTime = m_expTime;

            if(m_save & Replay)
                inputFileData = m_replayFileData;
            else
                inputFileData = m_simulateData;


            usbdevice = new Fx3Device(m_nDeviceIndex, true);
            tFactor = usbdevice->GetClockSpeed() / usbdevice->GetDataRate();

            memset(aux[0]->m_data, 0, BUF_SIZE*sizeof(MyVector));
            memset(aux[1]->m_data, 0, BUF_SIZE*sizeof(MyVector));
            m_parser->ClearBuffersAll();

            // Precalculate x for correlation curve
            for(int ii=0; ii < m_parser->dataDCS->nCh; ii++) {
                bfi[ii]->ClearBuffers();
                corr[ii]->ClearBuffers();
                for(jj=0; jj < m_corrdata[0]->nbins; jj++)
                    corr[ii]->m_coord->plot.data[jj].x = log10(m_corrdata[0]->tmean[jj]);
            }


            // Determine source of data
            if(usbdevice->IsStreamer() && !(m_save & Replay)) {
                usbdevice->SendStartNotification();
                totalTransferSize = usbdevice->GetTotalTransferSize();
                if(CheckDeviceSpeed(message))
                    inputsrc = DeviceSuperSpeed;
                else
                    inputsrc = DeviceNormalSpeed;
            }
            else {
                if((inputFileData->OpenForReading()) != NULL) {
                    totalTransferSize = m_maxPktSizeDefault * PACKETS_PER_TRANSFER;
                    inputsrc = File_;
                }
            }

            if(inputsrc == None) {
                Cleanup(L"Error: No data source selected. ", usbdevice, buffers);
                return;
            }

            // At present cannot acquire data at NormalSpeed. Need to be SuperSpeed
            if(inputsrc == DeviceNormalSpeed) {
                Cleanup(L"", usbdevice, buffers);
                return;
            }

            // Open files for saving data if save was selected
            if(m_save & SaveRaw) {
                if(m_saveFileDataRaw->OpenForWriting() < 0)
                {
                    Cleanup(L"", usbdevice, buffers);
                    return;
                }
            }
            if(m_save & SaveCorr) {
                if(m_saveFileDataCorr->OpenForWriting() < 0)
                {
                    Cleanup(L"", usbdevice, buffers);
                    return;
                }
            }
            if(m_save & SaveAux) {
                if(m_saveFileDataAux->OpenForWriting() < 0)
                {
                    Cleanup(L"", usbdevice, buffers);
                    return;
                }
            }

            buffers = new UCHAR[totalTransferSize];
            buffersPtr = (unsigned short*)buffers;


            // Get the start time of the measurement. This won't change 
            // until the current thread termintas signaling the end of the 
            // measurement. 
            acqStartTime = getCurrTime(NULL);
            acqElapsedTime = 0;

            // Now that we know we have a data source, launch the monitor threads.
            Thread^ workerThread2 = gcnew Thread(gcnew ThreadStart(this, &DcsAppForm::CorrDataMonitor));
            workerThread2->Start();
            Thread^ workerThread3 = gcnew Thread(gcnew ThreadStart(this, &DcsAppForm::BloodFlowMonitor));
            workerThread3->Start();
            Thread^ workerThread1 = gcnew Thread(gcnew ThreadStart(this, &DcsAppForm::AnalogDataMonitor));
            workerThread1->Start();

            if(m_saveFileDataRaw->IsValid())
            {
                Thread^ th = gcnew Thread(gcnew ThreadStart(this, &DcsAppForm::DataWriteRoutine));
                th->Start();
            }

            // Get and save start of measurement date and time 
            getCurrDateTime(&timeinfo, datetime);
            if(m_saveFileDataRaw->IsValid()) {
                WriteFile(m_saveFileDataRaw->GetHandle(), datetime, 8*sizeof(unsigned short), &dwWrite, NULL);
            }

            ////////////////////////////////////////////////////////////////////////
            // Start the data collection phase..................
            ///////////////////////////////////////////////////////////////////////
            memset(buffers, 0x00, totalTransferSize);
            while(m_acquisitionInProgress == true)
            {
                // Get data from whichever src has data available
                if(inputsrc & DeviceAny) {
                    // Get data from device 
                    if(usbdevice->RecieveData(buffers) < 0) {
                        errMsg = usbdevice->GetErrorMsg();
                        break;
                    }
                }
                else {
                    // Read the data from the file 
                    if(ReadFile(inputFileData->GetHandle(), buffers, totalTransferSize, &nRead, NULL) == 0)
                        break;

                    if(nRead == 0)
                        break;

                    pause(30);
                }

                BytesXferred += totalTransferSize;

                // BytesXFerred is needed for current data rate calculation.
                if(BytesXferred < 0) // Rollover - reset counters
                    BytesXferred = 0;

                // Parse data into separate data streams. Tell parser how big the raw data buffer 
                // is (buffersPtr) so it knows how much to allocate for it's own data which holds 
                // the counts. 
                m_parser->parseData(buffersPtr, totalTransferSize / sizeof(unsigned short));

                // Copy data into display buffer
                for(int jj=0; jj < (int)m_parser->dataADC->countTotal; jj+=incr)
                {
                    // for(int kk=0; kk < m_parser->nCh_adc; kk++)
                    if(m_count > 0)
                    {
                        for(int kk=0; kk<m_parser->nCh_adc; kk++)
                            aux[kk]->m_data[m_count].x = (double)(m_parser->dataADC->t[jj] / tFactor);
                    }
                    for(int kk=0; kk<m_parser->nCh_adc; kk++)
                        aux[kk]->m_data[m_count].y = (double)(m_parser->dataADC->amp[kk][jj]);

                    m_count++;

                    if(m_count >= BUF_SIZE)
                    {
                        memset(aux[0]->m_data, 0, BUF_SIZE*sizeof(MyVector));
                        memset(aux[1]->m_data, 0, BUF_SIZE*sizeof(MyVector));
                        m_count = 0;
                        m_bufferWrap = true;
                    }
                }
                m_parser->dataADC->countTotal = 0;

                ///////////////////////////////////////////////////////////
                /// Time to write the data to the file, if needed /////////
                ///////////////////////////////////////////////////////////
                if(m_saveFileDataRaw->IsValid())
                {
                    ///////////File writting is valid, so push the data to the file.////////////
                    //// Is this device super speed device?
                    if(inputsrc & DeviceSuperspeedOrFile) {
                        ///////////////////////////////////////////////////////
                        /// Don't block the data transfer by write here.
                        /// Let's push the data to a link list and 
                        /// the disk write thread will push the data to the file.
                        /////////////////////////////////////////////////////

                        Monitor::Enter(_critSect);

                        // Create new queue entry
                        CY_DATA_BUFFER *qEntryNew = new CY_DATA_BUFFER;
                        qEntryNew->next = NULL;
                        qEntryNew->buffer = buffers;
                        qEntryNew->length = totalTransferSize;

                        // Push new queue entry to queue
                        if(qHead == NULL) {
                            qHead = qEntryNew;
                            qTail = qEntryNew;
                        }
                        else {
                            qTail->next = qEntryNew;
                            qTail = qEntryNew;
                        }

                        // Send signal to writer thread that we have N new buffers
                        m_hDataQueueEvent->Set();

                        Monitor::Exit(_critSect);

                        ///////////////////Link List Population completes///////////
                    }
                    else {
                        ///// For non super speed devices, write the data. We have lots of time.
                        WriteFile(m_saveFileDataRaw->GetHandle(), buffers, totalTransferSize, &dwWrite, NULL);
                    }
                }
                else if(m_saveFileDataAux->IsValid())
                {
                    int nCh = m_parser->dataADC->nCh;
                    unsigned long count = m_parser->dataADC->count;
                    unsigned long long* t = m_parser->dataADC->t;
                    unsigned long long** amp = m_parser->dataADC->amp;

                    ///// For non super speed devices, write the data. We have lots of time.

                    if(m_parser->dataADC->countTotal == count)
                        WriteFile(m_saveFileDataAux->GetHandle(), &nCh, sizeof(nCh), &dwWrite, NULL);
                    WriteFile(m_saveFileDataAux->GetHandle(), &count, sizeof(count), &dwWrite, NULL);
                    WriteFile(m_saveFileDataAux->GetHandle(), t, count*sizeof(t[0]), &dwWrite, NULL);
                    for(int ii=0; ii < nCh; ii++)
                        WriteFile(m_saveFileDataAux->GetHandle(), amp[ii], count*sizeof(amp[ii][0]), &dwWrite, NULL);
                }

                // Get elapsed time every 100 data transfers
                acqElapsedTime = getElapsedTime(acqStartTime, 1e3);
                if((acqElapsedTime - elapsedTimePrev) >= 1.0)
                {
                    // Display elapsed time every half second
                    elapsedTimePrev = acqElapsedTime;
                    swprintf(str, L"%0.1f", round(acqElapsedTime));
                    textBoxElapsedTime_SetText(gcnew String(str));
                    BytesXferred = 0;
                }

                // Check if elapsed time has exceeded the expiration time 
                if(acqElapsedTime > expTime)
                    m_acquisitionInProgress = false;

                nCount++;
            }

            // Get and save end of measurement date and time 
            getCurrDateTime(&timeinfo, datetime);
            if(m_saveFileDataRaw->IsValid()) {
                WriteFile(m_saveFileDataRaw->GetHandle(), datetime, 8*sizeof(unsigned short), &dwWrite, NULL);
            }

            ///////////////////////////////////////////////////////////////
            // Alright, we out of data collection loop./////////
            // Stop can happen from User pressing the button or
            // Time out can trigger the exit. //////////////////

            ////////////////////////////////////////////////////////////////////////////
            /////// Send stop notification to the device //////////////////////////////
            ////////////////////////////////////////////////////////////////////////////
            if(inputsrc & DeviceAny) {
                usbdevice->SendStopNotification();
            }

            if(m_save & SaveRaw || m_save & SaveCorr || m_save & SaveAux) {
                // Increment run for new filename 
                GetNewFilename();

                swprintf(str, L"%0.1f", (abs(m_expTime-acqElapsedTime) < 1)? m_expTime : round(acqElapsedTime));
                textBoxElapsedTime_SetText(gcnew String(str));
            }

            delete str;

            // Perform Memory cleanup now
            Cleanup(errMsg, usbdevice, buffers);
    }



    // --------------------------------------------------------------------------------------------
    void textBoxCorrEapsedTime_SetText(String^ text)
    {
        // InvokeRequired required compares the thread ID of the  
        // calling thread to the thread ID of the creating thread.  
        // If these threads are different, it returns true.  
        if(this->textBoxCorrEapsedTime->InvokeRequired)
        {
            StringArgReturningVoidDelegate^ d =
                gcnew StringArgReturningVoidDelegate(this, &DcsAppForm::textBoxCorrEapsedTime_SetText);
            this->Invoke(d, gcnew array<Object^> { text });
        }
        else
        {
            this->textBoxCorrEapsedTime->Text = text;
        }
    }



    // --------------------------------------------------------------------------------------------
    void textBoxElapsedTime_SetText(String^ text)
    {
        // InvokeRequired required compares the thread ID of the  
        // calling thread to the thread ID of the creating thread.  
        // If these threads are different, it returns true.  
        if(this->textBoxElapsedTime->InvokeRequired)
        {
            StringArgReturningVoidDelegate^ d =
                gcnew StringArgReturningVoidDelegate(this, &DcsAppForm::textBoxElapsedTime_SetText);
            this->Invoke(d, gcnew array<Object^> { text });
        }
        else
        {
            this->textBoxElapsedTime->Text = text;
        }
    }




    // --------------------------------------------------------------------------------------------
    int DisplayCorrCurve(unsigned long long* startTime)
    {
        wchar_t* strData = new wchar_t[100]();
        wchar_t* strBFi = new wchar_t[100]();
        int nChDlg = (sizeof(m_chDlg_rate) / sizeof(int));
        LightIntensityData* dataDCS = m_parser->dataDCS;
        int buffsize = dataDCS->maxsize - (dataDCS->maxsize / 100);
        int countTotal[50];
        int countTotalDropped[50];
        int countTotalAll[50];
        double unit = 1;
        double elapsedTime = getElapsedTime(*startTime, unit);
        int nbytestowrite = sizeof(m_corrdata[0]->gmean[0]) * m_corrdata[0]->nbins;
        DWORD nbyteswritten;
        unsigned long long corrStartTime;
        double corrElapsedTime;
        int numChBFi = 0;

        if(elapsedTime >= 500.0)
        {
            MyCoord** p = new MyCoord*[dataDCS->nCh];
            int jj;
            double unit = 1;

            memcpy(countTotal, dataDCS->countTotal, dataDCS->nCh * sizeof(int));
            memcpy(countTotalDropped, dataDCS->countTotalDropped, dataDCS->nCh * sizeof(int));
            for(int ii=0; ii<dataDCS->nCh; ii++) {
                countTotalAll[ii] = countTotal[ii] + countTotalDropped[ii];
                p[ii] = corr[ii]->m_coord;
            }
            int elapsedTimeInSec2 = getElapsedTime(*startTime, 1);

            // Generate correlation data
            corrStartTime = getCurrTime(NULL);
            for(int ii=0; ii < dataDCS->nCh; ii++)
            {
                m_corrdata[ii]->calcAutocorrCurve(dataDCS->arrtimes[ii][0], countTotal[ii]);
                m_corrdata[ii]->calcBloodFlowIndex(&(m_BFi[ii]), &(m_beta[ii]), &(m_err[ii]));
            }
            corrElapsedTime = getElapsedTime(corrStartTime, unit);

            // Display time elapsed since start of last correlation
            swprintf(strData, L"%0.3f", corrElapsedTime / 1000.0);
            textBoxCorrEapsedTime_SetText(gcnew String(strData));

            // This function is used to set the graphics textBox from another thread
            corrmult->disp->clearDisplay(p[0]);

            // Display photon count 
            wsprintf(strData, L"%d", (int)(((double)countTotalAll[0] / 1000.0) / (elapsedTime * unit / 1000.0)));
            textBoxDet1Count_SetText(gcnew String(strData));
            wsprintf(strData, L"%d", (int)(((double)countTotalAll[1] / 1000.0) / (elapsedTime * unit / 1000.0)));
            textBoxDet2Count_SetText(gcnew String(strData));
            wsprintf(strData, L"%d", (int)(((double)countTotalAll[2] / 1000.0) / (elapsedTime * unit / 1000.0)));
            textBoxDet3Count_SetText(gcnew String(strData));
            wsprintf(strData, L"%d", (int)(((double)countTotalAll[3] / 1000.0) / (elapsedTime * unit / 1000.0)));
            textBoxDet4Count_SetText(gcnew String(strData));

            // Display BFi and beta
            if(countTotal[0] > 1000 && m_BFi[0] > 0 && m_beta[0] > 0)
                swprintf(strBFi, L"%0.3g,   %0.3f ", m_BFi[0], m_beta[0]);
            else
                swprintf(strBFi, L"0.0,  0.0");
            textBoxDet1BFi_SetText(gcnew String(strBFi));

            if(countTotal[1] > 1000 && m_BFi[1] > 0 && m_beta[1] > 0)
                swprintf(strBFi, L"%0.3g,   %0.3f ", m_BFi[1], m_beta[1]);
            else
                swprintf(strBFi, L"0.0,  0.0");
            textBoxDet2BFi_SetText(gcnew String(strBFi));

            if(countTotal[2] > 1000 && m_BFi[2] > 0 && m_beta[2] > 0)
                swprintf(strBFi, L"%0.3g,   %0.3f ", m_BFi[2], m_beta[2]);
            else
                swprintf(strBFi, L"0.0,  0.0");
            textBoxDet3BFi_SetText(gcnew String(strBFi));

            if(countTotal[3] > 1000 && m_BFi[3] > 0 && m_beta[3] > 0)
                swprintf(strBFi, L"%0.3g,   %0.3f ", m_BFi[3], m_beta[3]);
            else
                swprintf(strBFi, L"0.0,  0.0");
            textBoxDet4BFi_SetText(gcnew String(strBFi));

            for(int ii=0; ii<dataDCS->nCh; ii++)
            {
                if(countTotal[ii] > 0)
                {
                    if(checkedplots_corr & (1<<ii))
                    {
                        for(jj=0; jj < m_corrdata[0]->nbins; jj++)
                            p[ii]->plot.data[jj].y = m_corrdata[ii]->gmean[jj];
                        corrmult->disp->plotData(p[ii], jj-1, ii);
                    }

                    // Reset photon counts
                    dataDCS->countTotal[ii] = 0;
                }
                dataDCS->countTotalDropped[ii] = 0;

                if(m_saveFileDataCorr->IsValid())
                    WriteFile(m_saveFileDataCorr->GetHandle(), m_corrdata[ii]->gmean, nbytestowrite, &nbyteswritten, NULL);
            }


            // Reset start time
            *startTime = getCurrTime(NULL);

            delete p;
        }

        delete strData;
        delete strBFi;

        return 0;
    }



    // --------------------------------------------------------------------------------------------
    void CorrDataMonitor()
    {
        // Allocate the arrays needed for queueing
        int nChDlg = (sizeof(m_chDlg_rate) / sizeof(int));
        int nbytestowrite = sizeof(m_corrdata[0]->tmean[0]) * m_corrdata[0]->nbins;
        DWORD nbyteswritten;
        String^ strBFi = gcnew String("");

        Thread^ workerThread = gcnew Thread(gcnew ThreadStart(this, &DcsAppForm::GenerateBloodFlowIndex));
        workerThread->Start();


        // Initialize BFi and beta display to all zeros
        for(int ii=0; ii < m_parser->dataDCS->nCh; ii++) {
            bfi[ii]->Reset();
        }
        textBoxDet1Count_SetText(gcnew String(L"0"));
        textBoxDet2Count_SetText(gcnew String(L"0"));
        textBoxDet3Count_SetText(gcnew String(L"0"));
        textBoxDet4Count_SetText(gcnew String(L"0"));

        // If user chose to save the data, then write the file header: 
        //    a) Size of function
        //    b) Number of channels
        //    c) Delays 
        if(m_saveFileDataCorr->IsValid())
        {
            WriteFile(m_saveFileDataCorr->GetHandle(), &(m_corrdata[0]->nbins), sizeof(int), &nbyteswritten, NULL);
            WriteFile(m_saveFileDataCorr->GetHandle(), &(m_parser->dataDCS->nCh), sizeof(int), &nbyteswritten, NULL);
            WriteFile(m_saveFileDataCorr->GetHandle(), m_corrdata[0]->tmean, nbytestowrite, &nbyteswritten, NULL);
        }

        // Get start time and then display correlation curve every few milliseconds
        unsigned long long startTime = getCurrTime(NULL);
        while(m_acquisitionInProgress)
        {
            // Give user chance to pause and resume a trace replay 

            DisplayCorrCurve(&startTime);
            pause(10);
        }
        corrmult->disp->clearDisplay(corrmult->m_coord);
    }



    // --------------------------------------------------------------------------------------------
    void GenerateBloodFlowIndex()
    {
        while(m_acquisitionInProgress)
        {
            for(int ii=0; ii<m_parser->dataDCS->nCh; ii++)
                bfi[ii]->AddDataItem(m_BFi[ii]);
            bfimult->AddDataItem(m_BFi[0]);
            Sleep((DWORD)(1/bfi[0]->m_dRatePlot * 1000));
        }
    }



    // --------------------------------------------------------------------------------------------
    void UpdateAuxBfiAxesParams(DataStreamPlot^ datastream, int size)
    {
        int nplotsactive = getActivePlots(BFI);
        bool updateflag = false;

        lock.acquire(5000);
        if(((datastream->disp->plottype & WATERFALL) >> 5)  != m_waterfall)
            if(datastream->disp->multplots)
                updateflag = true;
        if(bfi[0]->m_tWidth != pictureBox1_twidth)
            updateflag = true;
        if(bfi[0]->m_coord->min.y != pictureBox1_ymin)
            updateflag = true;
        if(bfi[0]->m_coord->max.y != pictureBox1_ymax)
            updateflag = true;
        lock.release();

        if(updateflag == false)
            return;

        for(int ii=0; ii<nplots_bfi; ii++)
            bfi[ii]->UpdateParams(pictureBox1_twidth, pictureBox1_ymin, pictureBox1_ymax, size, false, false);
        bfimult->UpdateParams(pictureBox1_twidth, pictureBox1_ymin, pictureBox1_ymax, size, true, m_waterfall);

        if(size >= pictureBox1_twidth)
        {
            if(nplotsactive==1)
                bfi[0]->disp->updateDisplay(bfi[0]->m_coord, size, usbdevice->GetDataRate());
            else
                bfimult->disp->updateDisplay(bfimult->m_coord, size, usbdevice->GetDataRate());
        }
        else
        {
            if(nplotsactive==1)
                bfi[0]->disp->updateDisplay(bfi[0]->m_coord, usbdevice->GetDataRate());
            else
                bfimult->disp->updateDisplay(bfimult->m_coord, usbdevice->GetDataRate());
        }
    }



    // ------------------------------------------------------------------------------------------------
    void BloodFlowMonitor()
    {
        int nplotsactive = getActivePlots(BFI);
        MyCoord** p = new MyCoord*[nplots_bfi]();
        MyVector** data_0 = new MyVector*[nplots_bfi]();
        MyVector** data = new MyVector*[nplots_bfi]();

        for(int ii=0; ii<nplots_bfi; ii++) 
		{
            p[ii] = bfi[ii]->m_coord;
            data_0[ii] = bfi[ii]->m_data;
            data[ii] = bfi[ii]->m_data;
        }

        int ii = 0;
        int jj = 0;
        int nDispMoves = 0;
        int c = 0;
        DataStreamPlot^ datastream;
        if(nplotsactive > 1)
            datastream = bfimult;
        else if(nplotsactive == 1)
            datastream = bfi[(int)log2(checkedplots_bfi)];
        else
            datastream = bfi[0];

        UpdateAuxBfiAxesParams(datastream, 0);

        // Increase update rate by about 40% over the data rate to make sure 
        // we keep up with incoming data. The display lags behind slightly 
        // (which increases with time) when sleeptime = 1/m_updateRate*1000
        // because displaying itself also uses up a few milliseconds. 6
        int sleeptime = (int)(.30 / datastream->m_updateRate*1000.0);

        int dStep = datastream->m_dStep;
        int dRate = datastream->m_dRateRaw;
        int incr = ceil(datastream->m_dIncrDisp);

        datastream->disp->clearDisplay(datastream->m_coord, usbdevice->GetDataRate());

        while(m_acquisitionInProgress)
        {
            // Copy aux data from generated input data (aux[i]->data) to output data to be displayed 
            // (aux[i]->m_coord->plot.data)
            for(jj=0, ii=0; jj < datastream->m_dWidthPlot; jj++, ii+=incr)
            {
                // Check if we displayed all the points available
                if((nDispMoves*dStep + ii) >= (bfi[0]->m_inext-1))
                    break;

                // x is the same for all plots, therefore do it once for each 
                // data transfer.
                datastream->m_coord->plot.data[jj].x = data[0][ii].x;

                for(int kk=0; kk<nplotsactive; kk++)
                {
                    p[idx_active_plots_bfi[kk]]->plot.data[jj] = data[idx_active_plots_bfi[kk]][ii];

                    // Calculate y mean and y max/min of current plot
                    p[idx_active_plots_bfi[kk]]->plot.CalcStats(jj);
                }
            }


            // Plot bfi data
            if(nplotsactive==1)
            {
                int coloridx = datastream->disp->getColorIdxFromBottom(idx_active_plots_bfi[0]);
                datastream->disp->plotData(p[idx_active_plots_bfi[0]], jj-1, (double)dRate, coloridx);
            }
            else
                datastream->disp->plotData(p, datastream->m_coord, jj-1, (double)dRate, idx_active_plots_bfi, nplotsactive);

            pause(sleeptime);

            if(((data[0] + dStep) - data_0[0] >= BUF_SIZE) || m_bufferWrap == true)
            {
                for(int kk=0; kk<nplots_bfi; kk++) 
                    data[kk] = data_0[kk];
                datastream->disp->clearDisplay(datastream->m_coord, jj, usbdevice->GetDataRate());
                nDispMoves = 0;
                m_bufferWrap = false;
                Sleep(10);
            }

            if(jj >= datastream->m_dWidthPlot)
            {
                // Make sure not to overstep data buffer
                for(int kk=0; kk<nplots_bfi; kk++) {
                    data[kk] += dStep;
                }
                nDispMoves++;
                datastream->disp->clearDisplay(datastream->m_coord, jj, datastream->m_dRatePlot);
                Sleep(10);
            }
            UpdateAuxBfiAxesParams(datastream, jj-1);

            for(int kk=0; kk<nplotsactive; kk++)
                p[idx_active_plots_bfi[kk]]->plot.ResetStats();
        }

        // Finished with data acquisition. Now clean up. 
        for(int kk=0; kk<nplots_bfi; kk++)
            bfi[kk]->m_coord->ResetAxes(dRate);
        bfimult->m_coord->ResetAxes(dRate);
        bfimult->disp->clearDisplay(bfimult->m_coord, usbdevice->GetDataRate());
    }



    // ------------------------------------------------------------------------------------------------
    void AnalogDataMonitor()
    {
        int nplotsactive = getActivePlots(AUX);
        MyCoord** p = new MyCoord*[nplots_aux]();
        MyVector** data_0 = new MyVector*[nplots_aux]();
        MyVector** data = new MyVector*[nplots_aux]();

        for(int ii=0; ii<nplots_aux; ii++) 
		{
            p[ii] = aux[ii]->m_coord;
            data_0[ii] = aux[ii]->m_data;
            data[ii] = aux[ii]->m_data;
        }

        int ii = 0;
        int jj = 0;
        int nDispMoves = 0;
        int c = 0;
        DataStreamPlot^ datastream;

        if(nplotsactive > 1)
            datastream = auxmult;
        else if(nplotsactive == 1)
            datastream = aux[(int)log2(checkedplots_aux)];
        else
            datastream = aux[0];

        // Increase update rate by about 40% over the data rate to make sure 
        // we keep up with incoming data. The display lags behind slightly 
        // (which increases with time) when sleeptime = 1/m_updateRate*1000
        // because displaying itself also uses up a few milliseconds. 
        int sleeptime = (int)(.50 / datastream->m_updateRate*1000.0);


        int dStep = datastream->m_dStep;
        int dRate = datastream->m_dRateRaw;
        int incr = ceil(datastream->m_dIncrDisp);

        datastream->disp->clearDisplay(datastream->m_coord, usbdevice->GetDataRate());

        while(m_acquisitionInProgress)
        {
            // Copy aux data from generated input data (aux[i]->data) to output data to be displayed 
            // (aux[i]->m_coord->plot.data)
            for(jj=0, ii=0; jj < datastream->m_dWidthPlot; jj++, ii=ii+incr)
            {
                // Check if we displayed all the points available
                if((nDispMoves*dStep + ii) >= (m_count-1))
                    break;

                // x is the same for all plots, therefore do it once for each 
                // data transfer.
                datastream->m_coord->plot.data[jj].x = data[0][ii].x;

                for(int kk=0; kk<nplotsactive; kk++)
                {
                    // Transfer data received from CollectData to plot data that will be displayed. 
                    // The received data is at higher resolution than plot data - we don't want to 
                    // plot everything, just enough to approximate the curve more or less accurately.
                    p[idx_active_plots_aux[kk]]->plot.data[jj].y = data[idx_active_plots_aux[kk]][ii].y * ANALOG_RAW_TO_VOLTS;
                    p[idx_active_plots_aux[kk]]->plot.data[jj].x = data[idx_active_plots_aux[kk]][ii].x;

                    // Calculate y mean and y max/min of current plot
                    p[idx_active_plots_aux[kk]]->plot.CalcStats(jj);
                }
            }


            // Plot aux data
            if(nplotsactive == 1)
            {
                int coloridx = datastream->disp->getColorIdxFromTop(idx_active_plots_aux[0]);
                datastream->disp->plotData(p[idx_active_plots_aux[0]], jj-1, (double)dRate, coloridx);
            }
            else
                datastream->disp->plotData(p, datastream->m_coord, jj-1, (double)dRate, idx_active_plots_aux, nplotsactive);


            pause(sleeptime);

            if(((data[0] + dStep) - data_0[0] >= BUF_SIZE) || m_bufferWrap == true)
            {
                for(int kk=0; kk<nplots_aux; kk++) {
                    data[kk] = data_0[kk];
                }
                datastream->disp->clearDisplay(datastream->m_coord, jj, usbdevice->GetDataRate());
                nDispMoves = 0;
                m_bufferWrap = false;
                Sleep(1);
            }

            if(jj >= datastream->m_dWidthPlot)
            {
                // Make sure not to overstep data buffer
                for(int kk=0; kk<nplots_aux; kk++) {
                    data[kk] += dStep;
                }
                nDispMoves++;
                datastream->disp->clearDisplay(datastream->m_coord, jj, usbdevice->GetDataRate());
                Sleep(1);
            }

            for(int kk=0; kk<nplotsactive; kk++)
                p[idx_active_plots_aux[kk]]->plot.ResetStats();
        }

        // Finished with data acquisition. Now clean up. 
        for(int kk=0; kk<nplots_aux; kk++)
            aux[kk]->m_coord->ResetAxes(dRate);
        auxmult->m_coord->ResetAxes(dRate);
        auxmult->disp->clearDisplay(bfimult->m_coord, usbdevice->GetDataRate());
    }


    // Input data streams  
    array<DataStreamPlot^>^ aux;
    array<DataStreamPlot^>^ bfi;
    array<DataStreamPlot^>^ corr;

    DataStreamPlot^ auxmult;
    DataStreamPlot^ bfimult;
    DataStreamPlot^ corrmult;

    int nplots_aux;
    int nplots_bfi;
    int nplots_corr;

    DcsDeviceDataParser* m_parser;
    CorrFunc** m_corrdata;

    Fx3Device* usbdevice;

    bool m_acquisitionInProgress;
    bool m_acquisitionPaused;

    int m_count;
    bool m_bufferWrap;
    int errstatus;

    double* m_BFi;
    double* m_beta;
    double* m_err;

    int* m_chDlg_rate;
    int* m_chDlg_BFi;
    int* m_enableDet;

    long m_maxPktSizeDefault;

    int m_save;

    int m_nDeviceIndex;

    delegate void StringArgReturningVoidDelegate(String^ text);
    delegate void SetTextBoxDetDelegate(String^ text);
    delegate void IntArgReturningVoidDelegate(int^ idx);
    delegate void ListBoxDelegate(int^ idx);
    delegate bool MessageBoxDelegate(String^ message);

    UINT32  checkedplots_aux;
    UINT32  checkedplots_bfi;
    UINT32  checkedplots_corr;

    int*    idx_active_plots_aux;
    int*    idx_active_plots_bfi;
    int*    idx_active_plots_corr;

    double  datafactor;
    unsigned long long  acqStartTime;
    double  acqElapsedTime;

    array<Control^>^ countTextBoxes;
    array<Control^>^ bfiTextBoxes;

    double pictureBox1_twidth;
    double pictureBox1_ymin;
    double pictureBox1_ymax;
    MyLock lock;

    bool m_waterfall;

    Color buttonStartForeColor;
    Color buttonStartBackColor;
    Color buttonStopForeColor;
    Color buttonStopBackColor;
    Color textBoxElapsedTimeColor_AcqStart;
    Color textBoxElapsedTimeColor_AcqStop;

    int laserpower;

    MyFile^ m_replayFileData;
    MyFile^ m_saveFileDataRaw;
    MyFile^ m_saveFileDataCorr;
    MyFile^ m_saveFileDataAux;
    MyFile^ m_simulateData;

    int m_MaxFiles;
    int m_MaxNameSize;
    int m_nRun;

    PCY_DATA_BUFFER qHead;
    PCY_DATA_BUFFER qTail;
    static AutoResetEvent^ m_hDataQueueEvent = gcnew AutoResetEvent(false);

    Object^  _critSect;

    double m_expTime;
    int    m_toggleBFiAux;

    LightAndTissPropertiesForm::LightAndTissPropertiesForm^ form_lightTissProp;

    };
}
