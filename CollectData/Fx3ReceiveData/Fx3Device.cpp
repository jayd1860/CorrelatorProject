#include "stdafx.h"
#include <windows.h>
#include "Fx3Device.h"

#define     PACKETS_PER_TRANSFER                8		// 256
#define     NUM_TRANSFER_PER_TRANSACTION        8		// 64
#define     MAX_QUEUE_SZ                        64
#define     BULK_END_POINT                      2
#define     VENDOR_CMD_START_TRANSFER           0xB5



// --------------------------------------------------------------------------------------------
Fx3Device::Fx3Device()
{
    int ii = 0;
    int kk = 0;
    int	endPointCount = 0;

    m_deviceCount = 0;
    m_USBDevice = new CCyUSBDevice(NULL, CYUSBDRV_GUID, true);
    m_inOvLap = NULL;
    m_maxPktSizeDefault = (long)pow(2.0, 14.0);
    m_endPoint = NULL;


	memset(m_errStr, 0, sizeof(m_errStr));

    if(m_USBDevice != NULL)
    {
        // Sometimes device don't become active right away after uploading firmaware. 
        // Give it a few tries (about 100) to get devices before giving up
        while(m_deviceCount == 0)
        {
            m_deviceCount = m_USBDevice->DeviceCount();
            Sleep(10);
            if(kk > 2000)
                break;
            kk++;
        }

        for(ii=0; ii<m_deviceCount; ii++)
        {
            m_USBDevice->Open(ii);
            m_name[ii].Format(L"(0x%04X - 0x%04X) %s", m_USBDevice->VendorID, m_USBDevice->ProductID, CString(m_USBDevice->FriendlyName));
        }

        m_endPointCount = m_USBDevice->EndPointCount();
        for(ii=0; ii<m_endPointCount; ii++)
        {
            m_endPoint = m_USBDevice->EndPoints[ii];
            if(m_endPoint->Attributes == BULK_END_POINT && m_endPoint->bIn == true)
                break;
        }
        m_endPointCount = ii;

        if(m_endPoint != NULL)
            m_totalTransferSize = m_endPoint->MaxPktSize * PACKETS_PER_TRANSFER;
        else
            m_totalTransferSize = m_maxPktSizeDefault * PACKETS_PER_TRANSFER;
    }
}



// --------------------------------------------------------------------------------------------
Fx3Device::Fx3Device(int devIdx, bool input)
{
    int ii = 0;
    int endPointCount = 0;

    m_deviceCount = 0;
    m_USBDevice = new CCyUSBDevice(NULL, CYUSBDRV_GUID, true);
    m_inOvLap = new OVERLAPPED;
    m_inOvLap->hEvent = CreateEvent(NULL, false, false, NULL);
    m_endPoint = NULL;

    if(devIdx<0)
        return;

    m_USBDevice->Open(devIdx);

    memset(m_errStr, 0, sizeof(m_errStr));

    endPointCount = m_USBDevice->EndPointCount();
    for(ii=0; ii<endPointCount; ii++)
    {
        m_endPoint = m_USBDevice->EndPoints[ii];
        if(m_endPoint->Attributes == BULK_END_POINT && m_endPoint->bIn == input)
            break;
    }
    m_endPointCount = ii;

    m_deviceCount = m_USBDevice->DeviceCount();

    if(m_endPoint != NULL)
        m_totalTransferSize = m_endPoint->MaxPktSize * PACKETS_PER_TRANSFER;
    else {
        m_totalTransferSize = m_maxPktSizeDefault * PACKETS_PER_TRANSFER;
        return;
    }

    m_endPoint->SetXferSize(m_totalTransferSize);
}



// --------------------------------------------------------------------------------------------
Fx3Device::~Fx3Device()
{
    if(m_inOvLap != NULL)
    {
        CloseHandle(m_inOvLap->hEvent);
        delete 	m_inOvLap;
    }

    if(m_endPoint != NULL)
        m_endPoint->Abort();

    if(m_USBDevice != NULL)
        delete m_USBDevice;
}



// --------------------------------------------------------------------------------------------
int Fx3Device::SendStartNotification()
{

    // Send start notification to the device 
    if(m_USBDevice->bSuperSpeed)
    {
        CCyControlEndPoint *ControlEndPt = (CCyControlEndPoint *)m_USBDevice->EndPointOf(0);

        ControlEndPt->Target = TGT_DEVICE;
        ControlEndPt->ReqType = REQ_VENDOR;

        // Vendor Command that is transmitted for starting the read.
        ControlEndPt->ReqCode = VENDOR_CMD_START_TRANSFER;
        ControlEndPt->Direction = DIR_TO_DEVICE;

        // Send Value = 1 and Index = 0, to kick start the transaction.
        ControlEndPt->Value = 0x0001;
        ControlEndPt->Index = 0;

        // Send vendor command now......
        long len = 0;
        ControlEndPt->XferData(NULL, len, NULL);
    }
    return 0;
}




// --------------------------------------------------------------------------------------------
int Fx3Device::SendStopNotification()
{

    // Send stop notification to the device 
    if(m_USBDevice->bSuperSpeed)
    {
        LONG dataLength = 0;
        CCyControlEndPoint *ControlEndPt = (CCyControlEndPoint *)m_USBDevice->EndPointOf(0);

        ControlEndPt->Target = TGT_DEVICE;
        ControlEndPt->ReqType = REQ_VENDOR;

        // Vendor Command that is transmitted for starting the read.
        ControlEndPt->ReqCode = VENDOR_CMD_START_TRANSFER;
        ControlEndPt->Direction = DIR_TO_DEVICE;

        // Send Value = 0 and Index = 0, to stop the transaction.
        ControlEndPt->Value = 0x0000;
        ControlEndPt->Index = 0;

        // Send vendor command now......
        ControlEndPt->XferData(NULL, dataLength, NULL);
    }
    m_endPoint->Abort();
    m_endPoint = NULL;

    return 0;
}




// --------------------------------------------------------------------------------------------
int Fx3Device::SetCurrent(PUCHAR buffer, long length)
{
    //
    // Calculate the max packet size (USB Frame Size).
    // From bulk burst transfer, this size represent bulk burst size and this 
    // size now belongs to multiple USB frames
    //
    // Tony Oct 14 2016
    m_endPoint->SetXferSize(length);

    // Send start notification to the device 
    if(m_USBDevice->bSuperSpeed)
    {
        if(buffer != NULL)
        {
            bool bXferCompleted = m_endPoint->XferData(buffer, length, NULL); //IsPkt
            if(bXferCompleted)
                return 0;
            else
                return -1;
        }
    }
    return -1;
}




// -------------------------------------------------------------------------------
bool Fx3Device::IsSuperSpeed()
{
    return m_USBDevice->bSuperSpeed;
}

