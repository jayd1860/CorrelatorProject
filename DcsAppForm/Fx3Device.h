#pragma once
#include "cyapi.h"

#define     TIMEOUT_PER_TRANSFER_MILLI_SEC      1500
#define     PACKETS_PER_TRANSFER                8


class Fx3Device
{
public:

	Fx3Device();
    Fx3Device(int devIdx, bool input);
    ~Fx3Device();


    // ---------------------------------------------------------------
    bool IsConnected()
    {
        bool r = false;

        if(m_USBDevice==NULL)
            return r;

        if(m_USBDevice->DeviceCount()<1)
            return r;

        if(!strcmp(m_USBDevice->DeviceName, "WestBridge "))
            r = true;

        if(!strcmp(m_USBDevice->FriendlyName, "Cypress FX3 USB BootLoader Device"))
            r = true;

        if(!strcmp(m_USBDevice->DeviceName, "Streamer Device"))
            r = true;

        if(!strcmp(m_USBDevice->FriendlyName, "Cypress FX3 USB StreamerExample Device"))
            r = true;

        return r;
    }


    // ---------------------------------------------------------------
    bool IsBootLoader()
    {
        bool r = false;

        if(!strcmp(m_USBDevice->FriendlyName, "Cypress FX3 USB BootLoader Device"))
            r = true;

        return r;
    }


    // ---------------------------------------------------------------
    int GetDeviceCount()
    {
        return m_deviceCount;
    }


    // ---------------------------------------------------------------
    int GetTotalTransferSize()
    {
        return m_totalTransferSize;
    }


    // ---------------------------------------------------------------
	wchar_t** GetDeviceNames()
    {
		return m_names;
    }



    // ---------------------------------------------------------------
    wchar_t* GetErrorMsg()
    {
        return m_errStr;
    }


    
        
    // ---------------------------------------------------------------
    void ConvertFriendlyName()
    {
        if(!strcmp(m_USBDevice->FriendlyName, "Cypress FX3 USB BootLoader Device"))
            sprintf(m_FriendlyName, "Boot Loader");

        if(!strcmp(m_USBDevice->DeviceName, "Streamer Device"))
            sprintf(m_FriendlyName, "Streamer");

        if(!strcmp(m_USBDevice->FriendlyName, "Cypress FX3 USB StreamerExample Device"))
            sprintf(m_FriendlyName, "Streamer");
    }



    // ---------------------------------------------------------------
    bool IsStreamer()
    {
        bool r = false;

        if(!IsConnected())
            return false;

        if(!strcmp(m_USBDevice->DeviceName, "Streamer Device"))
            r = true;

        if(!strcmp(m_USBDevice->FriendlyName, "Cypress FX3 USB StreamerExample Device"))
            r = true;

        return r;
    }



    int SendStartNotification();
    int SendStopNotification();

    int SetCurrent(PUCHAR buffer, long length);


    // ------------------------------------------------------------------------------
    _inline int Fx3Device::RecieveData(PUCHAR buffer)
    {
        long readlen = m_totalTransferSize;

        // Re-submit this queue element to keep the queue full
        m_contexts = m_endPoint->BeginDataXfer(buffer, m_totalTransferSize, m_inOvLap);
        if(m_endPoint->NtStatus || m_endPoint->UsbdStatus)
        {
            // BeginDataXfer failed. Time to bail out now....
            wcsncpy_s(m_errStr, L"Error queuing the application buffer now, bailing out from data collection routine...",
                sizeof(L"Error queuing the application buffer now, bailing out from data collection routine..."));
            return -1;
        }

        //////////Wait till the transfer completion..///////////////////////////
        if(!m_endPoint->WaitForXfer(m_inOvLap, TIMEOUT_PER_TRANSFER_MILLI_SEC))
        {
            m_endPoint->Abort();
            if(m_endPoint->LastError == ERROR_IO_PENDING)
                WaitForSingleObject(m_inOvLap->hEvent, TIMEOUT_PER_TRANSFER_MILLI_SEC);
        }

        // Read the trasnferred data from the device
        if(!(m_endPoint->FinishDataXfer(buffer, readlen, m_inOvLap, m_contexts)))
            return -1;

        return 0;
    }

    bool IsSuperSpeed();


	int GetClockSpeed()
	{
		return m_clockSpeed;
	}

	int GetDataRate()
	{
		return m_dRate;
	}

public:

    long				m_maxPktSizeDefault;
    long				m_totalTransferSize;
	wchar_t**			m_names;
    int					m_deviceCount;
    int             	m_endPointCount;
    CCyUSBDevice*       m_USBDevice;
    CCyUSBEndPoint*     m_endPoint;
    PUCHAR			    m_contexts;
    OVERLAPPED*		    m_inOvLap;
    wchar_t             m_errStr[512];
	double              m_clockSpeed;
	double              m_dRate;
    char*               m_FriendlyName;
};


