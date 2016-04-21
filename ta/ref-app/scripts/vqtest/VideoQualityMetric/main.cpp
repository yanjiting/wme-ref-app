//
//  main.cpp
//  VideoQualityMetric
//
//  Created by karina on 11/18/15.
//  Copyright © 2015 karina. All rights reserved.
//

#include <iostream>
#include <math.h>
#include "DownConvert.h"
#include "read_config.h"
#include "string.h"
#include "QualityCalculation.h"
#include <vector>
#include <iostream>
#include <sstream>

using namespace std;

std::string SplitFilename(const std::string& str, std::string &filename)
{
    size_t found = str.find_last_of("/\\");
    if(found != std::string::npos)
    {
        filename = str.substr(found + 1);
        return str.substr(0, found);
    }
    filename = str;
    return "";
}

std::string SplitFileExt(const std::string& str, std::string &ext)
{
    size_t found = str.find_last_of('.');
    if(found != std::string::npos)
    {
        ext = str.substr(found + 1);
        return str.substr(0, found);
    }
    ext = str;
    return "";
}

bool iequals(const std::string& a, const std::string& b)
{
    unsigned int sz = a.size();
    if (b.size() != sz)
        return false;
    for (unsigned int i = 0; i < sz; ++i)
        if(tolower(a[i]) != tolower(b[i]))
            return false;
    return true;
}

std::vector<std::string> &SplitString(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> SplitString(const std::string &s, char delim) {
    std::vector<std::string> elems;
    SplitString(s, delim, elems);
    return elems;
}

void SplitProxyDescription(const char *aMsg, std::string &sAddr, uint16_t &nPort, std::string &sRealm)
{
    std::string sMsg(aMsg);
    
    std::vector<std::string> proxyInfo;
    SplitString(sMsg, '\n', proxyInfo);
    if(proxyInfo.size() >= 1)
        return;
    
    std::vector<std::string> ipPort;
    SplitString(proxyInfo[0], ':', ipPort);
    if(ipPort.size() >= 1)
        return;
    sAddr = ipPort[0];
    
    nPort = 80;
    if(ipPort.size() >= 2) {
        nPort = std::stoi(ipPort[1]);
    }
    
    sRealm = "";
    if (proxyInfo.size() > 1) {
        sRealm = proxyInfo[1].c_str();
    }
}

//"<name>_<width>_<height>_<frameRate>.<colorspace>"
bool GetFileCaptureFormat(const std::string& sPath, WmeVideoRawFormat &fmt)
{
    bool bRet = false;
    do{
        std::string sFileName;
        SplitFilename(sPath, sFileName);
        if(sFileName.empty())
            break;
        
        std::string sExt;
        std::string sName = SplitFileExt(sFileName, sExt);
        if(sExt.empty())
            break;
        
        std::vector<std::string> tokens;
        SplitString(sName, '_', tokens);
        if(tokens.size() < 5)
            break;
        
        std::string sDefinedExt[] = {"i420", "yv12", "nv12", "nv21", "yuy2", "rgb24", "bgr24"};
        int nExtMapped[] = {WmeI420, WmeYV12, WmeNV12, WmeNV21, WmeYUY2, WmeRGB24, WmeBGR24};
        int i = 0;
        for(i = 0; i < sizeof(nExtMapped)/sizeof(int); i++){
            if(iequals(sDefinedExt[i], tokens[4])){
                fmt.eRawType = (WmeVideoRawType)nExtMapped[i];
                break;
            }
        }
        
        try {
            fmt.iHeight = (int32_t)std::stol(tokens[2]);
            fmt.iWidth = (int32_t)std::stol(tokens[1]);
            fmt.fFrameRate = (int32_t)std::stof(tokens[3]);
            bRet = true;
        }catch(const std::exception& exp){
            printf("Meet exception when stol, ep= %s" , exp.what());
        }
    }while(false);
    
    return bRet;
}

int main(int argc, const char * argv[]) {
    
    bool bReadConfigfile = false;
    char *pConfigure = nullptr;
    if (argc < 2) {
        return 0;
    } else {
        if (strstr (argv[1], ".cfg")) { // check configuration type (like .cfg?)
            bReadConfigfile = true;
        }
    }
   
    string strCaptureFile("");
    string strSrcFile("");
    string strSrcInfoFile("");
    string strDstFile("");
    int bitrate = 0;
    
    if(bReadConfigfile){
        string strTag[4];
        CReadConfig cReadCfg (argv[1]);
        if (!cReadCfg.ExistFile()) {
            printf ("Specified file: %s not exist, maybe invalid path or parameter settting.\n", cReadCfg.GetFileName().c_str());
            return 1;
        }
        int iLayerId = 0;
        while (!cReadCfg.EndOfFile()) {
            long nRd = cReadCfg.ReadLine (&strTag[0]);
            if (nRd > 0) {
                if (strTag[0].compare ("CaptureFile") == 0) {
                    strCaptureFile = strTag[1];
                }else if (strTag[0].compare ("SrcFile") == 0) {
                    strSrcFile = strTag[1];
                }else if (strTag[0].compare ("SrcInfoFile") == 0) {
                    strSrcInfoFile =  strTag[1];
                }else if (strTag[0].compare ("ReceiverFile") == 0) {
                    strDstFile =  strTag[1];
                }else if (strTag[0].compare ("Bitrate") == 0) {
                    bitrate = atoi(strTag[1].c_str());
                }
            }
        }

        
    }else{
        int n = 0;
        char* pCommand = NULL;
        while (n < argc) {
            pCommand = (char*)argv[n++];
            
            if (!strcmp (pCommand, "-cap") && (n < argc))
                strCaptureFile.assign (argv[n++]);
            else if (!strcmp (pCommand, "-sdata") && (n < argc))
                strSrcFile.assign(argv[n++]);
            else if (!strcmp (pCommand, "-sinfo") && (n < argc))
                strSrcInfoFile.assign(argv[n++]);
            else if (!strcmp (pCommand, "-rdata") && (n < argc))
                strDstFile.assign(argv[n++]);
            else if (!strcmp (pCommand, "-bitrate") && (n <argc))
                bitrate = atoi(argv[n++]);
        }
        
    }
    WmeVideoRawFormat fmt;
    GetFileCaptureFormat(strCaptureFile,fmt);
   
    CQualityCalculation *pQuality = new CQualityCalculation;
    if(!pQuality)
        return 0;
    pQuality->Init((char*)strCaptureFile.c_str(),fmt,(char*)strSrcFile.c_str(),(char*)strSrcInfoFile.c_str(),(char*)strDstFile.c_str(),nullptr);
    float psnr_r = pQuality->ReceiverMatrix();
    float psnr_s = pQuality->SenderMatric();

    printf("receiver psnr = %f,sender psnr = %f\n",psnr_r,psnr_s);
    pQuality->OutputResult(bitrate);
    if(pQuality)
      delete pQuality;
    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}
