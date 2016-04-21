using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Coverage2XML
{
    class Program
    {
        static void Main(string[] args)
        {
            string exePath = args[0];
            string pdbPath = args[1];
            string covPath = args[2];
            string htmlPath = args[3];

            CCovInfo cov = new CCovInfo();
            cov.GenerageReport(pdbPath, exePath, covPath, htmlPath); 
        }
    }
}
