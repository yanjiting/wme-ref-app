using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Reflection;
using System.Xml.Xsl;
using Microsoft.VisualStudio.CodeCoverage;

namespace Coverage2XML
{
    public interface ICovInfo
    {
        void GenerageReport(string bstrSymbolPath, string bstrExePath, string bstrCovPath, string bstrHtmlPath);
    };

    // Interface implementation.
    public class CCovInfo : ICovInfo
    {
        public void GenerageReport(string bstrSymbolPath, string bstrExePath, string bstrCovPath, string bstrHtmlPath)
        {
            string DLLPathWithDLLName = Assembly.GetExecutingAssembly().Location;
            string DLLPath = Path.GetDirectoryName(DLLPathWithDLLName);
            string coverageXslt = Path.Combine(DLLPath, @"coverage.xslt");

            CoverageInfo coverageInfo = GetCoverageInfo(bstrCovPath, bstrSymbolPath, bstrExePath);
            string coverageOutputXmlFilename = bstrExePath;
            coverageOutputXmlFilename += @".xml";
            WriteCoverageOutputAsXml(coverageInfo, coverageOutputXmlFilename);
            
            WriteCoverageOutputWithTransfrom(coverageOutputXmlFilename, coverageXslt, bstrHtmlPath);
        }

        private static CoverageInfo GetCoverageInfo(string vstsCoverageOutputFile, string symbolsPath, string exePath)
        {
            CoverageInfoManager.SymPath = symbolsPath;
            CoverageInfoManager.ExePath = exePath;
            return CoverageInfoManager.CreateInfoFromFile(vstsCoverageOutputFile);
        }

        private static void WriteCoverageOutputWithTransfrom(string coverageOutputXmlFilename, string transformXsltFilename, string transformOutputFilename)
        {
            XslCompiledTransform transformer = new XslCompiledTransform();
            transformer.Load(transformXsltFilename);
            transformer.Transform(coverageOutputXmlFilename, transformOutputFilename);
        }

        private static void WriteCoverageOutputAsXml(CoverageInfo coverageInfo, string coverageOutputXmlFilename)
        {
            CoverageDS data = coverageInfo.BuildDataSet(null);
            data.ExportXml(coverageOutputXmlFilename);
        }
    }
}
