#ifndef _SCREEN_TRACK_VIEW_ANALYZER_H_
#define _SCREEN_TRACK_VIEW_ANALYZER_H_

#include "WmeUnknownImpTemplate.h"
#include "WmeExternalRenderer.h"
#include <vector>


#if (defined(WIN32) || defined(MACOSX))
//#if defined(WIN32)
#include "zxinghelper.h"
#endif

using namespace wme;

class CScreenTrackViewAnalyzer : public IWmeExternalRenderer,
                                 public CWmeUnknownImpl
{
public:
    IMPLEMENT_WME_REFERENCE
    IMPLEMENT_WME_QUERY_INTERFACE_UNKNOWN
    
    CScreenTrackViewAnalyzer();
    virtual ~CScreenTrackViewAnalyzer();
    
    virtual WMERESULT IsPushModeSupport(bool& isSupport);
    virtual WMERESULT RenderMediaData(IWmeMediaPackage *pPackage);
    virtual WMERESULT RenderMediaData(uint32_t uTimestamp, WmeMediaFormatType eType, void* pFormat, unsigned char *pData, uint32_t uLength);
   	virtual WMERESULT IsPullModeSupport(bool& isSupport);
    virtual WMERESULT RegisterRequestAnswerer(IWmeExternalRenderAnswerer* pAnswerer);
    virtual WMERESULT UnregisterRequestAnswerer();
    void SetQRMatrix(int nRowCount, int nColCount);
    void SetFrameSize(int nFrameWidth, int nFrameHeight);
    void SetQRRgn(int nQRRgnLeft, int nQRRgnTop, int nQRRgnRight, int nQRRgnBottom);
	std::vector<std::string> GetQRList();
	void CleanQRList();
	void SetReplayTempFiles(bool bReplayTempFiles){ m_bReplayTempFiles = bReplayTempFiles; }
	void SetQRCodeContextWithJson(std::string arg);
    void EnableDumpFrame2File(bool bDumpFrame2File){ m_bDumpFrame2File = bDumpFrame2File;}
    void EnableScanQRCode(bool bScanQRCode){ m_bScanQRCode = bScanQRCode; }
private:
	std::string DecodeQR(unsigned char *pData, WmeVideoRawType dataType, int width, int height,
                         int bytesPerPix);
	std::string GetOneQRBlockFromFrame(unsigned char *pData, WmeVideoRawType dataType,
                                       int row, int col, int block_w,
                                       int block_h, int bytesPerPix, int nFrameWidth);
    
private:
    int m_nQRRgnLeft;
    int m_nQRRgnTop;
    int m_nQRRgnRight;
    int m_nQRRgnBottom;
    int m_nQRColumns;
    int m_nQRRows;
    
    bool m_bDumpFrame2File;
    unsigned long m_nPreTickCount;
    unsigned int m_nDump2FileIndex;
    bool m_bScanQRCode;

    std::vector<std::string>  m_vctQRBlocks;
	CCmMutexThreadRecursive	 m_mutexQRBlocks;

	//Debug
	int m_nTmpBlockIndex;
	bool m_bReplayTempFiles;
#if (defined(WIN32) || defined(MACOSX))
//#if defined(WIN32)
    zxinghelp   m_zxing;
#endif
};
#include <fstream>
#include "WmeExternalCapturer.h"

struct RawFileHeader{
    char formatType[4];
    int nWidth;
    int nHieght;
    int nFrameCount;
};
class CScreenFileCapturer : public IWmeScreenCapturer,public CWmeUnknownImpl{

public:
    IMPLEMENT_WME_REFERENCE
    IMPLEMENT_WME_QUERY_INTERFACE_UNKNOWN
                                
    CScreenFileCapturer(std::string strFilePath);
    virtual ~CScreenFileCapturer();
    
    void SetFilePath(std::string strFilePath);
    void SetCaptureSkipCount(int nSkipCount);
    
    virtual WMERESULT GetCurrentCaptureSourceSize(int32_t &nWidth,int32_t &nHeight);
    virtual WMERESULT GetCurrentCaptureSize(int32_t &nWidth,int32_t &nHeight);
    virtual WMERESULT SetCaptureMaxSize(const int32_t nWidth,const int32_t nHeight) { return WME_S_OK; }
    virtual WMERESULT Capture(IWmeMediaPackage *pPackage);
protected:
    void cleanup();
    void resetfileToDataPos();
protected:
    std::string m_strFilePath;
    std::fstream m_fsRawFile;
    RawFileHeader m_fileHeader;
    int m_nFileLen;
    int m_nSkipCapture;
    int m_nCurStep;
};


#endif // _SCREEN_TRACK_VIEW_ANALYZER_H_
