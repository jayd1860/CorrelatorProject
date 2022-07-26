using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FPGAConfig
{
    public class ConfigFormat
    {
        public string ProjectRootDir { get; set; }
        public string FX3_FPGAFirmwareUploadExe { get; set; }
        public string FPGA_FirmwareExe { get; set; }
        public string FX3_StreamerFirmwareExe { get; set; }
    }
}
