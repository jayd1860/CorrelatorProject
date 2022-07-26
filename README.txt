

How to run correlator executable stand alone with no visual studio. 

1. First install Visual Studio 2015 Redistributable. 
Search Online "Visual Studio 2015 Redistributable" and download the version x86.

2. Install Cypress FX3 USB drivers.

a) Go to Cypress website and set up an account if you don't have one already. You'll need
it to download Cypress products - but it is free otherwise. Then log in with an email user name 
and password and search for 

EZ-USB FX3 SDK v1.3.3 for Windows 

Download it. They try to make you install some download manager but give you options to 
bypass it. Close any window that pops up that doesn't look relevant to Cypress. 
Then, to install and bypass download manager (or any other application that's not needed), 
click on the 

"If you prefer, you may bypass the download manager here." link 

or 

"download the file without using the download manager" link. 

b) Run the installer. When offered to mnodify the default installation 
folder, modify the installation folder to be "C:\Program Files\Cypress"

c) When Choosing installation type, choose custom and then uncheck

ARM GCC
Eclipse 

You don't need either of these. All you need really are the FX3 USB Windows drivers. 

d) To install the FX3 driver, 
   1. connect FX3 device.
   2. open Device Manager. 
   3. Click Other Devices. 
   4. Right Click on the device under other device and choose Update Driver --> Update driver from local drive. 
   5. Navigate to the Cypress drivers folder under 

      C:\Program Files (x86)\Cypress\EZ-USB FX3 SDK\1.3\driver\bin\<your Windows OS>\x64

      and select it as the folder where to look for the drivers. 

      NOTE:  the folder where the drivers are is C:\Program Files (x86) even though we installed FX3 SDK 
      under C:\Program Files\Cypress. This is because the installer installed the bulk of the files 
      that matter under C:\Program Files (x86)\Cypress anyway. But also some files were installed under
      C:\Program Files\Cypress...for whatever reason that's unknown. I guess some files are x86 while 
      others are x64, bla, bla, bla. 

e) Click install and the drivers should now be usable. 

f) Navigate to the <Folder where you downloaded correlatorproject>/correlatorproject/bin folder
and create a Desktop shortcut to the executable file DcsCollectDataForm.exe

How to use this software:

1. Connect the fast DCS to the laptop through USB3 cable type-A to type-B (USB2 cable won't work.)
2. Open the folder "bin," and open the software "Fx3ReceiveData"
3. Configure FX3 through clicking "FX3 Config GUI" and "Initialize all"
4. Set the current which drives the laser.
