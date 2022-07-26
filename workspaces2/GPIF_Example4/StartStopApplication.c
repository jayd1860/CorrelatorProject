/*
 * StartStopApplication.c
 *
 *      Author: john@usb-by-example.com
 */

#include "Application.h"
#include "SlaveFifoRead.h"				// File generated by GPIF Designer
char* CyFxGpifConfigName = { "SlaveFifoRead" };

// Declare external functions
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
//Tony Oct 11 2016
extern void WriteLEDs(int DeviceNumber,uint8_t *Value, uint8_t Count);

// Declare external data
extern CyBool_t glIsApplicationActive;		// Set true once device is enumerated
extern uint32_t ClockValue;					// Used to select GPIF speed

CyU3PDmaChannel GPIF2USB_Handle, USB2CPU_Handle;

const char* BusSpeed[] = { "Not Connected", "Full ", "High ", "Super" };
const uint16_t EpSize[] = { 0, 64, 512, 1024 };

// Tony Oct 12 2016
uint8_t trans_2laser[7] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //ascii codes for dec 0 to 9
// Tony Oct 12 2016
void set_trans_current2instruction(uint8_t* current, uint16_t current_char_vect_len){
	// Ryan 7/7/16: Code conversion for generating the set current command
	double setCurrent = 0.0;	// all currents are set in mA so 50 is 50mA and 500 is 500mA
	double vRef = 2.5; 		// value of the reference voltage set on laser driver
	double constant = 65535; 	// this is 2^16 -1
	uint8_t conversion[10] = {0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39}; //ascii codes for dec 0 to 9
	double resistor = 0.005; // value of resistor in k ohms
	unsigned int code;
	int char_i = 0;
	double times_index = 1.0;
	CyBool_t without_dot = CyTrue;
	for (char_i = 0; char_i < current_char_vect_len; char_i++)
	{
		if(current[char_i]==0x2E){
			without_dot = CyFalse;
			setCurrent = setCurrent/(times_index*10);
			times_index = 0.1;
		}
		else{
			if((current[char_i] >= 0x30) && (current[char_i] <= 0x39) ){
				setCurrent = setCurrent+times_index*((double)(current[char_i]-0x30));
				times_index = times_index*0.1;
			}
		}
	}
	if(without_dot){setCurrent = setCurrent/(times_index*10);}
	DebugPrint(4, "\r\nconvert to double: %d.%d\r\n", (int)setCurrent,(int)((setCurrent-(int)setCurrent)*10000.0));
	code = (unsigned int)((constant/vRef)*(vRef - (setCurrent * resistor)));	//math to determine the current code for the laser
	unsigned int value = code;
	int count = 0;
	int base = 10;
	int codeSize = 5;
	unsigned int revCodeParts[5] = {0,0,0,0,0};
	unsigned int codeParts[5] = {0,0,0,0,0};
	//separating the int into the component digits
	while (count < codeSize){
		unsigned int digit = value % base;
		value = (value - digit) / base;
		revCodeParts[count] = digit;
		count++;
	}
	//putting the digits back into the correct order
	for (count = 0; count < codeSize; count++ ){
		codeParts[count] = revCodeParts[codeSize - count - 1];
	}
	//determining the ascii code for the laser current code
	int trans_2laser_i;
	trans_2laser[0] = 0x53;
	trans_2laser[1] = 0x43;
	for (trans_2laser_i = 2; trans_2laser_i < 7; trans_2laser_i++) {
		trans_2laser[trans_2laser_i] = conversion[codeParts[trans_2laser_i-2]];
	}
}

//Tony Oct 11 2016
extern CyBool_t default_I2C_sent;

CyU3PReturnStatus_t StartGPIF(void)
{
	CyU3PReturnStatus_t Status;
	Status = CyU3PGpifLoad(&CyFxGpifConfig);
	DebugPrint(7, "\r\nUsing GPIF:%s", CyFxGpifConfigName);
	CheckStatus("GpifLoad", Status);
	Status = CyU3PGpifSocketConfigure(0, GPIF_PRODUCER_SOCKET, 2, CyFalse, 1);
	CheckStatus("SetWatermark", Status);
	Status = CyU3PGpifSMStart(START, ALPHA_START);
	return Status;
}

void CB_USB2CPU(CyU3PDmaChannel *Handle, CyU3PDmaCbType_t Type, CyU3PDmaCBInput_t *Input)
{
	DebugPrint(4, "Enter CB_USB2CPU");
	if (glIsApplicationActive)
	{
		if(default_I2C_sent)
		{
			if (Type == CY_U3P_DMA_CB_PROD_EVENT)
			{
				uint8_t* BytePtr = Input->buffer_p.buffer;
				uint8_t* EndPtr = BytePtr + Input->buffer_p.count;
				*EndPtr = 0;
				// Shouldn't call DebugPrint in a callback but this is a special case
				DebugPrint(4, "\r\nReceived: %s", BytePtr);
				set_trans_current2instruction(BytePtr, Input->buffer_p.count);
				// Open the channel to Davide's Laser
				uint8_t temp_ch = 0x03;
				temp_ch = temp_ch | 0b0001000;//enable the selected channel
				uint8_t temp_trans[1];
				temp_trans[0] = temp_ch;
				WriteLEDs(0,&temp_trans[0],1);
				WriteLEDs(5,&trans_2laser[0],7);
				DebugPrint(5, "\r\nI2C send: %s\r\n", trans_2laser);
				CyU3PDmaChannelDiscardBuffer(Handle);
			}
		}
		else
		{
			DebugPrint(4, "Does not work on CB_USB2CPU \r\nbecause Basic Settings via I2C are not ready.");}
	}
	else
	{DebugPrint(4, "Does not work on CB_USB2CPU because it is not ready.");}
}

void StartApplication(void)
// USB has been enumerated, time to start the application running
{
	CyU3PEpConfig_t epConfig;
	CyU3PDmaChannelConfig_t dmaConfig;
	CyU3PReturnStatus_t Status;
    CyU3PPibClock_t pibClock;

    CyU3PUSBSpeed_t usbSpeed = CyU3PUsbGetSpeed();
    // Display the enumerated device bus speed
    DebugPrint(4, "\r\n@StartApplication, running at %sSpeed", BusSpeed[usbSpeed]);

    // Start GPIF clocks, they need to be running before we attach a DMA channel to GPIF
    pibClock.clkDiv = ClockValue>>1;
    pibClock.clkSrc = CY_U3P_SYS_CLK;
    pibClock.isHalfDiv = (ClockValue & 1);
    pibClock.isDllEnable = CyFalse;		// Disable Dll since this application is synchronous
    Status = CyU3PPibInit(CyTrue, &pibClock);
 	CheckStatus("Start GPIF Clock", Status);
 	DebugPrint(4, "\r\nGPIF Clock = %d MHz = %d MB/s", 800/ClockValue, 3200/ClockValue);

    // Based on the Bus Speed configure the endpoint packet size
	CyU3PMemSet((uint8_t *)&epConfig, 0, sizeof(epConfig));
	epConfig.enable = CyTrue;
	epConfig.epType = CY_U3P_USB_EP_BULK;
	epConfig.burstLen = (usbSpeed == CY_U3P_SUPER_SPEED) ? (ENDPOINT_BURST_LENGTH) : 1;
	epConfig.pcktSize = EpSize[usbSpeed];

	// Setup and flush the consumer endpoint
	Status = CyU3PSetEpConfig(USB_CONSUMER_ENDPOINT, &epConfig);
	CheckStatus("Setup USB_CONSUMER_ENDPOINT", Status);

	// Create a AUTO channel for the GPIF to USB transfer, GPIF detects and COMMITs the last short packet
	CyU3PMemSet((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
	dmaConfig.size           = DMA_BUFFER_SIZE;			// Use same size buffers for all USB Speeds
	dmaConfig.count          = 4;//DMA_BUFFER_COUNT; //3;//Tony OCt 17 2016 to avoid DMA buffers overlap
	dmaConfig.prodSckId		 = GPIF_PRODUCER_SOCKET;
	dmaConfig.consSckId		 = USB_CONSUMER_ENDPOINT_SOCKET;
	dmaConfig.dmaMode        = CY_U3P_DMA_MODE_BYTE;
	Status = CyU3PDmaChannelCreate(&GPIF2USB_Handle, CY_U3P_DMA_TYPE_AUTO, &dmaConfig);
	CheckStatus("GPIF2USB DmaChannelCreate", Status);

	Status = CyU3PUsbFlushEp(USB_CONSUMER_ENDPOINT);
	CheckStatus("Flush USB_CONSUMER_ENDPOINT", Status);

	// Start the DMA Channel with transfer size to Infinite
	Status = CyU3PDmaChannelSetXfer(&GPIF2USB_Handle, 0);
	CheckStatus("GPIF2USB DmaChannelStart", Status);

	if(1){
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Tony Oct 11 2016
		// Setup and flush the producer endpoint
		Status = CyU3PSetEpConfig(USB_PRODUCER_ENDPOINT, &epConfig);
		CheckStatus("Setup USB_PRODUCER_ENDPOINT", Status);

		// Create a AUTO channel for the USB to CPU transfer, USB detects and COMMITs the last short packet
		CyU3PMemSet((uint8_t *)&dmaConfig, 0, sizeof(dmaConfig));
		dmaConfig.size = 32;		// I assume a person is typing
		dmaConfig.count = 1;
		dmaConfig.dmaMode = CY_U3P_DMA_MODE_BYTE;
		dmaConfig.notification = CY_U3P_DMA_CB_PROD_EVENT;
		dmaConfig.prodSckId		 = USB_PRODUCER_ENDPOINT_SOCKET;
		dmaConfig.consSckId		 = CY_U3P_CPU_SOCKET_CONS;
		dmaConfig.cb		 	 = CB_USB2CPU;
		Status = CyU3PDmaChannelCreate(&USB2CPU_Handle, CY_U3P_DMA_TYPE_MANUAL_IN, &dmaConfig);
		CheckStatus("GPIF2USB DmaChannelCreate", Status);

		Status = CyU3PUsbFlushEp(USB_PRODUCER_ENDPOINT);
		CheckStatus("Flush USB_PRODUCER_ENDPOINT", Status);

		// Start the DMA Channel with transfer size to Infinite
		Status = CyU3PDmaChannelSetXfer(&USB2CPU_Handle, 0);
		CheckStatus("USB2CPU DmaChannelStart", Status);
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Load, configure and start the GPIF state machine
    Status = StartGPIF();
	CheckStatus("GpifStart", Status);

	// BZ Jan 22
	CyU3PUsbLPMDisable();
	CyU3PUsbSetLinkPowerState(CyU3PUsbLPM_U0);

    // OK, Application can now run
    glIsApplicationActive = CyTrue;
}

void StopApplication(void)
// USB connection has been lost, time to stop the application running
{
    CyU3PEpConfig_t epConfig;
    CyU3PReturnStatus_t Status;

    // Disable GPIF, close the DMA channels, flush and disable the endpoints
    CyU3PGpifDisable(CyTrue);
    Status = CyU3PPibDeInit();
    CheckStatus("Stop GPIF Block", Status);
    Status = CyU3PDmaChannelDestroy(&GPIF2USB_Handle);
    CheckStatus("GPIF2USB DmaChannelDestroy", Status);
    Status = CyU3PUsbFlushEp(USB_CONSUMER_ENDPOINT);
    CheckStatus("Flush USB_CONSUMER_ENDPOINT", Status);
	CyU3PMemSet((uint8_t *)&epConfig, 0, sizeof(&epConfig));
    Status = CyU3PSetEpConfig(USB_CONSUMER_ENDPOINT, &epConfig);
	CheckStatus("Disable USB_CONSUMER_ENDPOINT", Status);

    // OK, Application is now stopped
    glIsApplicationActive = CyFalse;
}

