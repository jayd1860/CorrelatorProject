

// Fx3ReceiveDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "windows.h"
#include "Coord.h"
#include "Fx3ReceiveData.h"
#include "Fx3Device.h"
#include "Fx3ReceiveDataDlg.h"
#include <dbt.h>
#include "time.h"
#include "Utils.h"
#include "pathProcess.h"
#include "WinUser.h"

#define     PACKETS_PER_TRANSFER                8		// 256
#define     STOP_DATA_COLLECTION_EVENT          33
#define     ANALOG_RAW_TO_VOLTS                 0.0006114-5.0

void BloodFlowMonitor(LPVOID lpParam);

// CFx3ReceiveDataDlg dialog
CCriticalSection CFx3ReceiveDataDlg::_critSect;
PCY_DATA_BUFFER CFx3ReceiveDataDlg::pHead = NULL;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

    // Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};



// --------------------------------------------------------------------------------------------
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}



// --------------------------------------------------------------------------------------------
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// --------------------------------------------------------------------------------------------
CFx3ReceiveDataDlg::CFx3ReceiveDataDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CFx3ReceiveDataDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_configInProgress = false;
    m_maxPktSizeDefault = (long)pow(2.0, 14.0);
    m_clockSpeed = 1.5e8;
    m_dRateRaw = 50000.0;
    m_dRateGen = 300;
    m_segSize = 11141120;

    m_redcolor = RGB(255, 40, 30);                      // red
    m_greencolor = RGB(40, 200, 30);
    m_bluecolor = RGB(34, 30, 255);                     // blue
    m_cyancolor = RGB(0, 255, 255);
    m_magentacolor = RGB(255, 0, 255);

    m_textcolor = RGB(255, 255, 255);

    m_redbrush.CreateSolidBrush(m_redcolor);
    m_greenbrush.CreateSolidBrush(m_greencolor);
    m_bluebrush.CreateSolidBrush(m_bluecolor);
    m_cyanbrush.CreateSolidBrush(m_cyancolor);
    m_magentabrush.CreateSolidBrush(m_magentacolor);

    m_enableDet[0] = 1;
    m_enableDet[1] = 1;
    m_enableDet[2] = 1;
    m_enableDet[3] = 1;

    m_dirnameRoot.Format(_T("C:\\Users\\Public\\DCS"));
    if(!PathFileExists(m_dirnameRoot.GetString()))
        CreateDirectory(m_dirnameRoot, NULL);

    wchar_t datetime[13];
    memset(datetime, 0, sizeof(datetime));
    getCurrDateTimeString(datetime);
    m_dirnameBase.Format(_T("%s\\dcs_%s"), m_dirnameRoot.GetString(), datetime);

    m_nRun = 0;

    m_filenameRaw.Empty();
    m_filenameCorr.Empty();
    m_filenameAux.Empty();
	m_dirnameInput.Empty();
	m_filenameInput.Empty();
	m_filenameInputFull.Empty();

}



// --------------------------------------------------------------------------------------------
int CFx3ReceiveDataDlg::GetRunNumberFromGui(CString str)
{
    LPCWSTR strP;
    const wchar_t* r;
    char  nRunStr[4];
    int nRun=0;

    strP = str.GetString();

    if((r = wcsstr(strP, L"_run")) != NULL)
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

    return 0;
}




// --------------------------------------------------------------------------------------------
int CFx3ReceiveDataDlg::GetFirstFilename(CString* filename)
{
	CString names[1000];
	CString search_path = m_dirnameInput + "/*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(search_path,&fd);
	int nFiles = 0;

	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names[nFiles].Format(fd.cFileName);
				nFiles++;
			}
		} while(FindNextFile(hFind,&fd));
		FindClose(hFind);
	}

	*filename = names[0];

	return 0;
}




// --------------------------------------------------------------------------------------------
int CFx3ReceiveDataDlg::GetNewRunNumber()
{
    if(!PathFileExists(m_dirnameBase.GetString())) {
        m_nRun = 1;
    }
    else {
        CString names[100];
        CString search_path = m_dirnameBase + "/*.*";
        WIN32_FIND_DATA fd;
        HANDLE hFind = FindFirstFile(search_path, &fd);
        int nFiles = 0;
        const wchar_t* r;
        const wchar_t* s;
        char  nRunStr[4];
        int nRun = 0;

        memset(nRunStr, 0, sizeof(nRunStr));

        if(hFind != INVALID_HANDLE_VALUE) {
            do {
                // read all (real) files in current folder
                // , delete '!' read other 2 default folder . and ..
                if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    names[nFiles].Format(fd.cFileName);
                    nFiles++;
                }
            } while(FindNextFile(hFind, &fd));
            FindClose(hFind);
        }

        m_nRun = 1;

        // Check all the found files to determine run number
        for(int ii=0; ii < nFiles; ii++)
        {
            s = names[ii].GetString();
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
    }
    return 0;
}




// --------------------------------------------------------------------------------------------
int CFx3ReceiveDataDlg::GetFilenamePrefixFromGui(CString* prefix, CString editstr)
{
    LPCWSTR editstrP = editstr.GetString();
    const wchar_t* r;
    wchar_t s[100];

    if(editstr.IsEmpty()) {
        prefix->Format(_T("dcs"));
        return 0;
    }

    memset(s, 0, sizeof(s));

    prefix->Format(editstrP);

    if((r = wcsstr(editstrP, L"_run")) != NULL) {

        if(r[4]==0 || r[5]==0 || r[6]==0)
            return 0;

        if(!isdigit(r[4]) || !isdigit(r[5]) || !isdigit(r[6]))
            return 0;

        for(int ii=0; &editstrP[ii] != r; ii++)
            s[ii] = editstrP[ii];

        prefix->Format(s);

    }

    return 0;
}



// --------------------------------------------------------------------------------------------
int CFx3ReceiveDataDlg::GetNewBaseFilename()
{
    int nRun;
    CString editstr;
    CString prefix;

    m_editFile.GetWindowText(editstr);

    // Dtermine which run this is
    GetNewRunNumber();
    nRun = GetRunNumberFromGui(editstr);

    if(nRun==m_nRun)
        m_filenameBase = editstr;
    else {
        GetFilenamePrefixFromGui(&prefix, editstr);
        if(m_nRun < 10)
            m_filenameBase.Format(_T("%s_run00%d"), prefix.GetString(), m_nRun);
        else if(m_nRun < 100)
            m_filenameBase.Format(_T("%s_run0%d"), prefix.GetString(), m_nRun);
        else
            m_filenameBase.Format(_T("%s_run%d"), prefix.GetString(), m_nRun);
    }



    // First get the filename from GUI
    m_editFolder.SetWindowText(m_dirnameBase);
    m_editFile.SetWindowText(m_filenameBase);

    m_filenameBaseFull.Format(_T("%s\\%s"), m_dirnameBase.GetString(), m_filenameBase.GetString());

    // Set all the rest of the different types of file names used for saving data
    SetFilenames();

    return 0;
}



// --------------------------------------------------------------------------------------------
void CFx3ReceiveDataDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_ManagedControl(pDX, IDC_CTRL1, m_ctrl1);
    DDX_ManagedControl(pDX, IDC_CTRL2, m_ctrl2);
    DDX_ManagedControl(pDX, IDC_CTRL3, m_ctrl3);
    DDX_Control(pDX, IDC_CBO_DEVICES, m_cboDevices);
    DDX_Control(pDX, IDC_EDIT_SESSION_FOLDER, m_editFolder);
    DDX_Control(pDX, IDC_EDIT_FILENM_BASE, m_editFile);
    DDX_Control(pDX, IDC_BTN_FILE, m_radCounter);
    DDX_Control(pDX, IDC_CBO_TIMEOUT, m_cboTimeout);
    DDX_Control(pDX, IDC_BTN_START_TRANSFER, m_btnTransfer);
}



// --------------------------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CFx3ReceiveDataDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_WM_CTLCOLOR()
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BTN_FILE, &CFx3ReceiveDataDlg::OnBnClickedBtnFile)
    ON_BN_CLICKED(IDC_RAD_SAVE_RAW_DATA, &CFx3ReceiveDataDlg::OnBnClickedRadSaveRawData)
    ON_BN_CLICKED(IDC_BTN_START_TRANSFER, &CFx3ReceiveDataDlg::OnBnClickedStartDataTransfer)
    ON_MESSAGE(WM_DATA_TRANSFER_COMPLETE, &CFx3ReceiveDataDlg::DataTransferComplete)
    ON_MESSAGE(WM_DATA_TRANSFER_STARTED, &CFx3ReceiveDataDlg::DataTransferStarted)
    ON_BN_CLICKED(IDC_BUTTON_DEV_CONFIG, &CFx3ReceiveDataDlg::OnBnClickedButtonDevConfig)
    ON_BN_CLICKED(IDC_CHECK_AUTOZOOM, &CFx3ReceiveDataDlg::OnBnClickedCheckAutozoom)
    ON_BN_CLICKED(IDC_PHOT_COUNT_DET1_CHECKBOX, &CFx3ReceiveDataDlg::OnBnClickedPhotCountDet1Checkbox)
    ON_BN_CLICKED(IDC_PHOT_COUNT_DET2_CHECKBOX, &CFx3ReceiveDataDlg::OnBnClickedPhotCountDet2Checkbox)
    ON_BN_CLICKED(IDC_PHOT_COUNT_DET3_CHECKBOX, &CFx3ReceiveDataDlg::OnBnClickedPhotCountDet3Checkbox)
    ON_BN_CLICKED(IDC_PHOT_COUNT_DET4_CHECKBOX, &CFx3ReceiveDataDlg::OnBnClickedPhotCountDet4Checkbox)
    ON_BN_CLICKED(IDC_BTN_SEND_CMD_2SETCURRENT, &CFx3ReceiveDataDlg::OnBnClickedBtnSendCmd2setcurrent)
    ON_BN_CLICKED(IDC_BUTTON_DELAY_THRESHOLD, &CFx3ReceiveDataDlg::OnBnClickedButtonDelayThreshold)
    ON_BN_CLICKED(IDC_RAD_SAVE_CORR_DATA, &CFx3ReceiveDataDlg::OnBnClickedRadSaveCorrData)
    ON_BN_CLICKED(IDC_BTN_SUBMIT, &CFx3ReceiveDataDlg::OnBnClickedBtnSubmit)
    ON_BN_CLICKED(IDC_RAD_REPLAY_EXIST_DATA, &CFx3ReceiveDataDlg::OnBnClickedRadReplayExistFile)
    ON_BN_CLICKED(IDC_BTN_BROWSE_FILE, &CFx3ReceiveDataDlg::OnBnClickedBtnBrowseFile)
END_MESSAGE_MAP()



// --------------------------------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnOK(void)
{
    CWnd* pWnd = GetFocus();
    if(GetDlgItem(IDOK) == pWnd)
    {
        CDialog::OnOK();
        return;
    }

    // Enter key was hit -> do whatever you want
    CFx3ReceiveDataDlg::OnBnClickedBtnSendCmd2setcurrent();
}



// --------------------------------------------------------------------------------------------
// CFx3ReceiveDataDlg message handlers
BOOL CFx3ReceiveDataDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    CButton* pBtn;
    CString   str;

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if(pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if(!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // TODO: Add extra initialization here
    m_hDeviceNotify = NULL;
    AddDevicesToDropdownMenu();

    EnableSavePanelObjs(FALSE);
    ((CButton *)GetDlgItem(IDC_RAD_SAVE_RAW_DATA))->SetCheck(0);
    ((CButton *)GetDlgItem(IDC_RAD_SAVE_CORR_DATA))->SetCheck(0);

    m_saveraw = (((CButton *)GetDlgItem(IDC_RAD_SAVE_RAW_DATA))->GetCheck() == BST_CHECKED);
    m_savecorr = (((CButton *)GetDlgItem(IDC_RAD_SAVE_CORR_DATA))->GetCheck() == BST_CHECKED);
    m_replay = (((CButton *)GetDlgItem(IDC_RAD_REPLAY_EXIST_DATA))->GetCheck() == BST_CHECKED);

    m_hDataQueueEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hWriteCompleted = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_pInfoMessage = NULL;

    m_dataCollectionInProgress = false;

    // Initialize device data streamers

    double tWidth1 = 10;                  // Display window time width in seconds
    double dRateRaw1 = m_dRateRaw;                    // The data generator speed in data points per second.
    double dRateGen1 = m_dRateGen;                    // The data generator speed in data points per second.
    double dRatePlot1 = m_dRateGen / 10;                    // The data generator speed in data points per second.
    MyVector origin1(0, 0);
    MyVector min1(0, 0);
    MyVector max1(tWidth1*dRateRaw1, 1000);
    m_aux1 = new DataStreamPlot(tWidth1, dRateRaw1, dRateGen1, dRatePlot1, origin1, min1, max1, 0, 1, 1);
    m_ctrl1->setDataCoord(*m_aux1->m_coord[0]);

    double tWidth2 = 10;                  // Display window time width in seconds
    double dRateRaw2 = m_dRateRaw;                    // The data generator speed in data points per second.
    double dRateGen2 = m_dRateGen;                    // The data generator speed in data points per second.
    double dRatePlot2 = m_dRateGen / 10;                    // The data generator speed in data points per second.
    MyVector origin2(0, 0);
    MyVector min2(0, 0);
    MyVector max2(tWidth2*dRateRaw2, 1000);
    m_aux2 = new DataStreamPlot(tWidth2, dRateRaw2, dRateGen2, dRatePlot2, origin2, min2, max2, 0, 3, 1);
    m_ctrl2->setDataCoord(*m_aux2->m_coord[0]);


    double tWidth3 = 0;                    // This field isn't used for this display
    double dRateRaw3 = m_corrdata->nbins;  // The data generator speed in data points per second.
    double dRateGen3 = m_corrdata->nbins;  // The data generator speed in data points per second.
    double dRatePlot3 = m_corrdata->nbins; // The data generator speed in data points per second.
    double min_x_corr = 4e-7;
    double max_x_corr = 1e-2;
    MyVector origin3(0, 0);
    MyVector min3(log10(min_x_corr), .99);
    MyVector max3(log10(max_x_corr), 1.7);
    m_corrCurve = new DataStreamPlot(tWidth3, dRateRaw3, dRateGen3, dRatePlot3, origin3, min3, max3, m_corrdata->nbins, 3, m_corrdata->nCh);
    for(int ii=0; ii < m_corrdata->nCh; ii++)
        m_ctrl3->setDataCoord(*m_corrCurve->m_coord[ii]);

    m_BFi = new double[m_corrdata->nCh];
    m_beta = new double[m_corrdata->nCh];
    m_err = new double[m_corrdata->nCh];

    pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOZOOM);
    pBtn->SetCheck(1);
    m_aux1->m_coord[0]->plot.SetAutozoom(pBtn->GetCheck());
    m_aux2->m_coord[0]->plot.SetAutozoom(pBtn->GetCheck());
    for(int ii=0; ii < m_corrdata->nCh; ii++)
        m_corrCurve->m_coord[ii]->plot.SetAutozoom(0);


    // Enable all detector correlation curve plots
    pBtn = (CButton*)GetDlgItem(IDC_PHOT_COUNT_DET1_CHECKBOX);
    pBtn->SetCheck(m_enableDet[0]);
    pBtn = (CButton*)GetDlgItem(IDC_PHOT_COUNT_DET2_CHECKBOX);
    pBtn->SetCheck(m_enableDet[1]);
    pBtn = (CButton*)GetDlgItem(IDC_PHOT_COUNT_DET3_CHECKBOX);
    pBtn->SetCheck(m_enableDet[2]);
    pBtn = (CButton*)GetDlgItem(IDC_PHOT_COUNT_DET4_CHECKBOX);
    pBtn->SetCheck(m_enableDet[3]);

    str.Format(_T("%d"), (int)((double)m_parser->dataDCS->countThreshold / 1000.0));
    GetDlgItem(IDC_EDIT_COUNT_LIMIT)->SetWindowText(str);

    str.Format(_T("%0.1g,  %0.1g"), min_x_corr, max_x_corr);
    GetDlgItem(IDC_EDIT_DELAY_THRESHOLD)->SetWindowText(str);

    m_chDlg_rate[0] = IDC_STC_PHOTCOUNT_DET1;
    m_chDlg_rate[1] = IDC_STC_PHOTCOUNT_DET2;
    m_chDlg_rate[2] = IDC_STC_PHOTCOUNT_DET3;
    m_chDlg_rate[3] = IDC_STC_PHOTCOUNT_DET4;
    m_chDlg_BFi[0] = IDC_STC_BFIBETA_DET1;
    m_chDlg_BFi[1] = IDC_STC_BFIBETA_DET2;
    m_chDlg_BFi[2] = IDC_STC_BFIBETA_DET3;
    m_chDlg_BFi[3] = IDC_STC_BFIBETA_DET4;

    // Generate new base file name for new run
    GetNewBaseFilename();

    m_cboTimeout.SetCurSel(4);

    return TRUE;  // return TRUE  unless you set the focus to a control
}




// --------------------------------------------------------------------------------------------
HBRUSH CFx3ReceiveDataDlg::OnCtlColor(CDC* pDC, CWnd *pWnd, UINT nCtlColor)
{
    HBRUSH hbr;

    pDC->SetTextColor(m_textcolor);  // change the text color
    switch(pWnd->GetDlgCtrlID())
    {
    case IDC_PHOT_COUNT_DET1_CHECKBOX:
        pDC->SetBkColor(m_redcolor);    // change the background
        hbr = (HBRUSH)m_redbrush;    // apply the blue brush
        break;

    case IDC_PHOT_COUNT_DET2_CHECKBOX:
        pDC->SetBkColor(m_greencolor);    // change the background
        hbr = (HBRUSH)m_greenbrush;    // apply the blue brush
        break;

    case IDC_PHOT_COUNT_DET3_CHECKBOX:
        pDC->SetBkColor(m_bluecolor);    // change the background
        hbr = (HBRUSH)m_bluebrush;    // apply the blue brush
        break;

    case IDC_PHOT_COUNT_DET4_CHECKBOX:
        pDC->SetBkColor(m_magentacolor);    // change the background
        hbr = (HBRUSH)m_magentabrush;    // apply the blue brush
        break;

    default:
        hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    };

    return hbr;
}



// --------------------------------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}




// --------------------------------------------------------------------------------------------
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CFx3ReceiveDataDlg::OnPaint()
{
    if(IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}




// --------------------------------------------------------------------------------------------
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFx3ReceiveDataDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}




// --------------------------------------------------------------------------------------------
LRESULT CFx3ReceiveDataDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    return CDialogEx::DefWindowProc(message, wParam, lParam);
}




// -----------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedButtonDevConfig()
{

    int nDeviceIndex = m_cboDevices.GetCurSel();
    if(nDeviceIndex == -1)
        return;

    Fx3Device* USBDevice = new Fx3Device(nDeviceIndex, true);

    m_configInProgress = true;
    if(!USBDevice->IsConnected()) {
        MessageBox(L"No device is connected for configuration");
        return;
    }

    if(USBDevice->IsStreamer()) {
        MessageBox(L"Device already configured Streamer");
        return;
    }

    m_ctrl1->FPGAConfigEntry();

    Cleanup(this, L"", USBDevice, NULL);
    m_configInProgress = false;
    AddDevicesToDropdownMenu();
}




// --------------------------------------------------------------------------------------------
BOOL CFx3ReceiveDataDlg::AddDevicesToDropdownMenu()
{
    Fx3Device* USBDevice = new Fx3Device();
    int nDeviceCount = USBDevice->GetDeviceCount();
    long totalTransferSize = USBDevice->GetTotalTransferSize();
    CString* devNames = USBDevice->GetDeviceNames();
    int nCount;

    m_cboDevices.ResetContent();

    for(nCount = 0; nCount < nDeviceCount; nCount++)
        m_cboDevices.InsertString(nCount, devNames[nCount]);

    m_parser = new Fx3DataParser(totalTransferSize*PACKETS_PER_TRANSFER);
    m_corrdata = new CorrFunc(m_parser->dataDCS->nCh);

    if(m_cboDevices.GetCount() >= 1)
        m_cboDevices.SetCurSel(0);
    SetFocus();

    delete USBDevice;
    return TRUE;
}




// --------------------------------------------------------------------------------------------
void CFx3ReceiveDataDlg::EnableSavePanelObjs(bool enable)
{
    GetDlgItem(IDC_STATIC_SESSION_FOLDER)->EnableWindow(enable);
    GetDlgItem(IDC_STATIC_FILENM_BASE)->EnableWindow(enable);
    GetDlgItem(IDC_EDIT_SESSION_FOLDER)->EnableWindow(enable);
    GetDlgItem(IDC_EDIT_FILENM_BASE)->EnableWindow(enable);
    GetDlgItem(IDC_BTN_BROWSE_FOLDER)->EnableWindow(enable);
    GetDlgItem(IDC_CBO_TIMEOUT)->EnableWindow(enable);
    GetDlgItem(IDC_STC_TIMEOUT)->EnableWindow(enable);
    GetDlgItem(IDC_STC_MILLI)->EnableWindow(enable);
    GetDlgItem(IDC_BTN_SUBMIT)->EnableWindow(enable);
    if(enable==true) {
        GetDlgItem(IDC_BTN_BROWSE_FILE)->ShowWindow(SW_HIDE);
        m_editFile.SetWindowText(m_filenameBase);
		m_editFolder.SetWindowText(m_dirnameBase);
	}
}



// --------------------------------------------------------------------------------------------
void CFx3ReceiveDataDlg::EnableSavePanelReplayObjs(bool enable)
{
    GetDlgItem(IDC_STATIC_FILENM_BASE)->EnableWindow(enable);
    GetDlgItem(IDC_EDIT_FILENM_BASE)->EnableWindow(enable);
    GetDlgItem(IDC_BTN_BROWSE_FILE)->EnableWindow(enable);
    GetDlgItem(IDC_BTN_BROWSE_FILE)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_CBO_TIMEOUT)->EnableWindow(enable);
    GetDlgItem(IDC_STC_TIMEOUT)->EnableWindow(enable);
    GetDlgItem(IDC_STC_MILLI)->EnableWindow(enable);
    GetDlgItem(IDC_BTN_SUBMIT)->EnableWindow(enable);
    GetDlgItem(IDC_BTN_BROWSE_FOLDER)->EnableWindow(enable);
    GetDlgItem(IDC_STATIC_SESSION_FOLDER)->EnableWindow(enable);
    GetDlgItem(IDC_EDIT_SESSION_FOLDER)->EnableWindow(enable);
    if(enable==true) {
        GetDlgItem(IDC_RAD_SAVE_RAW_DATA)->EnableWindow(0);
        GetDlgItem(IDC_RAD_SAVE_CORR_DATA)->EnableWindow(0);
        GetDlgItem(IDC_BTN_BROWSE_FILE)->ShowWindow(SW_SHOW);
		if(!m_dirnameInput.IsEmpty())
			m_editFolder.SetWindowText(m_dirnameInput);
		if(!m_filenameInput.IsEmpty())
			m_editFile.SetWindowText(m_filenameInput);
	}
    else {
        GetDlgItem(IDC_BTN_BROWSE_FILE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_RAD_SAVE_RAW_DATA)->EnableWindow(1);
        GetDlgItem(IDC_RAD_SAVE_CORR_DATA)->EnableWindow(1);
		m_editFolder.SetWindowText(m_dirnameBase);
		m_editFile.SetWindowText(m_filenameBase);
	}

}



// --------------------------------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedBtnFile()
{
    TCHAR dirnameSelected[1024];
    if(browseForFolder(m_dirnameRoot, dirnameSelected) < 0)
        return;

	if(m_replay) {
		CString filename;

		m_dirnameInput.Format(dirnameSelected);
		GetFirstFilename(&filename);
		m_filenameInput = filename;
		m_filenameInputFull.Format(_T("%s\\%s"), m_dirnameInput.GetString(), m_filenameInput.GetString());

		m_editFolder.SetWindowText(m_dirnameInput);
		m_editFile.SetWindowText(m_filenameInput);
	}
	else {
		m_dirnameBase.Format(dirnameSelected);
		GetNewBaseFilename();
		SetFilenames();
	}
}




// --------------------------------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedRadSaveRawData()
{
    m_saveraw = !(((CButton *)GetDlgItem(IDC_RAD_SAVE_RAW_DATA))->GetCheck() == BST_CHECKED);
    m_savecorr = (((CButton *)GetDlgItem(IDC_RAD_SAVE_CORR_DATA))->GetCheck() == BST_CHECKED);
    ((CButton *)GetDlgItem(IDC_RAD_SAVE_RAW_DATA))->SetCheck(m_saveraw);


    ((CButton *)GetDlgItem(IDC_RAD_REPLAY_EXIST_DATA))->EnableWindow(!(m_saveraw || m_savecorr));
    if(m_saveraw || m_savecorr) {
        m_editFile.SetWindowText(m_filenameBase);
        ((CButton *)GetDlgItem(IDC_RAD_REPLAY_EXIST_DATA))->SetCheck(0);
        m_replay = false;
    }


    if(!m_saveraw && !m_savecorr)
    {
        EnableSavePanelObjs(FALSE);
        m_filenameAux.Empty();
    }
    else
        EnableSavePanelObjs(TRUE);

    if(!m_saveraw)
        m_filenameRaw.Empty();
    else
        m_filenameAux.Empty();

    SetFilenames();
}



// --------------------------------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedRadSaveCorrData()
{
    m_saveraw = (((CButton *)GetDlgItem(IDC_RAD_SAVE_RAW_DATA))->GetCheck() == BST_CHECKED);
    m_savecorr = !(((CButton *)GetDlgItem(IDC_RAD_SAVE_CORR_DATA))->GetCheck() == BST_CHECKED);
    ((CButton *)GetDlgItem(IDC_RAD_SAVE_CORR_DATA))->SetCheck(m_savecorr);

    ((CButton *)GetDlgItem(IDC_RAD_REPLAY_EXIST_DATA))->EnableWindow(!(m_saveraw || m_savecorr));
    if(m_saveraw || m_savecorr) {
        m_editFile.SetWindowText(m_filenameBase);
        ((CButton *)GetDlgItem(IDC_RAD_REPLAY_EXIST_DATA))->SetCheck(0);
        m_replay = false;
    }

    if(!m_saveraw && !m_savecorr)
    {
        EnableSavePanelObjs(FALSE);
        m_filenameAux.Empty();
    }
    else
        EnableSavePanelObjs(TRUE);

    if(!m_savecorr)
        m_filenameCorr.Empty();

    if(m_saveraw)
        m_filenameAux.Empty();

    SetFilenames();
}




// --------------------------------------------------------------------------------------------
int CFx3ReceiveDataDlg::SetFilenames()
{
    // Set all the filenames of the different data types
    if(!m_filenameBaseFull.IsEmpty())
    {
        if(m_saveraw)
            m_filenameRaw = m_filenameBaseFull + _T(".raw");

        if(m_savecorr)
            m_filenameCorr = m_filenameBaseFull + _T(".corr");;

        if(m_savecorr & !m_saveraw)
            m_filenameAux = m_filenameBaseFull + _T(".aux");
    }
    return 0;
}



// --------------------------------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedStartDataTransfer()
{
    // TODO: Add your control notification handler code here
    CString strText;
    CString filename;
    CString str;
    m_btnTransfer.GetWindowText(strText);
    bool saveraw = (((CButton *)GetDlgItem(IDC_RAD_SAVE_RAW_DATA))->GetCheck() == BST_CHECKED);
    bool savecorr = (((CButton *)GetDlgItem(IDC_RAD_SAVE_CORR_DATA))->GetCheck() == BST_CHECKED);


    if(strText.CompareNoCase(L"Start Data Transfer") == 0)
    {
        if(m_settingCurrentInProgress == false)
        {
            // If we're saving to file check if it already exists
            // Ask user if they want to overwrite it. 
            if(PathFileExists(m_filenameRaw))
                if(GetUserQuery(m_filenameRaw) == IDNO)
                    return;

            if(PathFileExists(m_filenameCorr))
                if(GetUserQuery(m_filenameCorr) == IDNO)
                    return;

            // DeleteFile handles empty strings so no need to check 
            // the argument. If the save option isn't selected then 
            // the file name will be empty which is ok. 
            DeleteFile(m_filenameRaw);
            DeleteFile(m_filenameCorr);

            GetDlgItem(IDC_EDIT_COUNT_LIMIT)->GetWindowText(str);
            m_parser->dataDCS->countThreshold = _wtoi(str.GetString()) * 1000;

            // Start the data transfer.
            m_dataCollectionInProgress = true;
            m_btnTransfer.SetWindowText(L"Stop Data Transfer");
            Sleep(100);
            AfxBeginThread((AFX_THREADPROC)CFx3ReceiveDataDlg::CollectData, (LPVOID)this);
            DisplayAnalogData();
        }
        else
        {
            this->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->GetWindowText(str);
            this->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->SetWindowText(L"Error: Data transferring is Prohibited\r\n...during setting current" + str);
        }
    }
    else
    {
        // Stop the data transfer.
        m_btnTransfer.SetWindowText(L"Start Data Transfer");
        m_dataCollectionInProgress = false;
        SetEvent(m_hDataQueueEvent);
        if(this->m_cboTimeout.IsWindowEnabled())
            KillTimer(STOP_DATA_COLLECTION_EVENT);
    }
}




// --------------------------------------------------------------------------------------------
LRESULT CFx3ReceiveDataDlg::DataTransferComplete(WPARAM wParam, LPARAM lParam)
{
    m_btnTransfer.SetWindowText(L"Start Data Transfer");
    m_dataCollectionInProgress = false;
    SetEvent(m_hDataQueueEvent);
    return 1;
}



// --------------------------------------------------------------------------------------------
LRESULT CFx3ReceiveDataDlg::DataWriteRoutine(LPVOID lpParam)
{
    CFx3ReceiveDataDlg *pThis = (CFx3ReceiveDataDlg *)lpParam;
    PCY_DATA_BUFFER pWorkingSet = pHead;

    DWORD dwWritten = 0;
    if(pWorkingSet == NULL)
    {
        // Make sure link list head is initialized...........
        do {
            WaitForSingleObject(pThis->m_hDataQueueEvent, INFINITE);
        } while(pHead == NULL);

        // Wait till the two datas are available to write.
        if(pHead->pNextData == NULL)
            WaitForSingleObject(pThis->m_hDataQueueEvent, INFINITE);

        if(pWorkingSet == NULL)
            pWorkingSet = pHead;
    }

    ////////////////////////////////////////////////////////////////////////////////
    /////////////// Let's start the data write loop /////////////////////////////////
    while(pWorkingSet != NULL || pThis->m_dataCollectionInProgress == true)
    {
        ////////// Write the device data to the file //////////////////////////////////////////////////
        ////////// At USB super speed, there is no hard drive can handle this back pressure.///////////
        ////////// So, This operation will have overlap errors, so data integrity can't maintained.////
        WriteFile(pThis->m_hFileDataRaw, pWorkingSet->buffer, pWorkingSet->length, &dwWritten, NULL);

        /// Traverse through the link list data structure.////////////////////////
        if(pWorkingSet->pNextData == NULL)
        {
            do {
                if(pWorkingSet->pNextData == NULL)
                    WaitForSingleObject(pThis->m_hDataQueueEvent, INFINITE);

                _critSect.Lock();
                if(pWorkingSet->pNextData == NULL && pThis->m_dataCollectionInProgress == false)
                {
                    _critSect.Unlock();
                    break;
                }
                _critSect.Unlock();
            } while(pWorkingSet->pNextData == NULL);
        }

        ///////// We are good to loop for the next operation..............///////////
        _critSect.Lock();
        pHead = pHead->pNextData;
        delete pWorkingSet;
        pWorkingSet = pHead;
        _critSect.Unlock();
    }

    CloseHandle(pThis->m_hFileDataRaw);
    pThis->m_hFileDataRaw = INVALID_HANDLE_VALUE;
    SetEvent(pThis->m_hWriteCompleted);
    pHead = NULL;
    return 1;
}



// --------------------------------------------------------------------------------------------
void CFx3ReceiveDataDlg::Cleanup(CFx3ReceiveDataDlg* pThis,
    wchar_t* msg,
    void* dev,
    PUCHAR buffers)
{
    int chDlg_rate[4] ={IDC_STC_PHOTCOUNT_DET1, IDC_STC_PHOTCOUNT_DET2, IDC_STC_PHOTCOUNT_DET3, IDC_STC_PHOTCOUNT_DET4};
    int nChDlg = (sizeof(chDlg_rate) / sizeof(int));
    Fx3Device* USBDevice = (Fx3Device*)dev;

    if((msg != NULL) && (msg[0] != 0))
    {
        CString strMsg;
        strMsg.Format(msg, GetLastError());
        AfxMessageBox(strMsg);
    }

    if(buffers != NULL)
        delete buffers;

    if(USBDevice != NULL)
        delete USBDevice;


    // m_hFileDataRaw cleanup is handles in DataWriteRoutine
#if 0 
    // Critical section to avoid race condition with DataWriteRoutine
    _critSect.Lock();
    if(pThis->m_hFileDataRaw != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pThis->m_hFileDataRaw);
        pThis->m_hFileDataRaw = INVALID_HANDLE_VALUE;
    }
    _critSect.Unlock();
#endif

    if(pThis->m_hFileDataCorr != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pThis->m_hFileDataCorr);
        pThis->m_hFileDataCorr = INVALID_HANDLE_VALUE;
    }
    if(pThis->m_hFileDataAux != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pThis->m_hFileDataAux);
        pThis->m_hFileDataAux = INVALID_HANDLE_VALUE;
    }
    if(pThis->m_hFileDataInput != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pThis->m_hFileDataInput);
        pThis->m_hFileDataInput = INVALID_HANDLE_VALUE;
    }
    pThis->PostMessage(WM_DATA_TRANSFER_COMPLETE, 0, 0);

    for(int ii=0; ii < nChDlg; ii++)
        pThis->GetDlgItem(chDlg_rate[ii])->SetWindowText(L"0");

    pThis->GetDlgItem(IDC_ELAPSED_TIME)->SetWindowText(L"0");
}



// --------------------------------------------------------------------------------------------
LRESULT CFx3ReceiveDataDlg::SetCurrent(LPVOID lpParam)
{
    CFx3ReceiveDataDlg *pThis = (CFx3ReceiveDataDlg *)lpParam;
    CString entered_current, old_Str;

    // Getting the input
    pThis->GetDlgItem(IDC_CURRENT_SET_EDIT)->GetWindowText(entered_current);
    long entered_current_len = entered_current.GetLength();

    //------------------------------------------
    // Getting the old strings of the status documented in the box
    pThis->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->GetWindowText(old_Str);
    CString temp_CString;
    if(entered_current_len <= 32) {
        temp_CString.Format(L"%d", entered_current_len);
        pThis->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->SetWindowText(L"Entering " + temp_CString + L" char(s).\r\n" + old_Str);
    }
    else {
        pThis->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->SetWindowText(L"Too many digits entered. (Max 32)\r\n" + old_Str);
        return -1;
    }

    //------------------------------------------
    // Examine the entered current
    pThis->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->GetWindowText(old_Str);
    int cur_i = 0;
    PUCHAR buffers_current = new UCHAR[entered_current_len];
    if(entered_current_len < 1) {
        pThis->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->SetWindowText(L"Nothing entered.\r\n" + old_Str);
        return -1; // insufficient char input
    }
    else
    {
        for(cur_i = 0; cur_i < entered_current_len; cur_i++)
        {
            buffers_current[cur_i] = entered_current.GetAt(cur_i);
            if((entered_current.GetAt(cur_i) >= 0x30) && (entered_current.GetAt(cur_i) <= 0x39)) {}
            else {
                if(entered_current.GetAt(cur_i) == 0x2E) {}
                else {
                    pThis->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->SetWindowText(L"Invalid character(s) entered.\r\n" + old_Str);
                    return cur_i;
                }
            }
        }
    }
    pThis->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->SetWindowText(L"Start sending " + entered_current + L"\r\n" + old_Str);

    int             nCount = 0;
    long            BytesXferred = 0;
    CString         strFile = L"";
    ULONG           iteration = 0;
    PCY_DATA_BUFFER pPrevBuffer = NULL;
    int             incr = pThis->m_aux1->m_dIncrGen;
    CString         strData;
    // Tony Oct 14 2016
    bool            bXferCompleted = false;


    // Get the index to the selected FX3 device 
    int nDeviceIndex = pThis->m_cboDevices.GetCurSel();

    // Get the USB Device Instance Going.
    Fx3Device* USBDevice = new Fx3Device(nDeviceIndex, false);
    if(!(USBDevice->IsConnected()))
        return NULL;

    if(USBDevice->SetCurrent(buffers_current, entered_current_len) == 0)
    {
        CString temp_CString_4SendMessage = L"";
        int buffers_current_i;
        for(buffers_current_i = 0; buffers_current_i < entered_current_len; buffers_current_i++) {
            temp_CString_4SendMessage = temp_CString_4SendMessage + (char)buffers_current[buffers_current_i];
        }
        pThis->GetDlgItem(IDC_OLD_CURRENT_NUM)->SetWindowText(temp_CString_4SendMessage);

        pThis->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->GetWindowText(old_Str);
        pThis->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->SetWindowText(L"Sucsessfully sent to FX3 " + temp_CString_4SendMessage + L"\r\n" + old_Str);
    }
    else
    {
        pThis->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->GetWindowText(old_Str);
        pThis->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->SetWindowText(L"Error: Unable to send.\r\n" + old_Str);
    }
}



// --------------------------------------------------------------------------------------------
int CFx3ReceiveDataDlg::OpenFilesForWriting()
{
    // Set all the types of filenames 
    SetFilenames();

    // Check that filename is not empty when user selects to save
    if(m_filenameBaseFull.IsEmpty() && (m_saveraw || m_savecorr))
    {
        Cleanup(this, L"No output file selected. Please select output file and restart acquisition ...",
            NULL, NULL);
        return -1;
    }

    // We are asked to run the data collection job in the file.

    // Sanity check on the file to dump data .
    // Is the file open already?, if so please close the file.
    if(m_hFileDataRaw != INVALID_HANDLE_VALUE)
        CloseHandle(m_hFileDataRaw);
    if(m_hFileDataCorr != INVALID_HANDLE_VALUE)
        CloseHandle(m_hFileDataCorr);
    if(m_hFileDataAux != INVALID_HANDLE_VALUE)
        CloseHandle(m_hFileDataAux);

    m_hFileDataRaw = INVALID_HANDLE_VALUE;
    m_hFileDataCorr = INVALID_HANDLE_VALUE;
    m_hFileDataAux = INVALID_HANDLE_VALUE;

    // Is this operation meant for discard data or file write?
    if(!m_filenameRaw.IsEmpty())
    {
        if(!PathFileExists(m_dirnameBase.GetString()))
            CreateDirectory(m_dirnameBase, NULL);

        m_hFileDataRaw = CreateFile(m_filenameRaw, (GENERIC_READ | GENERIC_WRITE), FILE_SHARE_READ, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        // Did file creation succeed?
        if(m_hFileDataRaw == INVALID_HANDLE_VALUE)
        {
            // Some issues in creating the file.
            Cleanup(this, L"Unable to create log file. Windows OS returned error code \"0x%X\" ",
                NULL, NULL);
            return -1;
        }
    }

    if(!m_filenameCorr.IsEmpty())
    {
        if(!PathFileExists(m_dirnameBase.GetString()))
            CreateDirectory(m_dirnameBase, NULL);

        m_hFileDataCorr = CreateFile(m_filenameCorr, (GENERIC_READ | GENERIC_WRITE), FILE_SHARE_READ, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        // Did file creation succeed?
        if(m_hFileDataCorr == INVALID_HANDLE_VALUE)
        {
            // No, its not. Some issues in creating the file.
            // Lets display the windows error code from the failed file creation operation.
            Cleanup(this, L"Unable to create log file. Windows OS returned error code \"0x%X\" ",
                NULL, NULL);
            return -1;
        }
    }

    if(!m_filenameAux.IsEmpty())
    {
        if(!PathFileExists(m_dirnameBase.GetString()))
            CreateDirectory(m_dirnameBase, NULL);

        m_hFileDataAux = CreateFile(m_filenameAux, (GENERIC_READ | GENERIC_WRITE), FILE_SHARE_READ, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        // Did file creation succeed?
        if(m_hFileDataAux == INVALID_HANDLE_VALUE)
        {
            // No, its not. Some issues in creating the file.
            // Lets display the windows error code from the failed file creation operation.
            Cleanup(this, L"Unable to create log file. Windows OS returned error code \"0x%X\" ",
                NULL, NULL);
            return -1;
        }
    }
}



// --------------------------------------------------------------------------------------------
int CFx3ReceiveDataDlg::SetTimer()
{
    if(m_saveraw || m_savecorr)
    {
        // Yeah we are into doing file and we need setup a timeout.
        // Read the time and set it up.
        CString strData;

        m_cboTimeout.GetWindowText(strData);
        int nElapseTime = _wtoi(strData.GetBuffer(0));

        // Ontimer callback function will be called at the time out.
        CWnd::SetTimer(STOP_DATA_COLLECTION_EVENT, (nElapseTime * 1000), NULL);
    }
    return 0;
}




// --------------------------------------------------------------------------------------------
int CFx3ReceiveDataDlg::OpenFilesForReading()
{
    if(m_hFileDataInput == INVALID_HANDLE_VALUE)
    {
        if(!PathFileExists(m_filenameInputFull.GetString()))
        {
            // Lets display the windows error code from the failed file creation operation.
            wchar_t* errmsg = new wchar_t[512];
            wsprintf(errmsg, L"Cannot replay file trace input file %s. File doesn't exist.", m_filenameInputFull.GetString());
            Cleanup(this, errmsg, NULL, NULL);
            delete errmsg;
            return -1;
        }
        m_hFileDataInput = CreateFile(m_filenameInputFull, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if(m_hFileDataInput == INVALID_HANDLE_VALUE)
        {
            // Lets display the windows error code from the failed file creation operation.
            Cleanup(this, L"Cannot replay file trace. File failed to open.",
                NULL, NULL);
            return -1;
        }
        return 0;
    }
}




// --------------------------------------------------------------------------------------------
LRESULT CFx3ReceiveDataDlg::CollectData(LPVOID lpParam)
{
    // Allocate the arrays needed for queueing
    CFx3ReceiveDataDlg* pThis = (CFx3ReceiveDataDlg *)lpParam;
    PUCHAR			    buffers;
    unsigned short*     buffersPtr;
    int                 nCount = 0;
    long                BytesXferred = 0;
    PCY_DATA_BUFFER     pPrevBuffer = NULL;
    int                 incr = pThis->m_aux1->m_dIncrGen;
    CString             strData;
    int                 iHr = pThis->m_aux1->m_physConn - 1;
    int                 iBp = pThis->m_aux2->m_physConn - 1;
    int                 iRr = pThis->m_corrCurve->m_physConn - 1;
    int                 tFactor = pThis->m_clockSpeed / pThis->m_dRateRaw;
    int                 jj;
    unsigned long long  startTime;
    double              elapsedTime = 0;
    double              elapsedTimePrev = 0;
    wchar_t*            errMsg = NULL;

    int                 nDeviceIndex = pThis->m_cboDevices.GetCurSel();
    Fx3Device*          USBDevice = new Fx3Device(nDeviceIndex, true);
    long                totalTransferSize;

    enum InputSrc       inputsrc;

    // Set timer unconditionally whenever aquisition starts. 
    // If acquisition fails then the Start Button will be updated 
    // automatically from Stop back to Start. 
    pThis->SetTimer();

    memset(pThis->m_aux1->m_data, 0, BUF_SIZE*sizeof(MyVector));
    memset(pThis->m_aux2->m_data, 0, BUF_SIZE*sizeof(MyVector));
    pThis->m_parser->ClearBuffers();

    // Precalculate x for correlation curve
    for(int ii=0; ii < pThis->m_parser->dataDCS->nCh; ii++)
        for(jj=0; jj < pThis->m_corrdata->nbins; jj++)
            pThis->m_corrCurve->m_coord[ii]->plot.data[jj].x = log10(pThis->m_corrdata->tmean[jj]);


    // Determine source of data
    if(USBDevice->IsStreamer() && !pThis->m_replay) {
        USBDevice->SendStartNotification();
        totalTransferSize = USBDevice->GetTotalTransferSize();
        if(USBDevice->IsSuperSpeed())
            inputsrc = DeviceSuperSpeed;
        else
            inputsrc = DeviceNormalSpeed;
    }
    else if(pThis->OpenFilesForReading() == 0) {
        totalTransferSize = pThis->m_maxPktSizeDefault * PACKETS_PER_TRANSFER;
        inputsrc = File;
    }
    else
        inputsrc = None;

    if(inputsrc == None)
    {
        Cleanup(pThis, L"Error: No data source selected. ", USBDevice, buffers);
        return -1;
    }

    // Open files for saving data if save was selected
    if(pThis->OpenFilesForWriting() < 0)
        return -1;

    buffers = new UCHAR[totalTransferSize];
    buffersPtr = (unsigned short*)buffers;

    // Launch data write thread to the disk for super speed operation...
    if(pThis->m_hFileDataRaw != INVALID_HANDLE_VALUE)
        AfxBeginThread((AFX_THREADPROC)DataWriteRoutine, (LPVOID)pThis);

    // Launch autocorrelation/display thread
    AfxBeginThread((AFX_THREADPROC)BloodFlowMonitor, (LPVOID)pThis);

    // Get the start time of the measurement. This won't change 
    // until the current thread termintas signaling the end of the 
    // measurement. 
    startTime = getCurrTime(NULL);

    ////////////////////////////////////////////////////////////////////////
    // Start the data collection phase..................
    ///////////////////////////////////////////////////////////////////////
    memset(buffers, 0x00, totalTransferSize);
    while(pThis->m_dataCollectionInProgress == true)
    {

        // Get data from whichever src has data available
        if(inputsrc & DeviceAny) {
            // Get data from device
            if(USBDevice->RecieveData(buffers) < 0) {
                errMsg = USBDevice->GetErrorMsg();
                break;
            }
        }
        else {
            // Read the data from the file 
            DWORD nRead;
            if(ReadFile(pThis->m_hFileDataInput, buffers, totalTransferSize, &nRead, NULL) == 0)
                break;

            // When replaying data trace we want to slow down the rate at which 
            // we acquire and display it.
            Sleep(20);
        }

        BytesXferred += totalTransferSize;

        // BytesXFerred is needed for current data rate calculation.
        if(BytesXferred < 0) // Rollover - reset counters
            BytesXferred = 0;

        // Parse data into separate data streams. Tell parser how big the raw data buffer 
        // is (buffersPtr) so it knows how much to allocate for it's own data which holds 
        // the counts. 
        pThis->m_parser->parseData(buffersPtr, totalTransferSize / sizeof(unsigned short));

        // Copy data into display buffer
        for(int jj=0; jj < pThis->m_parser->dataADC->count; jj+=incr)
        {
            // for(int kk = 0; kk < pThis->m_parser->nCh_adc; kk++)
            if(pThis->m_count > 0)
            {
                pThis->m_aux1->m_data[pThis->m_count].x =
                    pThis->m_parser->dataADC->t[jj] / tFactor;
                pThis->m_aux2->m_data[pThis->m_count].x =
                    pThis->m_parser->dataADC->t[jj] / tFactor;
            }
            pThis->m_aux1->m_data[pThis->m_count].y = (double)(pThis->m_parser->dataADC->amp[iHr][jj]);
            pThis->m_aux2->m_data[pThis->m_count].y = (double)(pThis->m_parser->dataADC->amp[iBp][jj]);

            pThis->m_count++;

            if(pThis->m_count >= BUF_SIZE)
            {
                memset(pThis->m_aux1->m_data, 0, BUF_SIZE*sizeof(MyVector));
                memset(pThis->m_aux2->m_data, 0, BUF_SIZE*sizeof(MyVector));
                pThis->m_count = 0;
                pThis->m_bufferWrap = true;
            }
        }


        ///////////////////////////////////////////////////////////
        /// Time to write the data to the file, if needed /////////
        ///////////////////////////////////////////////////////////
        DWORD dwWrite = 0;
        if(pThis->m_hFileDataRaw != INVALID_HANDLE_VALUE) 
        {
            ///////////File writting is valid, so push the data to the file.////////////
            //// Is this device super speed device?
            if(inputsrc & DeviceSuperspeedOrFile) {
                ///////////////////////////////////////////////////////
                /// Don't block the data transfer by write here.
                /// Let's push the data to a link list and 
                /// the disk write thread will push the data to the file.
                /////////////////////////////////////////////////////

                _critSect.Lock();
                CY_DATA_BUFFER *pBuffer = new CY_DATA_BUFFER;
                pBuffer->pNextData = NULL;

                pBuffer->buffer = buffers;
                pBuffer->length = totalTransferSize;

                if(pPrevBuffer == NULL)
                    pPrevBuffer = pBuffer;
                else
                {
                    pPrevBuffer->pNextData = pBuffer;
                    pPrevBuffer = pBuffer;
                }

                if(pHead == NULL)
                    pHead = pBuffer;
                else
                    SetEvent(pThis->m_hDataQueueEvent);
                _critSect.Unlock();

                ///////////////////Link List Population completes///////////
            }
            else {
                ///// For non super speed devices, write the data. We have lots of time.
                WriteFile(pThis->m_hFileDataRaw, buffers, totalTransferSize, &dwWrite, NULL);
            }
        }
        else if(pThis->m_hFileDataAux != INVALID_HANDLE_VALUE)
        {
            int nCh = pThis->m_parser->dataADC->nCh;
            unsigned long count = pThis->m_parser->dataADC->count;
            unsigned long long* t = pThis->m_parser->dataADC->t;
            unsigned long long** amp = pThis->m_parser->dataADC->amp;

            ///// For non super speed devices, write the data. We have lots of time.

            if(pThis->m_parser->dataADC->countTotal == count)
                WriteFile(pThis->m_hFileDataAux, &nCh, sizeof(nCh), &dwWrite, NULL);
            WriteFile(pThis->m_hFileDataAux, &count, sizeof(count), &dwWrite, NULL);
            WriteFile(pThis->m_hFileDataAux, t, count*sizeof(t[0]), &dwWrite, NULL);
            for(int ii=0; ii < nCh; ii++)
                WriteFile(pThis->m_hFileDataAux, amp[ii], count*sizeof(amp[ii][0]), &dwWrite, NULL);
        }


        // Get elapsed time every 100 data transfers
        elapsedTime = getElapsedTime(startTime, 1e3);
        if((elapsedTime - elapsedTimePrev) >= 1.0)
        {
            // Display elapsed time every half second
            strData.Format(L"%0.1f", elapsedTime);
            pThis->GetDlgItem(IDC_ELAPSED_TIME)->SetWindowText(strData);

            strData.Format(L"%0.0f", (double)BytesXferred / 1000.0);
            pThis->GetDlgItem(IDC_DATA_RATE)->SetWindowText(strData);

            elapsedTimePrev = elapsedTime;
            BytesXferred = 0;
            nCount = 0;
        }
        nCount++;
    }

    ///////////////////////////////////////////////////////////////
    // Alright, we out of data collection loop./////////
    // Stop can happen from User pressing the button or
    // Time out can trigger the exit. //////////////////

    ////////////////////////////////////////////////////////////////////////////
    /////// Send stop notification to the device //////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    if(inputsrc & DeviceAny) {
        USBDevice->SendStopNotification();

        // Increment run for new filename 
        pThis->GetNewBaseFilename();
    }

    // Perform Memory cleanup now
    Cleanup(pThis, errMsg, USBDevice, buffers);

    // Reset all displays to zeros
    return 0;
}



// --------------------------------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnTimer(UINT_PTR nIDEvent)
{
    if(nIDEvent == STOP_DATA_COLLECTION_EVENT)
    {
        m_dataCollectionInProgress = false;
        m_btnTransfer.SetWindowText(L"Start Data Transfer");
        SetEvent(m_hDataQueueEvent);
        KillTimer(nIDEvent);
        return;
    }
    CDialogEx::OnTimer(nIDEvent);
}



// --------------------------------------------------------------------------------------------
LRESULT CFx3ReceiveDataDlg::DataTransferStarted(WPARAM wParam, LPARAM lParam)
{
    m_pInfoMessage = new CDlgWait(this);
    m_pInfoMessage->Create(CDlgWait::IDD, GetDesktopWindow());
    m_pInfoMessage->ShowWindow(SW_SHOW);

    return 1;
}


// --------------------------------------------------------------------------------------------
int CFx3ReceiveDataDlg::DisplayCorrCurve(unsigned long long* startTime)
{
    CString strData;
    int nChDlg = (sizeof(m_chDlg_rate) / sizeof(int));
    BOOL overflowflag = false;
    bool* dataInputInProgress;
    LightIntensityData* dataDCS = m_parser->dataDCS;
    int buffsize = dataDCS->maxsize - (dataDCS->maxsize / 100);
    int countTotal[50];
    int countTotalDropped[50];
    int countTotalAll[50];
    double unit = 1;
    double elapsedTime = getElapsedTime(*startTime, unit);
    int nbytestowrite = sizeof(m_corrdata->gmean[0][0]) * m_corrdata->nbins;
    DWORD nbyteswritten;
    unsigned long long corrStartTime;
    double corrElapsedTime;
    CString strBFi;
    int numChBFi = 0;

    if(elapsedTime >= 1000.0)
    {
        MyCoord** p = m_corrCurve->m_coord;
        int jj;
        double unit = 1;

        memcpy(countTotal, dataDCS->countTotal, dataDCS->nCh * sizeof(int));
        memcpy(countTotalDropped, dataDCS->countTotalDropped, dataDCS->nCh * sizeof(int));
        for(int ii=0; ii<dataDCS->nCh; ii++)
            countTotalAll[ii] = countTotal[ii] + countTotalDropped[ii];

        int elapsedTimeInSec2 = getElapsedTime(*startTime, 1);

        // Generate correlation data
        corrStartTime = getCurrTime(NULL);
        for(int ii=0; ii < dataDCS->nCh; ii++)
        {
            m_corrdata->calcAutocorrCurve(dataDCS->arrtimes[ii][0], countTotal[ii], ii);
            m_corrdata->calcBloodFlowIndex(ii, &(m_BFi[ii]), &(m_beta[ii]), &(m_err[ii]));
        }
        corrElapsedTime = getElapsedTime(corrStartTime, unit);

        // Display time elapsed since start of last correlation
        strData.Format(L"%0.3f", corrElapsedTime / 1000.0);
        GetDlgItem(IDC_CORR_ELAPSED_TIME)->SetWindowText(strData);

        m_ctrl3->clearDisplay(p[0]);

        for(int ii=0; ii<dataDCS->nCh; ii++) {

            // Display photon count 
            strData.Format(L"%d", (int)(((double)countTotalAll[ii] / 1000.0) / (elapsedTime * unit / 1000.0)));
            GetDlgItem(m_chDlg_rate[ii])->SetWindowText(strData);

            // Display BFi and beta
            if(countTotal[ii] > 1000 && m_BFi[ii] > 0 && m_beta[ii] > 0)
                strBFi.Format(_T("%0.3g,  %0.3f "), m_BFi[ii], m_beta[ii]);
            else
                strBFi.Format(_T("0.0,  0.0"));

            GetDlgItem(m_chDlg_BFi[ii])->SetWindowText(strBFi);


            if(countTotal[ii] > 0)
            {
                if(m_enableDet[ii])
                {
                    for(jj=0; jj < m_corrdata->nbins; jj++)
                        p[ii]->plot.data[jj].y = m_corrdata->gmean[ii][jj];
                    m_ctrl3->plotData(p[ii], jj - 1, ii);
                }

                // Reset photon counts
                dataDCS->countTotal[ii] = 0;
            }

            dataDCS->countTotalDropped[ii] = 0;

            if(m_hFileDataCorr != INVALID_HANDLE_VALUE)
                WriteFile(m_hFileDataCorr, m_corrdata->gmean[ii], nbytestowrite, &nbyteswritten, NULL);
        }


        // Reset start time
        *startTime = getCurrTime(NULL);
    }
    return 0;
}



// --------------------------------------------------------------------------------------------
void BloodFlowMonitor(LPVOID lpParam)
{
    // Allocate the arrays needed for queueing
    CFx3ReceiveDataDlg *pThis = (CFx3ReceiveDataDlg*)lpParam;
    int nChDlg = (sizeof(pThis->m_chDlg_rate) / sizeof(int));
    int nbytestowrite = sizeof(pThis->m_corrdata->tmean[0]) * pThis->m_corrdata->nbins;
    DWORD nbyteswritten;
    CString strBFi;

    // Initialize BFi and beta display to all zeros
    for(int ii=0; ii < pThis->m_parser->dataDCS->nCh; ii++) {
        strBFi.Format(_T("0.0,  0.0"));
        pThis->GetDlgItem(pThis->m_chDlg_BFi[ii])->SetWindowText(strBFi);
    }

    // If user chose to save the data, then write the file header: 
    //    a) Size of function
    //    b) Number of channels
    //    c) Delays 
    if(pThis->m_hFileDataCorr != INVALID_HANDLE_VALUE)
    {
        WriteFile(pThis->m_hFileDataCorr, &pThis->m_corrdata->nbins, sizeof(int), &nbyteswritten, NULL);
        WriteFile(pThis->m_hFileDataCorr, &pThis->m_corrdata->nCh, sizeof(int), &nbyteswritten, NULL);
        WriteFile(pThis->m_hFileDataCorr, pThis->m_corrdata->tmean, nbytestowrite, &nbyteswritten, NULL);
    }

    // Get start time and then display correlation curve every few milliseconds
    unsigned long long startTime = getCurrTime(NULL);
    while(pThis->m_dataCollectionInProgress)
    {
        pThis->DisplayCorrCurve(&startTime);
        Sleep(10);
    }

    // Cleanup displays now
    pThis->m_ctrl3->clearDisplay(pThis->m_corrCurve->m_coord[0]);
    // Cleanup(pThis, L"", NULL, NULL);
}



// ------------------------------------------------------------------------
static void* AnalogDataMonitor(LPVOID lpThreadParameter)
{
    CFx3ReceiveDataDlg* dlg = (CFx3ReceiveDataDlg*)lpThreadParameter;
    DataStreamPlot* devDisp1 = dlg->m_aux1;
    DataStreamPlot* devDisp2 = dlg->m_aux2;

    MyCoord* p1 = devDisp1->m_coord[0];
    MyVector* data1_0 = devDisp1->m_data;
    MyVector* data1 = devDisp1->m_data;

    MyCoord* p2 = devDisp2->m_coord[0];
    MyVector* data2_0 = devDisp2->m_data;
    MyVector* data2 = devDisp2->m_data;

    int dStep = devDisp1->m_dStep;
    int dRate = devDisp1->m_dRateRaw;
    int incr = ceil(devDisp1->m_dIncrDisp);

    // Increase update rate by about 40% over the data rate to make sure 
    // we keep up with incoming data. The display lags behind slightly 
    // (which increases with time) when sleeptime = 1/m_updateRate*1000
    // because displaying itself also uses up a few milliseconds. 
    int sleeptime = (int)(.30 / devDisp1->m_updateRate*1000.0);

    int ii = 0;
    int jj = 0;
    int nDispMoves = 0;
    int c = 0;
    CString strData;

    dlg->m_ctrl1->clearDisplay(p1, dRate);
    dlg->m_ctrl2->clearDisplay(p2, dRate);
    while(dlg->m_dataCollectionInProgress)
    {
        for(jj=0, ii=0; jj < devDisp1->m_dWidthPlot; jj++, ii=ii+incr)
        {
            // Check if we displayed all the points available
            if((nDispMoves*dStep + ii) >= (dlg->m_count - 1))
                break;

            p1->plot.data[jj].y = data1[ii].y * ANALOG_RAW_TO_VOLTS;
            p1->plot.data[jj].x = data1[ii].x;
            p2->plot.data[jj].y = data2[ii].y * ANALOG_RAW_TO_VOLTS;
            p2->plot.data[jj].x = data2[ii].x;

            // Calculate y mean and y max/min of current plot
            p1->plot.CalcStats(jj);
            p2->plot.CalcStats(jj);
        }

        dlg->m_ctrl1->plotData(p1, jj-1, (double)dRate);
        dlg->m_ctrl2->plotData(p2, jj-1, (double)dRate);

        Sleep(sleeptime);

        if(((data1 + dStep) - data1_0 >= BUF_SIZE) || dlg->m_bufferWrap == true)
        {
            data1 = data1_0;
            data2 = data2_0;
            nDispMoves = 0;
            dlg->m_bufferWrap = false;
            dlg->m_ctrl1->clearDisplay(p1, jj-1, dRate);
            dlg->m_ctrl2->clearDisplay(p2, jj-1, dRate);
            Sleep(1);
        }

        if(jj >= dlg->m_aux1->m_dWidthPlot)
        {
            // Make sure not to overstep data buffer
            data1 = data1 + dStep;
            data2 = data2 + dStep;
            nDispMoves++;
            dlg->m_ctrl1->clearDisplay(p1, jj-1, dRate);
            dlg->m_ctrl2->clearDisplay(p2, jj-1, dRate);
            Sleep(1);
        }

        p1->plot.ResetStats();
        p2->plot.ResetStats();
    }

    p1->ResetAxes(dRate);
    p2->ResetAxes(dRate);

    dlg->m_ctrl1->clearDisplay(p1, dRate);
    dlg->m_ctrl2->clearDisplay(p2, dRate);

    return 0;
}





// -----------------------------------------------------------------
int CFx3ReceiveDataDlg::DisplayAnalogData()
{
    HANDLE  hThread;
    int		status;

    m_count = 0;
    m_bufferWrap = false;

    hThread = CreateThread(NULL,                       // default security attributes
                           0,                          // use default stack size  
                           (LPTHREAD_START_ROUTINE)AnalogDataMonitor, // thread function name
                           this,                       // argument to thread function 
                           0,                          // use default creation flags 
                           NULL);                      // returns the thread identifier 

    if(hThread == NULL)
        status = 0;
    else
        status = 1;

    return status;
}




// -------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedCheckAutozoom()
{
    CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_AUTOZOOM);
    m_aux1->m_coord[0]->plot.SetAutozoom(pBtn->GetCheck());
    m_aux2->m_coord[0]->plot.SetAutozoom(pBtn->GetCheck());
    for(int ii=0; ii < 4; ii++)
        m_corrCurve->m_coord[ii]->plot.SetAutozoom(pBtn->GetCheck());
}


// -------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedPhotCountDet1Checkbox()
{
    if(m_enableDet[0])
        m_enableDet[0] = 0;
    else
        m_enableDet[0] = 1;
}


// -------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedPhotCountDet2Checkbox()
{
    if(m_enableDet[1])
        m_enableDet[1] = 0;
    else
        m_enableDet[1] = 1;
}



// -------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedPhotCountDet3Checkbox()
{
    if(m_enableDet[2])
        m_enableDet[2] = 0;
    else
        m_enableDet[2] = 1;
}



// -------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedPhotCountDet4Checkbox()
{
    if(m_enableDet[3])
        m_enableDet[3] = 0;
    else
        m_enableDet[3] = 1;
}



// -------------------------------------------------------------------
LRESULT CFx3ReceiveDataDlg::Warning_ClickDuringMeasurement(LPVOID lpParam)
{
    CFx3ReceiveDataDlg *pThis = (CFx3ReceiveDataDlg *)lpParam;
    CString old_Str;
    pThis->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->GetWindowText(old_Str);
    pThis->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->SetWindowText(L"Error: Current won't be set during measurement.\r\n" + old_Str);
    return 0;
}




// -------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedBtnSendCmd2setcurrent()
{
    // IDC_BTN_SEND_CMD_2SETCURRENT clicked
    //if () {

    //}
    // TODO: Add your control notification handler code here
    CString strText;
    CString old_Str;
    CString filename;
    bool is_not_save2file;
    m_btnTransfer.GetWindowText(strText);

    // if not transfering data
    is_not_save2file = (((CButton *)GetDlgItem(IDC_RAD_SAVE_RAW_DATA))->GetCheck() == BST_CHECKED);
    if((strText.CompareNoCase(L"Start Data Transfer") == false) || (is_not_save2file))
    {
        if(m_settingCurrentInProgress == false)
        {
            m_settingCurrentInProgress = true;
            AfxBeginThread((AFX_THREADPROC)CFx3ReceiveDataDlg::SetCurrent, (LPVOID)this);
            m_settingCurrentInProgress = false;
        }
        else
        {
            this->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->GetWindowText(old_Str);
            this->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->SetWindowText(L"Error: Current can not be set\r\n...current setting is still under processing.\r\n" + old_Str);
        }
    }
    else
    {
        this->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->GetWindowText(old_Str);
        this->GetDlgItem(IDC_STC_STATUS_SETCURRENT)->SetWindowText(L"Error: Current can not be set\r\n...during Data Transferring.\r\n" + old_Str);
    }
}




// -------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedButtonDelayThreshold()
{
    double min_x;
    double max_x;
    CString str0;
    CString str1;
    CString str2;
    LPCWSTR str1P;
    LPCWSTR str2P;
    MyVector min3(log10(m_corrdata->tmean[1]), .99);
    MyCoord** p = m_corrCurve->m_coord;
    LightIntensityData* dataDCS = m_parser->dataDCS;
    int ii;

    GetDlgItem(IDC_EDIT_DELAY_THRESHOLD)->GetWindowText(str0);
    str1 = str0;

    // Get lower limit
    str1P = str1.GetString();
    if(str1P==NULL) {
        GetDlgItem(IDC_EDIT_DELAY_THRESHOLD)->SetWindowText(str0);
        return;
    }
    min_x = (double)_wtof(str1P);

    // Get upper limit
    for(ii=0; str1P[ii]!=0; ii++)
        if(str1P[ii]==' ' || str1P[ii]==',')
            break;
    str2P = &(str1P[ii+1]);
    if(str2P!=NULL)
        max_x = (double)_wtof(str2P);
    else
        max_x = pow(10.0, p[0]->max.x);

    GetDlgItem(IDC_EDIT_COUNT_LIMIT)->GetWindowText(str1);
    m_parser->dataDCS->countThreshold = _wtoi(str1.GetString()) * 1000;

    // Error checking
    bool errflag = false;
    if(min_x < m_corrdata->tmean[1])
        errflag = true;
    if(min_x > m_corrdata->tmean[m_corrdata->nbins-1])
        errflag = true;
    if(max_x < m_corrdata->tmean[1])
        errflag = true;
    if(min_x >= max_x)
        errflag = true;
    if(errflag==true) {
        GetDlgItem(IDC_EDIT_DELAY_THRESHOLD)->SetWindowText(str0);
        return;
    }

    // Set the x axis min in the correlation display to the new time bin 
    for(int kk=0; kk < dataDCS->nCh; kk++)
    {
        MyVector min0(log10(min_x), p[kk]->min.y);
        MyVector max0(log10(max_x), p[kk]->max.y);

        p[kk]->ResetXMaxMin(min0, max0);
        m_ctrl3->clearDisplay(p[kk]);
    }
}



// -----------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedBtnSubmit()
{
	if(m_replay) {
		m_editFolder.GetWindowText(m_dirnameInput);
		m_editFile.GetWindowText(m_filenameInput);
		m_filenameInputFull.Format(_T("%s\\%s"), m_dirnameInput.GetString(), m_filenameBase.GetString());
	}
	else {
		GetNewBaseFilename();

		m_filenameBaseFull.Format(_T("%s\\%s"),m_dirnameBase.GetString(),m_filenameBase.GetString());

		// Set all the rest of the different types of file names used for saving data
		SetFilenames();
	}
}


// -------------------------------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedRadReplayExistFile()
{
    m_replay = !(((CButton *)GetDlgItem(IDC_RAD_REPLAY_EXIST_DATA))->GetCheck() == BST_CHECKED);
    ((CButton *)GetDlgItem(IDC_RAD_REPLAY_EXIST_DATA))->SetCheck(m_replay);

    EnableSavePanelReplayObjs(m_replay);

    if(m_replay) {
        ((CButton *)GetDlgItem(IDC_RAD_SAVE_RAW_DATA))->SetCheck(0);
        ((CButton *)GetDlgItem(IDC_RAD_SAVE_CORR_DATA))->SetCheck(0);
        m_saveraw = false;
        m_savecorr = false;
    }
}



// -------------------------------------------------------------------------------------------
void CFx3ReceiveDataDlg::OnBnClickedBtnBrowseFile()
{
    // TODO: Add your control notification handler code here
    TCHAR szFilters[] = _T("Saved data (*.*)");
    CFileDialog fileDlg(FALSE, NULL, NULL, 0, NULL);
    if(fileDlg.DoModal() != IDOK)
        return;
    CString file = fileDlg.GetFileName();
    CString path = fileDlg.GetFolderPath();
    m_editFile.SetWindowText(file);

	m_dirnameInput = path.GetString();
	m_filenameInput = file.GetString();
	m_filenameInputFull.Format(_T("%s\\%s"), path.GetString(), file.GetString());
}


