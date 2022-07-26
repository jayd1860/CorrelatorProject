
// Fx3ReceiveDataDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "DlgWait.h"
#include "Fx3DataParser.h"
#include "Coord.h"
#include "CorrFunc.h"


#define WM_DATA_TRANSFER_COMPLETE   (WM_USER + 100)
#define WM_DATA_TRANSFER_STARTED    (WM_USER + 103)

typedef struct _CY_DATA_BUFFER {

	UCHAR *buffer;                      /*Pointer to the buffer from where the data is read/written */
	UINT32 length;                      /*Length of the buffer */
	_CY_DATA_BUFFER *pNextData;
} CY_DATA_BUFFER, *PCY_DATA_BUFFER;


#define BUF_SIZE 2000000

enum InputSrc
{
    DeviceSuperSpeed  = 1,
    DeviceNormalSpeed = 2,
    DeviceAny = DeviceSuperSpeed | DeviceNormalSpeed,
    File = 4,
    DeviceSuperspeedOrFile = DeviceSuperSpeed | File,
    None = 8
};

struct DataStreamPlot
{
	// Data simulation params
	DataStreamPlot(double tWidth, 
		           double dRateRaw, 
		           double dRateGen, 
		           double dRatePlot, 
		           MyVector origin, 
		           MyVector min0, 
		           MyVector max0, 
		           int nbins, 
		           int connector, 
		           int nplots)
	{
		m_data = new MyVector[BUF_SIZE];

		m_tWidth = tWidth;                  // Display window time width in seconds
		m_dRateRaw = dRateRaw;                    // The data generator speed in data points per second.
		m_dRateGen = dRateGen;                    // The data generator speed in data points per second.
		m_dRatePlot = dRatePlot;                    // The data generator speed in data points per second.
		m_dWidthRaw = m_tWidth * m_dRateRaw;  // Display window data width in number of data points
		m_dWidthGen = m_tWidth * m_dRateGen;  // Display window data width in number of data points
		m_dWidthPlot = m_tWidth * m_dRatePlot;  // Display window data width in number of data points
		m_updateRate = 1;       // Update rate in updates per seconds
		m_tStep = 1 / m_updateRate;                    // Step width in seconds
		m_dStep = ceil(m_tStep * m_dRateGen);    // Step width in data points
		m_dIncrGen = floor(m_dRateRaw / m_dRateGen);
		m_dIncrDisp = floor(m_dRateGen / m_dRatePlot);

		MyVector offset(0,0);

		for(int ii=0; ii<nplots; ii++)
		{
			if(nbins > 0)
				m_coord[ii] = new MyCoord(origin, min0, max0, positive, positive, offset, nbins);
			else
				m_coord[ii] = new MyCoord(origin, min0, max0, positive, positive, offset, m_dRateRaw);
		}
		m_physConn = connector;
	}


	~DataStreamPlot()
	{
		delete m_data;
	}

	MyVector* m_data;
	//int m_count;
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
	//bool  m_bufferWrap;
	int   m_physConn;
	MyCoord* m_coord[10];
};



// CFx3ReceiveDataDlg dialog
class CFx3ReceiveDataDlg : public CDialogEx
{
	// Construction
public:
	CFx3ReceiveDataDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FX3RECEIVEDATA_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	HDEVNOTIFY *m_hDeviceNotify;

	// Tony Oct 17 2016
	virtual void OnOK();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT DataTransferComplete(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT DataTransferStarted(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
	static LRESULT CollectData(LPVOID lpParam);
	static LRESULT SetCurrent(LPVOID lpParam);
	static LRESULT Warning_ClickDuringMeasurement(LPVOID lpParam);
	static LRESULT DataWriteRoutine(LPVOID lpParam);
	static void Cleanup(CFx3ReceiveDataDlg* pThis,
		wchar_t* msg,
		void* USBDevice,
		PUCHAR buffers);

private:
	int SetFilenames(); 
	int OpenFilesForWriting();
	int OpenFilesForReading();
    int GetNewRunNumber();
    int GetRunNumberFromGui(CString str);
    int GetFilenamePrefixFromGui(CString* prefix, CString editstr);
    int GetNewBaseFilename();
	int SetTimer();
    int DisplayAnalogData();
	int GetFirstFilename(CString* filename);

public:
    static CCriticalSection _critSect;
	static PCY_DATA_BUFFER pHead;

public:
	CWinFormsControl<WindowsFormsControlLibrary1::WindowsFormsControlLibrary1Control> m_ctrl1;
	CWinFormsControl<WindowsFormsControlLibrary1::WindowsFormsControlLibrary1Control> m_ctrl2;
	CWinFormsControl<WindowsFormsControlLibrary1::WindowsFormsControlLibrary1Control> m_ctrl3;
	afx_msg void OnBnClickedStartDataTransfer();
	afx_msg void OnBnClickedBtnFile();
	afx_msg void OnBnClickedRadSaveRawData();

	BOOL AddDevicesToDropdownMenu();
	BOOL DisplayStats(SYSTEMTIME* startTime, LightIntensityData* dataDCS, long* bytesXferred, bool overflowflag);
	int DisplayCorrCurve(unsigned long long* startTime);

public:

	afx_msg void OnBnClickedReplayFileTrace();
	afx_msg void OnBnClickedButtonDevConfig();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd *pWnd, UINT nCtlColor);

	afx_msg void OnBnClickedPhotCountDet1Checkbox();
	afx_msg void OnBnClickedPhotCountDet2Checkbox();
	afx_msg void OnBnClickedPhotCountDet3Checkbox();
	afx_msg void OnBnClickedPhotCountDet4Checkbox();
	afx_msg void OnBnClickedCheckAutozoom();

	afx_msg void OnBnClickedPhotCountDetCheckbox();
	afx_msg void OnBnClickedBtnSendCmd2setcurrent();

	afx_msg void OnBnClickedButtonDelayThreshold();
	afx_msg void OnBnClickedRadSaveCorrData();

	void EnableSavePanelObjs(bool enable);
    void EnableSavePanelReplayObjs(bool enable);

	CComboBox m_cboDevices;
	CEdit m_editFolder;
	CEdit m_editFile;
	CButton m_radCounter;
	CComboBox m_cboTimeout;
	CButton m_btnTransfer;
	CDlgWait* m_pInfoMessage;
	HANDLE m_hDataQueueEvent;

	HANDLE m_hWriteCompleted;

	HANDLE m_hFileDataRaw = INVALID_HANDLE_VALUE;
	HANDLE m_hFileDataAux = INVALID_HANDLE_VALUE;
	HANDLE m_hFileDataCorr = INVALID_HANDLE_VALUE;
    HANDLE m_hFileDataInput = INVALID_HANDLE_VALUE;

	CString m_dirnameRoot;
	CString m_dirnameBase;
	CString m_filenameBase;
	int m_nRun;
	CString m_filenameBaseFull;
	CString m_filenameRaw;
	CString m_filenameAux;
	CString m_filenameCorr;

	CString m_dirnameInput;
	CString m_filenameInput;
	CString m_filenameInputFull;

	Fx3DataParser* m_parser;

	DataStreamPlot* m_aux1;
	DataStreamPlot* m_aux2;
	DataStreamPlot* m_corrCurve;
	
	CorrFunc* m_corrdata;
	CButton m_btnTraceReplay;

	CBrush m_redbrush;
	CBrush m_greenbrush;
	CBrush m_bluebrush;
	CBrush m_cyanbrush;
	CBrush m_magentabrush;
	COLORREF m_redcolor;
	COLORREF m_greencolor;
	COLORREF m_bluecolor;
	COLORREF m_cyancolor;
	COLORREF m_magentacolor;
	COLORREF m_textcolor;

	bool m_configInProgress;
	bool m_autozoom;
	long m_maxPktSizeDefault;

	bool m_dataCollectionInProgress = false;
	bool m_settingCurrentInProgress = false;

	int m_count;
	bool m_bufferWrap;
	double m_clockSpeed;
	double m_dRateRaw;
	double m_dRateGen;
	int m_segSize;
	int m_enableDet[4];
	bool m_saveraw;
    bool m_savecorr;
    bool m_replay;

	double* m_BFi;
	double* m_beta; 
	double* m_err;

	int m_chDlg_rate[4];
	int m_chDlg_BFi[4];

	afx_msg void OnBnClickedBtnSubmit();
    afx_msg void OnBnClickedRadReplayExistFile();
    afx_msg void OnBnClickedBtnBrowseFile();
};
