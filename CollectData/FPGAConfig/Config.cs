using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace FPGAConfig
{
    using Newtonsoft.Json;

    public class Config
    {
        public Config()
        {
            string json = string.Empty;
            int ii=0;

            while (ii < cfgfilepath.Length)
            {
                if (File.Exists(cfgfilepath[ii]))
                    break;
                ii++ ;
            }
            if (ii >= cfgfilepath.Length)
                return;

            using (StreamReader r = new StreamReader(cfgfilepath[ii]))
            {
                json = r.ReadToEnd();
                ConfigFormat cfginfo = JsonConvert.DeserializeObject<ConfigFormat>(json);
                ProjectRootDir = cfginfo.ProjectRootDir;
                FX3_FPGAFirmwareUploadExe = cfginfo.FX3_FPGAFirmwareUploadExe;
                FPGA_FirmwareExe = cfginfo.FPGA_FirmwareExe;
                FX3_StreamerFirmwareExe = cfginfo.FX3_StreamerFirmwareExe;
            }
        }

        public string getFX3_FPGAFirmwareUploadExe()
        {
            return ProjectRootDir + "\\" + FX3_FPGAFirmwareUploadExe;
        }

        public string getFPGA_FirmwareExe()
        {
            return ProjectRootDir + "\\" + FPGA_FirmwareExe;
        }

        public string getFX3_StreamerFirmwareExe()
        {
            return ProjectRootDir + "\\" + FX3_StreamerFirmwareExe;
        }

        private string[] cfgfilepath = {
            ".\\Config.json",
            "..\\FPGAConfig\\Config.json"
        };

        private string ProjectRootDir;
        private string FX3_FPGAFirmwareUploadExe;
        private string FPGA_FirmwareExe;
        private string FX3_StreamerFirmwareExe;
    }
}
