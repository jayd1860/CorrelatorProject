#pragma once

#include <windows.h>
#include <shlwapi.h> 
#include <stdio.h>
#include <stdlib.h>
#include <limits>
#include <stdint.h>
#include <vcclr.h>  

namespace LightAndTissPropertiesForm {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;

    /// <summary>
    /// Summary for LightAndTissPropertiesForm
    /// </summary>
    public ref class LightAndTissPropertiesForm : public System::Windows::Forms::Form
    {
    public:

        LightAndTissPropertiesForm(double mua0, double musp0, double lamda0, double* rho0, int nDet0)
        {
            InitializeComponent();

            //
            //TODO: Add the constructor code here
            //
            char str[20];

            mua    = mua0;
            musp   = musp0;
            lamda  = lamda0;
            nDet   = nDet0;
            rho    = new double[nDet];
            for(int ii=0; ii<nDet; ii++)
                rho[ii] = rho0[ii];


            memset(str, 0, sizeof(str));
            sprintf(str, "%0.1f", musp);
            this->textBoxMusp->Text = gcnew String(str);

            memset(str, 0, sizeof(str));
            sprintf(str, "%0.1f", mua);
            this->textBoxMua->Text = gcnew String(str);

            memset(str, 0, sizeof(str));
            sprintf(str, "%0.2g", lamda);
            this->textBoxLamda->Text = gcnew String(str);

            memset(str, 0, sizeof(str));
            sprintf(str, "%0.1f", rho[0]);
            this->textBoxSrcDet1Separation->Text = gcnew String(str);

            memset(str, 0, sizeof(str));
            sprintf(str, "%0.1f", rho[1]);
            this->textBoxSrcDet2Separation->Text = gcnew String(str);

            memset(str, 0, sizeof(str));
            sprintf(str, "%0.1f", rho[2]);
            this->textBoxSrcDet3Separation->Text = gcnew String(str);

            memset(str, 0, sizeof(str));
            sprintf(str, "%0.1f", rho[3]);
            this->textBoxSrcDet4Separation->Text = gcnew String(str);

            submitflag = false;
            cancelflag = false;
        }

    protected:
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        ~LightAndTissPropertiesForm()
        {
            if(components)
            {
                delete components;
            }
        }
    private: System::Windows::Forms::TextBox^  textBoxMua;
    private: System::Windows::Forms::TextBox^  textBoxMusp;
    private: System::Windows::Forms::TextBox^  textBoxLamda;

    protected:

    protected:




    private: System::Windows::Forms::Label^  labelMua;
    private: System::Windows::Forms::Label^  label2;
    private: System::Windows::Forms::Label^  labelLamda;


    private: System::Windows::Forms::Button^  buttonSubmit;
    private: System::Windows::Forms::Button^  buttonCancel;
    private: System::Windows::Forms::TextBox^  textBoxSrcDet1Separation;
    private: System::Windows::Forms::Label^  labelSrcDet1Separation;
    private: System::Windows::Forms::TextBox^  textBoxSrcDet2Separation;

    private: System::Windows::Forms::Label^  labelSrcDet2Separation;
    private: System::Windows::Forms::TextBox^  textBoxSrcDet3Separation;

    private: System::Windows::Forms::Label^  labelSrcDet3Separation;
    private: System::Windows::Forms::TextBox^  textBoxSrcDet4Separation;

    private: System::Windows::Forms::Label^  labelSrcDet4Separation;
    private: System::Windows::Forms::GroupBox^  groupBoxSrcDetSeparation;
    private: System::Windows::Forms::GroupBox^  groupBoxTissueProperties;
    private: System::Windows::Forms::GroupBox^  groupBox2;

    private:
        /// <summary>
        /// Required designer variable.
        /// </summary>
        System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        void InitializeComponent(void)
        {
            this->textBoxMua = (gcnew System::Windows::Forms::TextBox());
            this->textBoxMusp = (gcnew System::Windows::Forms::TextBox());
            this->textBoxLamda = (gcnew System::Windows::Forms::TextBox());
            this->labelMua = (gcnew System::Windows::Forms::Label());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->labelLamda = (gcnew System::Windows::Forms::Label());
            this->buttonSubmit = (gcnew System::Windows::Forms::Button());
            this->buttonCancel = (gcnew System::Windows::Forms::Button());
            this->textBoxSrcDet1Separation = (gcnew System::Windows::Forms::TextBox());
            this->labelSrcDet1Separation = (gcnew System::Windows::Forms::Label());
            this->textBoxSrcDet2Separation = (gcnew System::Windows::Forms::TextBox());
            this->labelSrcDet2Separation = (gcnew System::Windows::Forms::Label());
            this->textBoxSrcDet3Separation = (gcnew System::Windows::Forms::TextBox());
            this->labelSrcDet3Separation = (gcnew System::Windows::Forms::Label());
            this->textBoxSrcDet4Separation = (gcnew System::Windows::Forms::TextBox());
            this->labelSrcDet4Separation = (gcnew System::Windows::Forms::Label());
            this->groupBoxSrcDetSeparation = (gcnew System::Windows::Forms::GroupBox());
            this->groupBoxTissueProperties = (gcnew System::Windows::Forms::GroupBox());
            this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
            this->groupBoxSrcDetSeparation->SuspendLayout();
            this->groupBoxTissueProperties->SuspendLayout();
            this->groupBox2->SuspendLayout();
            this->SuspendLayout();
            // 
            // textBoxMua
            // 
            this->textBoxMua->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxMua->Location = System::Drawing::Point(164, 49);
            this->textBoxMua->Margin = System::Windows::Forms::Padding(4);
            this->textBoxMua->Name = L"textBoxMua";
            this->textBoxMua->Size = System::Drawing::Size(132, 26);
            this->textBoxMua->TabIndex = 0;
            // 
            // textBoxMusp
            // 
            this->textBoxMusp->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxMusp->Location = System::Drawing::Point(164, 93);
            this->textBoxMusp->Margin = System::Windows::Forms::Padding(4);
            this->textBoxMusp->Name = L"textBoxMusp";
            this->textBoxMusp->Size = System::Drawing::Size(132, 26);
            this->textBoxMusp->TabIndex = 1;
            // 
            // textBoxLamda
            // 
            this->textBoxLamda->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBoxLamda->Location = System::Drawing::Point(167, 65);
            this->textBoxLamda->Margin = System::Windows::Forms::Padding(4);
            this->textBoxLamda->Name = L"textBoxLamda";
            this->textBoxLamda->Size = System::Drawing::Size(132, 26);
            this->textBoxLamda->TabIndex = 3;
            // 
            // labelMua
            // 
            this->labelMua->AutoSize = true;
            this->labelMua->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->labelMua->Location = System::Drawing::Point(84, 51);
            this->labelMua->Margin = System::Windows::Forms::Padding(4, 0, 4, 0);
            this->labelMua->Name = L"labelMua";
            this->labelMua->Size = System::Drawing::Size(62, 24);
            this->labelMua->TabIndex = 5;
            this->labelMua->Text = L"Mua :";
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label2->Location = System::Drawing::Point(74, 95);
            this->label2->Margin = System::Windows::Forms::Padding(4, 0, 4, 0);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(73, 24);
            this->label2->TabIndex = 6;
            this->label2->Text = L"Musp :";
            // 
            // labelLamda
            // 
            this->labelLamda->AutoSize = true;
            this->labelLamda->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->labelLamda->Location = System::Drawing::Point(17, 66);
            this->labelLamda->Margin = System::Windows::Forms::Padding(4, 0, 4, 0);
            this->labelLamda->Name = L"labelLamda";
            this->labelLamda->Size = System::Drawing::Size(132, 24);
            this->labelLamda->TabIndex = 7;
            this->labelLamda->Text = L"Wavelength :";
            // 
            // buttonSubmit
            // 
            this->buttonSubmit->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->buttonSubmit->Location = System::Drawing::Point(249, 428);
            this->buttonSubmit->Margin = System::Windows::Forms::Padding(4);
            this->buttonSubmit->Name = L"buttonSubmit";
            this->buttonSubmit->Size = System::Drawing::Size(112, 46);
            this->buttonSubmit->TabIndex = 9;
            this->buttonSubmit->Text = L"Submit";
            this->buttonSubmit->UseVisualStyleBackColor = true;
            this->buttonSubmit->Click += gcnew System::EventHandler(this, &LightAndTissPropertiesForm::buttonSubmit_Click);
            // 
            // buttonCancel
            // 
            this->buttonCancel->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->buttonCancel->Location = System::Drawing::Point(431, 428);
            this->buttonCancel->Margin = System::Windows::Forms::Padding(4);
            this->buttonCancel->Name = L"buttonCancel";
            this->buttonCancel->Size = System::Drawing::Size(112, 46);
            this->buttonCancel->TabIndex = 10;
            this->buttonCancel->Text = L"Cancel";
            this->buttonCancel->UseVisualStyleBackColor = true;
            this->buttonCancel->Click += gcnew System::EventHandler(this, &LightAndTissPropertiesForm::buttonCancel_Click);
            // 
            // textBoxSrcDet1Separation
            // 
            this->textBoxSrcDet1Separation->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->textBoxSrcDet1Separation->Location = System::Drawing::Point(151, 51);
            this->textBoxSrcDet1Separation->Margin = System::Windows::Forms::Padding(4);
            this->textBoxSrcDet1Separation->Name = L"textBoxSrcDet1Separation";
            this->textBoxSrcDet1Separation->Size = System::Drawing::Size(132, 26);
            this->textBoxSrcDet1Separation->TabIndex = 4;
            // 
            // labelSrcDet1Separation
            // 
            this->labelSrcDet1Separation->AutoSize = true;
            this->labelSrcDet1Separation->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->labelSrcDet1Separation->Location = System::Drawing::Point(60, 51);
            this->labelSrcDet1Separation->Margin = System::Windows::Forms::Padding(4, 0, 4, 0);
            this->labelSrcDet1Separation->Name = L"labelSrcDet1Separation";
            this->labelSrcDet1Separation->Size = System::Drawing::Size(70, 24);
            this->labelSrcDet1Separation->TabIndex = 8;
            this->labelSrcDet1Separation->Text = L"Det 1 :";
            // 
            // textBoxSrcDet2Separation
            // 
            this->textBoxSrcDet2Separation->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->textBoxSrcDet2Separation->Location = System::Drawing::Point(151, 109);
            this->textBoxSrcDet2Separation->Margin = System::Windows::Forms::Padding(4);
            this->textBoxSrcDet2Separation->Name = L"textBoxSrcDet2Separation";
            this->textBoxSrcDet2Separation->Size = System::Drawing::Size(132, 26);
            this->textBoxSrcDet2Separation->TabIndex = 9;
            // 
            // labelSrcDet2Separation
            // 
            this->labelSrcDet2Separation->AutoSize = true;
            this->labelSrcDet2Separation->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->labelSrcDet2Separation->Location = System::Drawing::Point(60, 109);
            this->labelSrcDet2Separation->Margin = System::Windows::Forms::Padding(4, 0, 4, 0);
            this->labelSrcDet2Separation->Name = L"labelSrcDet2Separation";
            this->labelSrcDet2Separation->Size = System::Drawing::Size(70, 24);
            this->labelSrcDet2Separation->TabIndex = 10;
            this->labelSrcDet2Separation->Text = L"Det 2 :";
            // 
            // textBoxSrcDet3Separation
            // 
            this->textBoxSrcDet3Separation->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->textBoxSrcDet3Separation->Location = System::Drawing::Point(151, 163);
            this->textBoxSrcDet3Separation->Margin = System::Windows::Forms::Padding(4);
            this->textBoxSrcDet3Separation->Name = L"textBoxSrcDet3Separation";
            this->textBoxSrcDet3Separation->Size = System::Drawing::Size(132, 26);
            this->textBoxSrcDet3Separation->TabIndex = 11;
            // 
            // labelSrcDet3Separation
            // 
            this->labelSrcDet3Separation->AutoSize = true;
            this->labelSrcDet3Separation->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->labelSrcDet3Separation->Location = System::Drawing::Point(60, 163);
            this->labelSrcDet3Separation->Margin = System::Windows::Forms::Padding(4, 0, 4, 0);
            this->labelSrcDet3Separation->Name = L"labelSrcDet3Separation";
            this->labelSrcDet3Separation->Size = System::Drawing::Size(70, 24);
            this->labelSrcDet3Separation->TabIndex = 12;
            this->labelSrcDet3Separation->Text = L"Det 3 :";
            // 
            // textBoxSrcDet4Separation
            // 
            this->textBoxSrcDet4Separation->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->textBoxSrcDet4Separation->Location = System::Drawing::Point(151, 219);
            this->textBoxSrcDet4Separation->Margin = System::Windows::Forms::Padding(4);
            this->textBoxSrcDet4Separation->Name = L"textBoxSrcDet4Separation";
            this->textBoxSrcDet4Separation->Size = System::Drawing::Size(132, 26);
            this->textBoxSrcDet4Separation->TabIndex = 13;
            // 
            // labelSrcDet4Separation
            // 
            this->labelSrcDet4Separation->AutoSize = true;
            this->labelSrcDet4Separation->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->labelSrcDet4Separation->Location = System::Drawing::Point(60, 219);
            this->labelSrcDet4Separation->Margin = System::Windows::Forms::Padding(4, 0, 4, 0);
            this->labelSrcDet4Separation->Name = L"labelSrcDet4Separation";
            this->labelSrcDet4Separation->Size = System::Drawing::Size(70, 24);
            this->labelSrcDet4Separation->TabIndex = 14;
            this->labelSrcDet4Separation->Text = L"Det 4 :";
            // 
            // groupBoxSrcDetSeparation
            // 
            this->groupBoxSrcDetSeparation->Controls->Add(this->textBoxSrcDet1Separation);
            this->groupBoxSrcDetSeparation->Controls->Add(this->labelSrcDet4Separation);
            this->groupBoxSrcDetSeparation->Controls->Add(this->labelSrcDet1Separation);
            this->groupBoxSrcDetSeparation->Controls->Add(this->textBoxSrcDet2Separation);
            this->groupBoxSrcDetSeparation->Controls->Add(this->textBoxSrcDet4Separation);
            this->groupBoxSrcDetSeparation->Controls->Add(this->labelSrcDet2Separation);
            this->groupBoxSrcDetSeparation->Controls->Add(this->textBoxSrcDet3Separation);
            this->groupBoxSrcDetSeparation->Controls->Add(this->labelSrcDet3Separation);
            this->groupBoxSrcDetSeparation->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->groupBoxSrcDetSeparation->Location = System::Drawing::Point(402, 40);
            this->groupBoxSrcDetSeparation->Name = L"groupBoxSrcDetSeparation";
            this->groupBoxSrcDetSeparation->Size = System::Drawing::Size(327, 310);
            this->groupBoxSrcDetSeparation->TabIndex = 15;
            this->groupBoxSrcDetSeparation->TabStop = false;
            this->groupBoxSrcDetSeparation->Text = L"Src/Det Separation";
            // 
            // groupBoxTissueProperties
            // 
            this->groupBoxTissueProperties->Controls->Add(this->textBoxMua);
            this->groupBoxTissueProperties->Controls->Add(this->textBoxMusp);
            this->groupBoxTissueProperties->Controls->Add(this->labelMua);
            this->groupBoxTissueProperties->Controls->Add(this->label2);
            this->groupBoxTissueProperties->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold,
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
            this->groupBoxTissueProperties->Location = System::Drawing::Point(28, 40);
            this->groupBoxTissueProperties->Name = L"groupBoxTissueProperties";
            this->groupBoxTissueProperties->Size = System::Drawing::Size(344, 151);
            this->groupBoxTissueProperties->TabIndex = 16;
            this->groupBoxTissueProperties->TabStop = false;
            this->groupBoxTissueProperties->Text = L"Tissue Properties";
            // 
            // groupBox2
            // 
            this->groupBox2->Controls->Add(this->textBoxLamda);
            this->groupBox2->Controls->Add(this->labelLamda);
            this->groupBox2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->groupBox2->Location = System::Drawing::Point(28, 208);
            this->groupBox2->Name = L"groupBox2";
            this->groupBox2->Size = System::Drawing::Size(344, 142);
            this->groupBox2->TabIndex = 17;
            this->groupBox2->TabStop = false;
            this->groupBox2->Text = L"Laser Settings";
            // 
            // LightAndTissPropertiesForm
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(791, 509);
            this->Controls->Add(this->groupBox2);
            this->Controls->Add(this->groupBoxTissueProperties);
            this->Controls->Add(this->groupBoxSrcDetSeparation);
            this->Controls->Add(this->buttonCancel);
            this->Controls->Add(this->buttonSubmit);
            this->Margin = System::Windows::Forms::Padding(4);
            this->Name = L"LightAndTissPropertiesForm";
            this->Text = L"LightAndTissPropertiesForm";
            this->groupBoxSrcDetSeparation->ResumeLayout(false);
            this->groupBoxSrcDetSeparation->PerformLayout();
            this->groupBoxTissueProperties->ResumeLayout(false);
            this->groupBoxTissueProperties->PerformLayout();
            this->groupBox2->ResumeLayout(false);
            this->groupBox2->PerformLayout();
            this->ResumeLayout(false);

        }
#pragma endregion

    private:

        // -----------------------------------------------------------------------
        System::Void buttonSubmit_Click(System::Object^  sender, System::EventArgs^  e)
        {
            char str[20];

            memset(str, 0, sizeof(str));
            convertStringToChar_local(this->textBoxMua->Text, str);
            mua = atof(str);

            memset(str, 0, sizeof(str));
            convertStringToChar_local(this->textBoxMusp->Text, str);
            musp = atof(str);

            memset(str, 0, sizeof(str));
            convertStringToChar_local(this->textBoxSrcDet1Separation->Text, str);
            rho[0] = atof(str);
            memset(str, 0, sizeof(str));
            convertStringToChar_local(this->textBoxSrcDet2Separation->Text, str);
            rho[1] = atof(str);
            memset(str, 0, sizeof(str));
            convertStringToChar_local(this->textBoxSrcDet3Separation->Text, str);
            rho[2] = atof(str);
            memset(str, 0, sizeof(str));
            convertStringToChar_local(this->textBoxSrcDet4Separation->Text, str);
            rho[3] = atof(str);

            memset(str, 0, sizeof(str));
            convertStringToChar_local(this->textBoxLamda->Text, str);
            lamda = atof(str);

            submitflag = true;

            this->Close();
        }



        // -----------------------------------------------------------------------
        System::Void buttonCancel_Click(System::Object^  sender, System::EventArgs^  e)
        {
            this->Close();
            cancelflag = true;
        }


        // -----------------------------------------------------------------------
        int convertStringToChar_local(String^ src, char* dst)
        {
            pin_ptr<const wchar_t> str = PtrToStringChars(src);

            size_t convertedChars = 0;
            size_t  sizeInBytes = ((src->Length + 1) * 2);
            errno_t err = 0;

            err = wcstombs_s(&convertedChars, dst, sizeInBytes, str, sizeInBytes);

            return 0;
        }


    public:


        // -----------------------------------------------------------------------------
        double getMusp()
        {
            return musp;
        }


        // -----------------------------------------------------------------------------
        double getMua()
        {
            return mua;
        }


        // -----------------------------------------------------------------------------
        double getLamda()
        {
            return lamda;
        }


        // -----------------------------------------------------------------------------
        double getSeparation(int iCh)
        {
            return rho[iCh];
        }


        // -----------------------------------------------------------------------------
        bool cancelled()
        {
            return cancelflag;
        }


    private:

        double  mua;
        double  musp;
        double  lamda;
        double* rho;

        int     nDet;

        bool submitflag;
        bool cancelflag;
    };
}
