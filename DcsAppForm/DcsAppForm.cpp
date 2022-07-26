#include "DcsAppForm.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThread]
void Main(array<String^>^ args)
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	DcsAppForm::DcsAppForm^ form = gcnew DcsAppForm::DcsAppForm;
	Application::Run(form);
}

