#include "LightAndTissPropertiesForm.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThread]
void Main(array<String^>^ args)
{
    double  mua    = .1;
    double  musp   = 6;
    double  lamda  = 850e-7;
    double  rho[4];
    int     nDet = (sizeof(rho)/sizeof(double));

    for(int ii=0; ii<nDet; ii++)
        rho[ii] = 2;

    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);
    LightAndTissPropertiesForm::LightAndTissPropertiesForm^ form = 
        gcnew LightAndTissPropertiesForm::LightAndTissPropertiesForm(mua, musp, lamda, rho, nDet);
    Application::Run(form);
}

