namespace FPGAConfig
{
    partial class FPGAConfigForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FPGAConfigForm));
            this.FOpenDialog = new System.Windows.Forms.OpenFileDialog();
            this.StatLabel = new System.Windows.Forms.StatusStrip();
            this.StatLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.button_ConfigureFpga = new System.Windows.Forms.Button();
            this.button_SelectFpgaBitstream = new System.Windows.Forms.Button();
            this.button_UploadFx3FpgaFirmware = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.button_InitDevice = new System.Windows.Forms.Button();
            this.rtConsole = new System.Windows.Forms.RichTextBox();
            this.button_UploadFx3Streamer = new System.Windows.Forms.Button();
            this.StatLabel.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // FOpenDialog
            // 
            this.FOpenDialog.FileName = "openFileDialog1";
            // 
            // StatLabel
            // 
            this.StatLabel.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.StatLabel.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.StatLabel1});
            this.StatLabel.Location = new System.Drawing.Point(0, 577);
            this.StatLabel.Name = "StatLabel";
            this.StatLabel.Padding = new System.Windows.Forms.Padding(1, 0, 19, 0);
            this.StatLabel.Size = new System.Drawing.Size(890, 22);
            this.StatLabel.TabIndex = 6;
            this.StatLabel.Text = "statusStrip1";
            // 
            // StatLabel1
            // 
            this.StatLabel1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.StatLabel1.Name = "StatLabel1";
            this.StatLabel1.Size = new System.Drawing.Size(164, 17);
            this.StatLabel1.Text = "toolStripStatusLabel1";
            // 
            // button_ConfigureFpga
            // 
            this.button_ConfigureFpga.Location = new System.Drawing.Point(391, 218);
            this.button_ConfigureFpga.Margin = new System.Windows.Forms.Padding(4);
            this.button_ConfigureFpga.Name = "button_ConfigureFpga";
            this.button_ConfigureFpga.Size = new System.Drawing.Size(173, 36);
            this.button_ConfigureFpga.TabIndex = 8;
            this.button_ConfigureFpga.Text = "Configure FPGA";
            this.button_ConfigureFpga.UseVisualStyleBackColor = true;
            this.button_ConfigureFpga.Click += new System.EventHandler(this.button_ConfigureFpga_Click);
            // 
            // button_SelectFpgaBitstream
            // 
            this.button_SelectFpgaBitstream.Location = new System.Drawing.Point(8, 218);
            this.button_SelectFpgaBitstream.Margin = new System.Windows.Forms.Padding(4);
            this.button_SelectFpgaBitstream.Name = "button_SelectFpgaBitstream";
            this.button_SelectFpgaBitstream.Size = new System.Drawing.Size(173, 36);
            this.button_SelectFpgaBitstream.TabIndex = 5;
            this.button_SelectFpgaBitstream.Text = "Select FPGA Bitstream";
            this.button_SelectFpgaBitstream.UseVisualStyleBackColor = true;
            this.button_SelectFpgaBitstream.Click += new System.EventHandler(this.button_SelectFpgaBitstream_Click);
            // 
            // button_UploadFx3FpgaFirmware
            // 
            this.button_UploadFx3FpgaFirmware.Location = new System.Drawing.Point(188, 156);
            this.button_UploadFx3FpgaFirmware.Margin = new System.Windows.Forms.Padding(4);
            this.button_UploadFx3FpgaFirmware.Name = "button_UploadFx3FpgaFirmware";
            this.button_UploadFx3FpgaFirmware.Size = new System.Drawing.Size(199, 33);
            this.button_UploadFx3FpgaFirmware.TabIndex = 9;
            this.button_UploadFx3FpgaFirmware.Text = "Upload FX3 FPGA Firmware";
            this.button_UploadFx3FpgaFirmware.UseVisualStyleBackColor = true;
            this.button_UploadFx3FpgaFirmware.Click += new System.EventHandler(this.button_UploadFx3FpgaFirmware_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(0, 14);
            this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(0, 17);
            this.label1.TabIndex = 10;
            this.label1.Click += new System.EventHandler(this.label1_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.button_InitDevice);
            this.groupBox1.Controls.Add(this.button_UploadFx3FpgaFirmware);
            this.groupBox1.Controls.Add(this.button_SelectFpgaBitstream);
            this.groupBox1.Controls.Add(this.button_ConfigureFpga);
            this.groupBox1.Location = new System.Drawing.Point(44, 2);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(4);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(4);
            this.groupBox1.Size = new System.Drawing.Size(576, 332);
            this.groupBox1.TabIndex = 11;
            this.groupBox1.TabStop = false;
            // 
            // button_InitDevice
            // 
            this.button_InitDevice.Location = new System.Drawing.Point(188, 44);
            this.button_InitDevice.Name = "button_InitDevice";
            this.button_InitDevice.Size = new System.Drawing.Size(199, 43);
            this.button_InitDevice.TabIndex = 10;
            this.button_InitDevice.Text = "INITIALIZE ALL";
            this.button_InitDevice.UseVisualStyleBackColor = true;
            this.button_InitDevice.Click += new System.EventHandler(this.button_InitDevice_Click);
            // 
            // rtConsole
            // 
            this.rtConsole.Location = new System.Drawing.Point(44, 342);
            this.rtConsole.Margin = new System.Windows.Forms.Padding(4);
            this.rtConsole.Name = "rtConsole";
            this.rtConsole.Size = new System.Drawing.Size(576, 220);
            this.rtConsole.TabIndex = 12;
            this.rtConsole.Text = "";
            this.rtConsole.TextChanged += new System.EventHandler(this.rtConsole_TextChanged);
            // 
            // button_UploadFx3Streamer
            // 
            this.button_UploadFx3Streamer.Location = new System.Drawing.Point(232, 278);
            this.button_UploadFx3Streamer.Margin = new System.Windows.Forms.Padding(4);
            this.button_UploadFx3Streamer.Name = "button_UploadFx3Streamer";
            this.button_UploadFx3Streamer.Size = new System.Drawing.Size(199, 35);
            this.button_UploadFx3Streamer.TabIndex = 13;
            this.button_UploadFx3Streamer.Text = "Upload FX3 Streamer";
            this.button_UploadFx3Streamer.UseVisualStyleBackColor = true;
            this.button_UploadFx3Streamer.Click += new System.EventHandler(this.button_UploadFx3Streamer_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Control;
            this.ClientSize = new System.Drawing.Size(890, 599);
            this.Controls.Add(this.button_UploadFx3Streamer);
            this.Controls.Add(this.rtConsole);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.StatLabel);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "Form1";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "FPGA Configuration Utility";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.StatLabel.ResumeLayout(false);
            this.StatLabel.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.OpenFileDialog FOpenDialog;
        private System.Windows.Forms.StatusStrip StatLabel;
        private System.Windows.Forms.Button button_ConfigureFpga;
        private System.Windows.Forms.Button button_SelectFpgaBitstream;
        private System.Windows.Forms.ToolStripStatusLabel StatLabel1;
        private System.Windows.Forms.Button button_UploadFx3FpgaFirmware;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RichTextBox rtConsole;
        private System.Windows.Forms.Button button_UploadFx3Streamer;
        private System.Windows.Forms.Button button_InitDevice;
    }
}

