using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.Threading;
using CyUSB;



namespace FPGAConfig
{
    public partial class FPGAConfigForm : Form
    {
        CyUSBDevice myDevice = null;                                                       // Create a USB device for our application called myUSB
        CyUSBDevice myFX3Device = null;
        USBDeviceList usbDevices = null;                                                // Find all USBDevice objects that represent all USB devices                                                  
        CyBulkEndPoint BulkOutEndPt = null;
        CyControlEndPoint CtrlEndPt = null;
        bool DeviceAttached = true;
        static int file_bytes;
        byte[] file_buffer;
        bool success;
        bool fx3fpgafirmwareuploaded = false;
        bool fpgafirmwareconfigured = false;
        bool fpgabitstreamselected = false;
        bool fpgaconfigured = false;
        bool fx3streameruploaded = false;
        Config cfginfo = new Config();
        bool stepbystep = false;

        public FPGAConfigForm()
        {
            InitializeComponent();                                                      // All components of the form are initialized
            Refresh();

            usbDevices = new USBDeviceList(CyConst.DEVICES_CYUSB);                      // All devices served by CyUSB.sys is added to the list usbDevices
            GetDevice();                                                                //  check if an EZ-USB device is already connected
            usbDevices.DeviceAttached += new EventHandler(usbDevices_DeviceAttached);   // Eventhandler assigned to DeviceAttached
                                                                                        // to handle the event when a device is attached
            usbDevices.DeviceRemoved += new EventHandler(usbDevices_DeviceRemoved);     // Eventhandler assigned to DeviceRemoved
                                                                                        // to handle the event when a device is removed
                                                                                        //StatLabel1.Text = "Download the image file into FX3 using Cypress Control Center";  
        }



        // --------------------------------------------------------------------------
        public void usbDevices_DeviceAttached(object sender, EventArgs e)
        {
            DeviceAttached = true;                                                      // A device has been connected

            GetDevice();
            int count;

            if(stepbystep)
                return;

            if(button_SelectFpgaBitstream.Enabled)
            {
                Thread.Sleep(1000);
                SelectFpgaBitstream();
                count = 0;
                while (!button_ConfigureFpga.Enabled)
                {
                    if(count > 10000)
                        break;
                    Thread.Sleep(100);
                    count += 100;
                }
                if(button_ConfigureFpga.Enabled)
                {
                    Thread.Sleep(1000);
                    ConfigureFpga();
                }
            }
        }



        // --------------------------------------------------------------------------
        public void usbDevices_DeviceRemoved(object sender, EventArgs e)
        {
            DeviceAttached = false;                                                      // A device has been disconnected
            RemoveDevice();
            //rtConsole.Clear();
        }


        // --------------------------------------------------------------------------
        public void RemoveDevice()
        {
            myFX3Device = usbDevices[0x04b4, 0x00F1] as CyFX3Device;

            if (!fx3fpgafirmwareuploaded)
            {
                if (myFX3Device == null)
                {
                    WriteConsole("No EZ-USB  FX3 device is connected\n");                           // Status message when device is disconnected
                    StatLabel1.Text = "NEXT STEP: Connect EZ-USB FX3 board to PC using an USB cable";
                    button_UploadFx3FpgaFirmware.Enabled = false;
                    button_ConfigureFpga.Enabled = false;
                    button_SelectFpgaBitstream.Enabled = false;
                }
            }
        }




        // --------------------------------------------------------------------------
        private void WriteConsole(string msg)
        {
            if(rtConsole.IsDisposed)
                return;

            rtConsole.AppendText(msg);
        }



        // --------------------------------------------------------------------------
        public void GetDevice()
        {
            if (DeviceAttached == true)
            {
                myDevice = usbDevices[0x04b4, 0x00F1] as CyUSBDevice;              // check for device with VID/PID of 0x04B4/0x1002
                myFX3Device = usbDevices[0x04B4, 0x00F3] as CyFX3Device;
                if(myFX3Device==null && fpgafirmwareconfigured)
                    myFX3Device = usbDevices[0x04B4, 0x00BC] as CyFX3Device;

                if ((myFX3Device != null) && (myDevice == null) && (!fpgafirmwareconfigured))                                           // If myDevice exists
                {
                    button_UploadFx3FpgaFirmware.Enabled = true;
                    button_SelectFpgaBitstream.Enabled = false;
                    button_ConfigureFpga.Enabled = false;
                    button_UploadFx3Streamer.Enabled = false;
                    button_InitDevice.Enabled = true;

                    StatLabel1.Text = "NEXT STEP: Use Download Firmware button to load image into FX3";
                    WriteConsole("EZ-USB FX3 Bootloader Device connected\n");

                    button_InitDevice.Select();
                }
                else if ((myFX3Device == null) && (myDevice != null) && (!fpgafirmwareconfigured))                                           // If myDevice exists
                {
                    BulkOutEndPt = myDevice.EndPointOf(0x02) as CyBulkEndPoint;   //Assign EP2 as BulkOutEP and EP6 as BulkInEP
                    button_UploadFx3FpgaFirmware.Enabled = false;
                    button_SelectFpgaBitstream.Enabled = true;
                    button_ConfigureFpga.Enabled = false;
                    button_UploadFx3Streamer.Enabled = false;
                    button_InitDevice.Enabled = false;

                    WriteConsole("FX3-Xilinx Slave Serial Programmer detected\n");
                    StatLabel1.Text = "NEXT STEP: Use Select Bitstream button to select the .bin file for FPGA ";
                }
                else if ((myFX3Device == null) && (myDevice == null) && (fpgafirmwareconfigured))                                           // If myDevice exists
                {
                    button_UploadFx3FpgaFirmware.Enabled = false;
                    button_SelectFpgaBitstream.Enabled = false;
                    button_ConfigureFpga.Enabled = false;
                    button_UploadFx3Streamer.Enabled = false;
                    button_InitDevice.Enabled = false;

                    WriteConsole("NO FX3-Xilinx Slave Serial Programmer detected\n");
                }
                else if ((myFX3Device != null) && (myDevice == null) && (fpgafirmwareconfigured))                                           // If myDevice exists
                {
                    button_UploadFx3FpgaFirmware.Enabled = false;
                    button_SelectFpgaBitstream.Enabled = false;
                    button_ConfigureFpga.Enabled = false;
                    button_UploadFx3Streamer.Enabled = true;
                    button_InitDevice.Enabled = false;

                    WriteConsole("EZ-USB FX3 Bootloader Device connected\n");
                    StatLabel1.Text = "NEXT STEP: Upload GPIF FX3 firmware image into FX3";

                    if(!stepbystep)
                        button_UploadFx3Streamer_Click(null, null);
                }
                else
                {
                    button_UploadFx3FpgaFirmware.Enabled = false;
                    button_SelectFpgaBitstream.Enabled = false;
                    button_ConfigureFpga.Enabled = false;
                    button_UploadFx3Streamer.Enabled = false;
                    button_InitDevice.Enabled = false;

                    WriteConsole("No EZ-USB  FX3 device is connected\n");
                    StatLabel1.Text = "NEXT STEP: Connect EZ-USB FX3 board to PC using an USB cable";
                    DeviceAttached = false;
                }
            }

            if (DeviceAttached == false)
            {
                DisableApp();                                                          // there is either no device attached or the attached device is not a FX2
            }
            else
            {
                EnableApp();                                                           // the attached device is a FX2LP with our VID/PID
            }
        }



        // --------------------------------------------------------------------------
        public void EnableApp()
        {

        }



        // --------------------------------------------------------------------------
        public void DisableApp()
        {

        }



        // --------------------------------------------------------------------------
        private void Form1_Load(object sender, EventArgs e)
        {

        }


        // --------------------------------------------------------------------------
        private void label1_Click(object sender, EventArgs e)
        {

        }



        // --------------------------------------------------------------------------
        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }



        // --------------------------------------------------------------------------
        private void rtConsole_TextChanged(object sender, EventArgs e)
        {

        }


        // --------------------------------------------------------------------------
        private void UploadFx3FpgaFirmware()
        {
            CyFX3Device fx = myFX3Device as CyFX3Device;
            string tmpFilter = FOpenDialog.Filter;
            string filename = cfginfo.getFX3_FPGAFirmwareUploadExe();

            if ((fx != null) && File.Exists(filename))
            {
                FX3_FWDWNLOAD_ERROR_CODE enmResult = FX3_FWDWNLOAD_ERROR_CODE.SUCCESS;

                WriteConsole("Programming RAM of ");
                WriteConsole(fx.FriendlyName);
                WriteConsole("........\n");

                Refresh();
                enmResult = fx.DownloadFw(filename, FX3_FWDWNLOAD_MEDIA_TYPE.RAM);

                WriteConsole("Programming");
                WriteConsole(fx.GetFwErrorString(enmResult));
                WriteConsole("\n");
                Refresh();
                button_UploadFx3FpgaFirmware.Enabled = false;
                fx3fpgafirmwareuploaded = true;
            }
            else if (myDevice != null)
            {
                WriteConsole("FX3 firmware is already downloaded\n");
                StatLabel1.Text = "NEXT STEP: Use Select Bitstream to select the FPGA .bin file ";
            }
        }



        // --------------------------------------------------------------------------
        private void button_UploadFx3FpgaFirmware_Click(object sender, EventArgs e)
        {
            UploadFx3FpgaFirmware();
            stepbystep = true;
        }



        // --------------------------------------------------------------------------
        private void SelectFpgaBitstream()
        {
            long flen = 0;
            string tmpFilter = FOpenDialog.Filter;
            string title = FOpenDialog.Title;
            string filename = cfginfo.getFPGA_FirmwareExe();

            if (File.Exists(filename)) //selecting bitstream
            {
                Refresh();
                WriteConsole("Bitstream File Selected\n");
                StatLabel1.Text = "NEXT STEP: Use Configure button to start Configuration ";
            }
            else
            {
                return;
            }

            FileStream file = new FileStream(filename, FileMode.Open, FileAccess.Read);
            flen = file.Length;
            //file_bytes = (int)flen;

            file_bytes = (int)flen;
            file_buffer = new byte[file_bytes];

            file.Read(file_buffer, 0, file_bytes);
            file.Close();
            WriteConsole(filename);
            WriteConsole("\n");

            button_ConfigureFpga.Enabled = true;
            button_SelectFpgaBitstream.Enabled = false;
            fpgabitstreamselected = true;
        }



        // --------------------------------------------------------------------------
        private void button_SelectFpgaBitstream_Click(object sender, EventArgs e)
        {
            SelectFpgaBitstream();
            stepbystep = true;
        }



        // --------------------------------------------------------------------------
        private void ConfigureFpga()
        {
            int len = 0;
            byte[] buf = new byte[16];

            buf[0] = (Byte)(file_bytes & 0x000000FF);
            buf[1] = (Byte)((file_bytes & 0x0000FF00) >> 8);
            buf[2] = (Byte)((file_bytes & 0x00FF0000) >> 16);
            buf[3] = (Byte)((file_bytes & 0xFF000000) >> 24);
            if (myDevice != null)
            {
                WriteConsole("Writing data to FPGA........\n");
                StatLabel1.Text = "NEXT STEP: Wait ... ";
                CtrlEndPt = myDevice.ControlEndPt;
                CtrlEndPt.Target = CyConst.TGT_DEVICE;
                CtrlEndPt.ReqType = CyConst.REQ_VENDOR;
                CtrlEndPt.Direction = CyConst.DIR_TO_DEVICE;
                CtrlEndPt.ReqCode = 0xB2;
                CtrlEndPt.Value = 0;
                CtrlEndPt.Index = 1;
                len = 16;
                CtrlEndPt.XferData(ref buf, ref len);//send vendor command to start configuration 
                // myDevice.BulkOutEndPt.TimeOut = 100000;
                myDevice.BulkOutEndPt.XferSize = 4096;//set transfer size as 4096

                //myDevice.BulkOutEndPt.TimeOut = 1000;

                success = myDevice.BulkOutEndPt.XferData(ref file_buffer, ref file_bytes); //check if transfer successful

                if (success == true)
                {
                    button_ConfigureFpga.Enabled = false;

                    WriteConsole("Configuration data has been sent to FPGA\n");
                    System.Threading.Thread.Sleep(200);
                    CtrlEndPt.Target = CyConst.TGT_DEVICE;
                    CtrlEndPt.ReqType = CyConst.REQ_VENDOR;
                    CtrlEndPt.Direction = CyConst.DIR_FROM_DEVICE;
                    CtrlEndPt.ReqCode = 0xB1;
                    CtrlEndPt.Value = 0;
                    CtrlEndPt.Index = 1;
                    len = 16;
                    buf[0] = 0;

                    CtrlEndPt.XferData(ref buf, ref len);//send vendor command to know the status of Configuration
                    button_ConfigureFpga.Enabled = false;

                    //rtConsole.Clear();

                    if (buf[0] == 1)
                    {
                        button_SelectFpgaBitstream.Enabled = false;
                        WriteConsole("Configuration Successful\n");
                        WriteConsole("FX3 Slave FIFO interface is activated\n");
                        WriteConsole("*********************************************\n");
                        StatLabel1.Text = "NEXT STEP: Reset FX3 board and upload the Streamer application using the Upload FX3 Streamer button.";
                        fpgafirmwareconfigured = true;
                    }
                    else
                    {
                        button_SelectFpgaBitstream.Enabled = true;
                        button_ConfigureFpga.Enabled = true;
                        WriteConsole("Configuration Failed\n");
                        StatLabel1.Text = "NEXT STEP: Please Repeat the Steps Carefully";
                    }
                }
                else
                {
                    WriteConsole("Failed to send the Configuration data to FPGA\n");
                    StatLabel1.Text = "NEXT STEP: Please Repeat the Steps Carefully";
                }
            }
            else
            {
                button_UploadFx3FpgaFirmware.Enabled = false;
                button_ConfigureFpga.Enabled = false;
                button_SelectFpgaBitstream.Enabled = false;

                WriteConsole("No EZ-USB FX3 device is connected\n");
                StatLabel1.Text = "NEXT STEP: Connect EZ-USB FX3 board to PC using an USB cable";
            }
        }



        // --------------------------------------------------------------------------
        private void button_ConfigureFpga_Click(object sender, EventArgs e)
        {
            ConfigureFpga();
            stepbystep = true;
        }



        // --------------------------------------------------------------------------
        private void UploadFx3Streamer()
        {
            CyFX3Device fx = myFX3Device as CyFX3Device;
            string tmpFilter = FOpenDialog.Filter;
            string filename = cfginfo.getFX3_StreamerFirmwareExe();

            if((fx != null) && File.Exists(filename))
            {
                FX3_FWDWNLOAD_ERROR_CODE enmResult = FX3_FWDWNLOAD_ERROR_CODE.SUCCESS;

                WriteConsole("Programming RAM of ");
                WriteConsole(fx.FriendlyName);
                WriteConsole("........\n");

                Refresh();
                enmResult = fx.DownloadFw(filename, FX3_FWDWNLOAD_MEDIA_TYPE.RAM);

                WriteConsole("Programming");
                WriteConsole(fx.GetFwErrorString(enmResult));
                WriteConsole("\n");
                Refresh();
                fx3streameruploaded = true;
            }
            else if(myDevice != null)
            {
                WriteConsole("FX3 Streamer firmware is already uploaded\n");
                StatLabel1.Text = "NEXT STEP: Ready to stream data to host application.";
                fx3streameruploaded = true;
            }
        }


        // --------------------------------------------------------------------------
        private void button_UploadFx3Streamer_Click(object sender, EventArgs e)
        {
            UploadFx3Streamer();
            Thread.Sleep(1000);
            Close();
        }



        // --------------------------------------------------------------------------
        private void button_InitDevice_Click(object sender, EventArgs e)
        {
            UploadFx3FpgaFirmware();
            stepbystep = false;            
        }
    }
}

