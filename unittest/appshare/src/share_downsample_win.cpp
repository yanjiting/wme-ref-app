#include "timer.h"
#include <atlbase.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atlimage.h>
//#include <atlmisc.h>
#include <atlconv.h>
#include <OCIdl.h>
#include <OleCtl.h>
#include <string>
#include "share_util.h"

HBITMAP LoadBmpFromImageFile(LPCTSTR lpszFileName)
{
    HBITMAP hBitmap = NULL;
    //Gdiplus::Bitmap bmp(lpszFileName, false);
    CImage image;
    image.Load(lpszFileName);
    hBitmap = image.Detach();
    return hBitmap;
}
/*nQualityLevel
1: low, 2: media, 3: High */
int SystemDownSample(const char *inputFile, int nWidth, int nHeight, int nQualityLevel, bool bOutPutFile,int nTimes)
{
    int nRet = -1;
    char *pData = NULL;
    CBitmap bmpSource;
    CWindowDC dcScreen(NULL);
    CDC hdcMemDest = CreateCompatibleDC(dcScreen);
    do {
        USES_CONVERSION;
        //load image from file
        std::string strInputPath = inputFile;
        if (strInputPath.find('\\') == std::string::npos) {
            char szPath[MAX_PATH] = { 0 };
            GetModuleFileNameA((HMODULE)NULL, szPath, MAX_PATH);
            std::string strNewInputPath = szPath;
            size_t found = strNewInputPath.rfind('\\');
            if (found != std::string::npos) {
                strNewInputPath.replace(found, std::string::npos, "\\");
            }
            strNewInputPath += strInputPath;
            strInputPath = strNewInputPath;
        }
        bmpSource = LoadBmpFromImageFile(A2W(strInputPath.c_str()));
        if (bmpSource.IsNull()) {
            break;
        }
        SIZE szBitmap  ;
        bmpSource.GetSize(szBitmap);

        //get raw data from HBITMAP to simulate the screen capture output
        CDC dcTemp = CreateCompatibleDC(dcScreen);
        dcTemp.SelectBitmap(bmpSource);

        BITMAPINFO  bmTmp = { 0 };
        bmTmp.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmTmp.bmiHeader.biPlanes = 1;
        bmTmp.bmiHeader.biBitCount = 32;
        bmTmp.bmiHeader.biCompression = BI_RGB;
        bmTmp.bmiHeader.biWidth = szBitmap.cx;
        bmTmp.bmiHeader.biHeight = szBitmap.cy;
        pData = new char[szBitmap.cx*szBitmap.cy*4];
        int nRetTmp = GetDIBits(dcTemp, bmpSource, 0,
                                (UINT)szBitmap.cy,
                                pData,
                                (BITMAPINFO *)&bmTmp, DIB_RGB_COLORS);
        if (nRetTmp <= 0) { break; }

        //prepare for dest for testing downsample .
        WBXSize szSource = { szBitmap.cx, szBitmap.cy };
        WBXSize szDest = { nWidth, nHeight };
        szDest = WbxGraphicUtil::WbxGetMaxCapSize(szSource, szDest);
        BITMAPINFO  bm = { 0 };
        bm.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bm.bmiHeader.biPlanes = 1;
        bm.bmiHeader.biBitCount = 32;
        bm.bmiHeader.biCompression = BI_RGB;
        bm.bmiHeader.biWidth = szDest.cx;
        bm.bmiHeader.biHeight = szDest.cy;
        void *pDestData = NULL;
        CBitmap hbmDest = CreateDIBSection(hdcMemDest, &bm, DIB_RGB_COLORS, &pDestData, NULL, 0);
        if (hbmDest.IsNull()) { break; }

        //real downsample by GDI
        int nStart = ticker::now();
        for (int i=0; i<nTimes; i++) {
            if (nQualityLevel>1) {
                SetStretchBltMode(hdcMemDest, HALFTONE);
            }
            HGDIOBJ hbmOldDest = SelectObject(hdcMemDest, hbmDest);
            bm.bmiHeader.biWidth = szSource.cx;
            bm.bmiHeader.biHeight = szSource.cy;
            StretchDIBits(hdcMemDest, 0, 0, szDest.cx, szDest.cy, 0, 0, szSource.cx, szSource.cy, pData, &bm, DIB_RGB_COLORS, SRCCOPY);
            SelectObject(hdcMemDest, hbmOldDest);
        }
        nRet = ticker::now() - nStart;
        if (bOutPutFile) {
            CImage image;
            image.Attach(hbmDest);
            std::stringstream streamPngFilePath;
            streamPngFilePath << strInputPath << "_" << nWidth << "_" << nHeight << "_" << nQualityLevel << ".png";
            image.Save(A2W(streamPngFilePath.str().c_str())); // change extension to save to png
        }
    } while (0);

    //clean
    if (pData) { delete[] pData; }
    return nRet;
}

int SystemDownSamplePerformanceTest(const char *inputFile, int nWidth, int nHeight, int nQualityLevel, int nTimes)
{
    return SystemDownSample(inputFile,nWidth,nHeight,nQualityLevel,false,nTimes);
}

int SystemDownSampleQualityTest(const char *inputFile, int nWidth, int nHeight, int nQualityLevel)
{
    return SystemDownSample(inputFile, nWidth, nHeight, nQualityLevel, true, 1);
}
