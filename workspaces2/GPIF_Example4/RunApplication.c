// Demonstrate GPIF as a Slave FIFO receiving data from the CPLD
//
// GPIF_Example4 uses SlaveFifoRead.h and CPLD requires CPLDasFifoMaster.xsvf
// Preset Button[6] = 0 to enable READ from CPLD
//
// There are some changes from the implementation described in the first edition of the book:
//  1	My development board worked successfully but some production boards did not operate correctly with the PushButton
//		The problem was tracked down to mechanical button bounce
//		A preferred fix was to put a button debouncer in the CPLD but there were not enough resources
//		So now the FX3 debounces the button on the CPLD's behalf and passes a CPLD_PUSH_BUTTON signal to it
//	2	On a READ the CPLD produces a LastWRData signal - this is detected by the GPIF state machine which COMMITs the last short packet
//  3	I was incorrectly using DebugPrint in several CallBack routines - I now set an Event and use DebugPrint in Main context
//	4	If a High Speed USB connection is made then PCLK is reduced to 10MHz and this allows debugging with simpler tools
//
// john@usb-by-example.com
//

#include "Application.h"

extern CyU3PReturnStatus_t InitializeDebugConsole(void);
extern CyU3PReturnStatus_t InitializeUSB(void);
extern void CheckStatus(char* StringPtr, CyU3PReturnStatus_t Status);
extern void DebugPrintEvent(uint32_t ActualEvents);
extern void ParseCommand(void);

CyU3PThread ThreadHandle;			// Handle to my Application Thread
CyBool_t glIsApplicationActive;		// Set true once device is enumerated
uint32_t Counter[12];				// Some DEBUG counters
TX_TIMER DebounceTimer;				// Timer used to debounce PushButton
CyU3PEvent CallbackEvent;			// Used by Callback to signal Main()

//Tony Oct 11 2016
CyBool_t default_I2C_sent = CyFalse;

//I2C by Tony
uint8_t DeviceAddress[6] = {0b01110000,0b01110100,0b01110110,0b01001101,0b00001111,0b01010010};
//Device 0: PCA9547 1110 A2 A1 A0 R/W
//Device 1,2: PCA9539 11101 A1 A0 R/W
//Temp 3: AD7415 0b01001101
//DAC 4: AD5667 0b00001111 (A1:GND+A0:GND = 1 1)
//Device 5: Davide's Board
uint8_t DACCh[3] = {0b00000000,0b00000001,0b00000111};
//A 0 0 0
//B 0 0 1
//AB(both) 1 1 1
uint8_t MuxCh[3] = {0x01,0x00,0x02};
//

void DebounceTimerExpired(ULONG NotUsed)
{
	// PushButton has finished bouncing, copy its current value to the CPLD
	CyBool_t Value;
	CyU3PGpioSimpleGetValue(PUSH_BUTTON, &Value);
	CyU3PGpioSetValue(CPLD_PUSH_BUTTON, Value);
}

void GPIO_CallBack(uint8_t gpioId)
{
	// At each detected edge of the PushButton restart the Debounce Timer
	UINT Active;
	uint32_t RemainingTicks;
	if (gpioId == PUSH_BUTTON)
	{
// Resync the Debounce Timer to the change on the PushButton
		tx_timer_info_get(&DebounceTimer, 0, &Active, &RemainingTicks, 0, 0);
		tx_timer_deactivate(&DebounceTimer);
		tx_timer_change(&DebounceTimer, RemainingTicks+DEBOUNCE_TIME, DEBOUNCE_TIME);
		tx_timer_activate(&DebounceTimer);
	}
}

void InitializeCPLD(void)
// CPLD needs to be RESET for correct operation
{
	CyU3PReturnStatus_t Status;
	CyU3PGpioClock_t GpioClock;
	CyU3PGpioSimpleConfig_t gpioConfig;
	CyBool_t Value;

    // Startup the GPIO module clocks
    GpioClock.fastClkDiv = 2;
    GpioClock.slowClkDiv = 0;
    GpioClock.simpleDiv = CY_U3P_GPIO_SIMPLE_DIV_BY_2;
    GpioClock.clkSrc = CY_U3P_SYS_CLK;
    GpioClock.halfDiv = 0;
    Status = CyU3PGpioInit(&GpioClock, 0);
    CheckStatus("Start GPIO Clocks", Status);

    // Need to claim CTRL[9] and CTRL[10] from the GPIF Interface
	Status = CyU3PDeviceGpioOverride(CPLD_PUSH_BUTTON, CyTrue);
	CheckStatus("CPLD_RUN_STOP Override", Status);
	Status = CyU3PDeviceGpioOverride(CPLD_RESET, CyTrue);
	CheckStatus("CPLD_RESET Override", Status);
	// Dec/30/2015 Tony ---------------------------------------------
	Status = CyU3PDeviceGpioOverride(SSN_pin, CyTrue);
	CheckStatus("SSN_pin Override", Status);
	//---------------------------------------------------------------

	// Reset by driving CPLD_RESET High
	CyU3PMemSet((uint8_t *)&gpioConfig, 0, sizeof(gpioConfig));
    gpioConfig.outValue = 1;
    gpioConfig.driveLowEn = CyTrue;
    gpioConfig.driveHighEn = CyTrue;
    Status = CyU3PGpioSetSimpleConfig(CPLD_RESET, &gpioConfig);
    CheckStatus("Reset CPLD", Status);
    // Dec/30/2015 Tony ---------------------------------------------
    Status = CyU3PGpioSetSimpleConfig(SSN_pin, &gpioConfig);
    CheckStatus("SSN_pin", Status);
    //---------------------------------------------------------------

    // Initial values for CPLD_PUSH_BUTTON = 0
    gpioConfig.outValue = 0;
    Status = CyU3PGpioSetSimpleConfig(CPLD_PUSH_BUTTON, &gpioConfig);
    CheckStatus("Set CPLD_PUSH_BUTTON", Status);
    // Setup PushButton as an input that can generate an interrupt
    gpioConfig.outValue = 1;
    gpioConfig.inputEn = CyTrue;
    gpioConfig.driveLowEn = CyFalse;
    gpioConfig.driveHighEn = CyFalse;
    gpioConfig.intrMode = CY_U3P_GPIO_INTR_BOTH_EDGE;
    Status = CyU3PGpioSetSimpleConfig(PUSH_BUTTON, &gpioConfig);
    CheckStatus("Setup PUSH_BUTTON", Status);
    // CPLD can also drive the PushButton, ensure that it isn't (ie check Value = 1)
	CyU3PGpioSimpleGetValue(PUSH_BUTTON, &Value);
	DebugPrint(4, ", Initial Value = %d,", Value);
    // Register a CallBack to deal with PushButton
    CyU3PRegisterGpioCallBack(GPIO_CallBack);
}

// Dec/30/2015 Tony
CyU3PReturnStatus_t I2C_Init(void)
{
	CyU3PI2cConfig_t i2cConfig;
	CyU3PReturnStatus_t Status;

	Status = CyU3PI2cInit();
	CheckStatus("CyU3PI2cInit",Status);
	i2cConfig.bitRate = CY_FX_USBI2C_I2C_BITRATE;
	i2cConfig.busTimeout = 0xFFFFFFFF;
	i2cConfig.dmaTimeout = 0xFFFF;
	i2cConfig.isDma = CyFalse;
	Status = CyU3PI2cSetConfig(&i2cConfig,NULL);
	CheckStatus("CyU3PI2cSetConfig",Status);
	return Status;
}
// Dec/30/2015 Tony
void ReadAD7415(int DeviceNumber,uint8_t *Value, uint8_t Count){
	CyU3PReturnStatus_t  Status;
	CyU3PI2cPreamble_t preamble;
	int read_i;
	preamble.length = 1;
	preamble.buffer[0] = (DeviceAddress[DeviceNumber]<<1) | 1;
	preamble.ctrlMask = 0x0000;

	Status = CyU3PI2cReceiveBytes(&preamble, Value, Count, 0);
	CheckStatus("I2C_Read", Status);
	for(read_i = 0; read_i<Count; read_i++){
		CyU3PDebugPrint(4,"\r\nRead%d = %x",read_i, *(Value+read_i));
	}
}
// Dec/30/2015 Tony
void WriteLEDs(int DeviceNumber,uint8_t *Value, uint8_t Count){
	CyU3PReturnStatus_t  Status;
	CyU3PI2cPreamble_t preamble;
	preamble.length = 1;
	preamble.buffer[0] = DeviceAddress[DeviceNumber]<<1;
	preamble.ctrlMask = 0x0000;

	Status = CyU3PI2cTransmitBytes(&preamble, Value, Count, 0);
	CheckStatus("I2C_Write", Status);
	//Wait for write to complete
	//preamble.length = 1;
	//Status = CyU3PI2cWaitForAck(&preamble, 10);
	//CheckStatus("I2C_WaitForAck",Status);
}
// Dec/30/2015 Tony
void SetVolt(double RefVolt, double Volt, uint8_t *MSB, uint8_t *LSB){
	double newVolt = 0.0;
	newVolt = Volt/2.0+RefVolt/2.0;
	uint16_t map = (uint16_t)((newVolt/RefVolt)*65535);
	*MSB = map>>8;
	*LSB = map;
	CyU3PDebugPrint(4, "\r\nRefVolt = %d, Volt = %d, map = %x, MSB = %x, LSB = %x",(int)RefVolt,(int)Volt,map,*MSB,*LSB);
}


void ApplicationThread(uint32_t Value)
{
	int32_t Seconds = 0;
    CyU3PReturnStatus_t Status;
    CyU3PI2cPreamble_t preamble; //BZ 16-2-28
    uint32_t ActualEvents, Count;

    // Dec/30/2015 Tony
    double T = 0.0;
    double V = 0.0;
    uint8_t pn = 0;//positive 0; negative 1
    uint8_t ch = 0;
    uint8_t temp = 0b01010101;
    uint8_t trans[7] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    uint8_t receive[7] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    //
    uint8_t cardsenses = 0;
    uint8_t cardmux[4] = {0x07, 0x06, 0x05, 0x04};

// Insert a delay here if using a USB Bus Spy to give time to start capture after the FX3 firmware has been loaded and started
//    DebugPrint(4, "\r\nReady:");
//    CyU3PThreadSleep(10000);

    Status = InitializeUSB();
    CheckStatus("Initialize USB", Status);

    if (Status == CY_U3P_SUCCESS)
    {
		// Wait for the device to be enumerated and initialized
		while (!glIsApplicationActive)
		{
			// Wait up to 100msec for USB CallBack Events
			Status = CyU3PEventGet(&CallbackEvent, USB_EVENTS, CYU3P_EVENT_OR_CLEAR, &ActualEvents, 100);
			if (Status == TX_SUCCESS) DebugPrintEvent(ActualEvents);
		}

		DebugPrint(4, "\r\nApplication started with %d", Value);

		// Dec/30/2015 Tony --------------------------------
		Status = CyU3PGpioSetValue(SSN_pin,1);
		CheckStatus("SSN_pin:1", Status);
		CyU3PThreadSleep(10);
		if (Status == CY_U3P_SUCCESS){
			Status = I2C_Init();
			CheckStatus("I2C_Init",Status);
//			//PCA9547
//			ch = MuxCh[0];
//			ch = ch | 0b0001000;//enable the selected channel
//			trans[0] = ch;
//			trans[1] = 0b00000000 | 0b00000000 | DACCh[2];
//			V = 0.8;
//			SetVolt(5.0,V,&trans[2],&trans[3]);
//			WriteLEDs(0,&trans[0],1);
//			WriteLEDs(4,&trans[1],3);
//
//			ch = MuxCh[1];
//			ch = ch | 0b0001000;//enable the selected channel
//			trans[0] = ch;
//			trans[1] = 0b00000000 | 0b00000000 | DACCh[2];
//			V = 0.8;
//			SetVolt(5.0,V,&trans[2],&trans[3]);
//			WriteLEDs(0,&trans[0],1);
//			WriteLEDs(4,&trans[1],3);
//
//			ch = MuxCh[2];
//			ch = ch | 0b0001000;//enable the selected channel
//			trans[0] = ch;
//			trans[1] = 0b00000000 | 0b00000000 | DACCh[2];
//			V = 0.8;
//			SetVolt(5.0,V,&trans[2],&trans[3]);
//			WriteLEDs(0,&trans[0],1);
//			WriteLEDs(4,&trans[1],3);

			//Set voltage of analog output
			ch = 0x02;//0x05;
			ch = ch | 0b0001000;//enable the selected channel
			trans[0] = ch;
			WriteLEDs(0,&trans[0],1);
//			trans[0] = 0b00111000; // turn on built in reference (not needed since there is an external reference
//			trans[1] = 0x00;
//			trans[2] = 0b00000001;
//			WriteLEDs(4,&trans[0],3);
			trans[0] = 0b00000000 | 0b00000000 | DACCh[2]; // set correct voltage
			trans[1] = 0x9A;
			trans[2] = 0x00;
			WriteLEDs(4,&trans[0],3);

			// Set voltage of on-board comparator DACs
			ch = 0x01;
			ch = ch | 0b0001000;//enable the selected channel
			trans[0] = ch;
			WriteLEDs(0,&trans[0],1);
			trans[0] = 0b00000000 | 0b00000000 | DACCh[0]; // set correct voltage of CMP IN 1
			trans[1] = 0x9A; //1.03V
			trans[2] = 0x00;
			WriteLEDs(4,&trans[0],3);
			trans[0] = 0b00000000 | 0b00000000 | DACCh[1]; // set correct voltage of CMP IN 2
			trans[1] = 0x9A;//1.03V
			trans[2] = 0x00;
			WriteLEDs(4,&trans[0],3);

			// Set voltage of on-board comparator DACs
			ch = 0x00;
			ch = ch | 0b0001000;//enable the selected channel
			trans[0] = ch;
			WriteLEDs(0,&trans[0],1);
			trans[0] = 0b00000000 | 0b00000000 | DACCh[0]; // set correct voltage of CMP IN 3
			trans[1] = 0x9A; //1.03V
			trans[2] = 0x00;
			WriteLEDs(4,&trans[0],3);
			trans[0] = 0b00000000 | 0b00000000 | DACCh[1]; // set correct voltage of CMP IN 4
			trans[1] = 0x9A;//1.03V
			trans[2] = 0x00;
			WriteLEDs(4,&trans[0],3);

			// Write PCA9547: Communicate with laser driver(Davide) through connector board
			// PartNo: M10X2	header: J203

			/*
			// Ryan 7/7/16: Code conversion for generating the set current command

			double setCurrent = 120;	// all currents are set in mA so 50 is 50mA and 500 is 500mA
			double vRef = 2.5; 		// value of the reference voltage set on laser driver
			double constant = 65535; 	// this is 2^16 -1
			uint8_t conversion[10] = {0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39}; //ascii codes for dec 0 to 9
			double resistor = 0.005; // value of resistor in k ohms
			unsigned int code;

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
			int i;
			for (i = 2; i < 7; i++) {
				trans[i] = conversion[codeParts[i-2]];
			}
			//
			*/
			//while(1){
			// Open the channel to Davide's Laser
				ch = 0x03;
				ch = ch | 0b0001000;//enable the selected channel
				trans[0] = ch;
				WriteLEDs(0,&trans[0],1);

				//'L1' means turn on
				//'L': 0x4c	'0':0x30	'1': 0x31
				trans[0] = 0x4c;
				trans[1] = 0x31;
				WriteLEDs(5,&trans[0],2);//Davide Laser Driver Board
				CyU3PThreadSleep(500);
				DebugPrint(4,"[I2C]Turning on the Laser.");
				//trans[0] = 0x4c;
				//trans[1] = 0x31;
				//WriteLEDs(5,&trans[0],2);//Davide Laser Driver Board
				//CyU3PThreadSleep(500);

				// set current // commented by Tony Oct 11 2016
				/*trans[0] = 0x53;
				trans[1] = 0x43;
				WriteLEDs(5,&trans[0],7);//Davide Laser Driver Board
				CyU3PThreadSleep(500);*/
			//}
			CyBool_t useTCD = CyFalse;
		if(useTCD){
			// read PCA9539 1
			preamble.buffer[0] = DeviceAddress[1]<<1;
			preamble.buffer[1] = 0x00;
			preamble.buffer[2] = DeviceAddress[1]<<1 | 0x01;
			preamble.length = 3;
			preamble.ctrlMask = 0x0002;
			Status = CyU3PI2cReceiveBytes (&preamble, &trans[0], 2,0);
			CheckStatus("I2C_Read", Status);
			cardsenses = !(trans[1] & 0x01)<<2; //B3

			// read PCA9539 2
			preamble.buffer[0] = DeviceAddress[2]<<1;
			preamble.buffer[1] = 0x00;
			preamble.buffer[2] = DeviceAddress[2]<<1 | 0x01;
			preamble.length = 3;
			preamble.ctrlMask = 0x0002;
			Status = CyU3PI2cReceiveBytes (&preamble, &trans[0], 2,0);
			CheckStatus("I2C_Read", Status);
			cardsenses = cardsenses | ~(trans[0] | 0b11111101); //B2
			cardsenses = cardsenses | (~(trans[0] | 0b11101111)>>4); //B1
			cardsenses = cardsenses | (~(trans[1] | 0b11011111)>>2); //B4

    		for (Count = 0; Count<4; Count++)
    		{
    			//Set comparator voltages of TDC cards
    			if (cardsenses & (0x01<<Count))
    			{
					ch = cardmux[Count];
					ch = ch | 0b0001000;//enable the selected channel
					trans[0] = ch;
					WriteLEDs(0,&trans[0],1);
					trans[0] = 0b00111000; // turn on built in reference
					trans[1] = 0x00;
					trans[2] = 0b00000001;
					WriteLEDs(4,&trans[0],3);
					trans[0] = 0b00000000 | 0b00000000 | DACCh[0]; // set correct voltage
					trans[1] = 0x48; //-22mV
					trans[2] = 0x00;
					WriteLEDs(4,&trans[0],3);
					trans[0] = 0b00000000 | 0b00000000 | DACCh[1]; // set correct voltage
					trans[1] = 0x46;//-150mV  0x3E; = -420mV
					trans[2] = 0x00;
					WriteLEDs(4,&trans[0],3);
    			}
    		}

			//write PCA9539 1
			ch = 0x06;
			trans[0] = ch;
			trans[1] = 0b11001111 | 0x00;
			trans[2] = 0b10101101 | 0b00000001; // B3 sense remains input
			WriteLEDs(1,&trans[0],3);
			ch = 0x02;
			trans[0] = ch;
			trans[1] = 0b00110000;
			trans[2] = 0b01010010;
			WriteLEDs(1,&trans[0],3);
			//write PCA9539 2
			ch = 0x06;
			trans[0] = ch;
			trans[1] = 0b11111110 | 0b10010010; // spare, B1 sense, B2 sense remain inputs
			trans[2] = 0b00100000; // B4 sense remains input
			WriteLEDs(2,&trans[0],3);
			ch = 0x02;
			trans[0] = ch;
			trans[1] = 0b00000001;
			trans[2] = 0b11111000 | (~cardsenses & 0b00000111);
			WriteLEDs(2,&trans[0],3);
		}


//			// Temp
//			trans[0] = 0x00;
//			WriteLEDs(3,&trans[0],1);
//			WriteLEDs(3,&receive[0],2);
//			pn = receive[0]>>7;
//			T = ((uint8_t)(receive[0]))+((double)(receive[1]>>6))/4.0;
//			if(pn == 1){T = T-128;}
//			CyU3PDebugPrint(4,"\r\nTemp = %d.%d", (int)T, (uint8_t)((T-(int)T)*100));
			CyU3PThreadSleep(10);
		}
		//--------------------------------------------------
		DebugPrint(4, "\r\nBasic Settings via I2C is complete.");
		default_I2C_sent = CyTrue;
    	// Now run forever
    	while (1)
    	{
    		for (Count = 0; Count<10; Count++)
    		{
				// Check for User Commands (and other CallBack Events) every 100msec
				CyU3PThreadSleep(100);
				Status = CyU3PEventGet(&CallbackEvent, ANY_EVENT, CYU3P_EVENT_OR_CLEAR, &ActualEvents, TX_NO_WAIT);
				if (Status == TX_SUCCESS)
				{
					if (ActualEvents & USER_COMMAND_AVAILABLE) ParseCommand();
					else DebugPrintEvent(ActualEvents);
				}
    		}
			DebugPrint(4, "%d, ", Seconds++);
//			// Temp
//			trans[0] = 0x00;
//			WriteLEDs(3,&trans[0],1);
//			WriteLEDs(3,&receive[0],2);
//			pn = receive[0]>>7;
//			T = ((uint8_t)(receive[0]))+((double)(receive[1]>>6))/4.0;
//			if(pn == 1){T = T-128;}
//			CyU3PDebugPrint(4,"\r\nTemp = %d.%d", (int)T, (uint8_t)((T-(int)T)*100));
			CyU3PThreadSleep(10);
		}
    }
    DebugPrint(4, "\r\nApplication failed to initialize. Error code: %d.\r\n", Status);
    // Returning here will stop the Application Thread running - it failed anyway so this is OK
}

// ApplicationDefine function called by RTOS to startup the application threads
void CyFxApplicationDefine(void)
{
    void *StackPtr;
    uint32_t Status;

    // Now create any needed resources then the Application thread
    Status = InitializeDebugConsole();
    CheckStatus("Initialize Debug Console", Status);

    // GPIO module already started, need to Initialize CPLD
    InitializeCPLD();

    // Need a system timer to debounce the pushbutton
    Status = tx_timer_create(&DebounceTimer, "DebounceTimer", DebounceTimerExpired, 0, DEBOUNCE_TIME, DEBOUNCE_TIME, TX_AUTO_ACTIVATE);
    CheckStatus("Create DebounceTimer", Status);

    // Create an Event so that alerts from CallBack routines can be monitored
    Status = CyU3PEventCreate(&CallbackEvent);
    CheckStatus("Create CallbackEvent", Status);

    StackPtr = CyU3PMemAlloc(APPLICATION_THREAD_STACK);
    Status = CyU3PThreadCreate(&ThreadHandle, 	// Handle to my Application Thread
            "11:GPIF_Example4",               	// Thread ID and name
            ApplicationThread,     				// Thread entry function
            42,                             	// Parameter passed to Thread
            StackPtr,                       	// Pointer to the allocated thread stack
            APPLICATION_THREAD_STACK,           // Allocated thread stack size
            APPLICATION_THREAD_PRIORITY,        // Thread priority
            APPLICATION_THREAD_PRIORITY,        // = Thread priority so no preemption
            CYU3P_NO_TIME_SLICE,            	// Time slice not supported
            CYU3P_AUTO_START                	// Start the thread immediately
            );

    if (Status != CY_U3P_SUCCESS)
    {
        // Thread creation failed with the Status = error code
        while(1)
        {
        	// Application cannot continue. Loop indefinitely
        }
    }
}

