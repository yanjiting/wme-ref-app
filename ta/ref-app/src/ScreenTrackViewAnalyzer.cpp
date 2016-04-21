#include "ScreenTrackViewAnalyzer.h"
#include "CMDEBUG.H"
#include <stdio.h>
#include <iosfwd>
#include <sstream>
#include "json/json.h"
#include "TALuminanceSource.h"

#if defined(WIN32) && !defined(WP8)
#include <gdiplus.h>
using namespace Gdiplus;

GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;
#endif


unsigned long GetCurTickCount()
{
#if defined(WIN32)
#ifdef WP8
	return GetTickCount64();
#else
	return GetTickCount();
#endif
#else
    struct timeval cur_time;
    gettimeofday(&cur_time, NULL);
    return cur_time.tv_sec *1000 + cur_time.tv_usec/1000;
#endif
}

#define AS_4BYTES_ALIGN(bitswidth)     (((bitswidth)+31)/32*4)
#if defined(MACOSX)
//#define QRCODE_DECODE_RAWDATA 1
#endif

static long int crv_tab[256];
static long int cbu_tab[256];
static long int cgu_tab[256];
static long int cgv_tab[256];
static long int tab_76309[256];
static unsigned char clp[1024]; //for clip in CCIR601

//
//Initialize conversion table for YUV420 to RGB
//
void init_dither_tab()
{
	long int crv, cbu, cgu, cgv;
	int i, ind;
	crv = 104597; cbu = 132201; /* fra matrise i global.h */
	cgu = 25675; cgv = 53279;

	for (i = 0; i < 256; i++) {
		crv_tab[i] = (i - 128) * crv;
		cbu_tab[i] = (i - 128) * cbu;
		cgu_tab[i] = (i - 128) * cgu;
		cgv_tab[i] = (i - 128) * cgv;
		tab_76309[i] = 76309 * (i - 16);
	}

	for (i = 0; i<384; i++)
		clp[i] = 0;

	ind = 384;

	for (i = 0; i<256; i++)
		clp[ind++] = i;

	ind = 640;

	for (i = 0; i<384; i++)
		clp[ind++] = 255;

}
void ConvertYUV2RGB(unsigned char *src0, unsigned char *src1, unsigned char *src2, unsigned char *dst_ori,
	int width, int height)
{
	int y1, y2, u, v;
	unsigned char *py1, *py2;
	int i, j, c1, c2, c3, c4;
	unsigned char *d1, *d2;

	py1 = src0;
	py2 = py1 + width;
	d1 = dst_ori;
	d2 = d1 + 3 * width;

	for (j = 0; j < height; j += 2) {
		for (i = 0; i < width; i += 2) {
			u = *src1++;
			v = *src2++;
			c1 = crv_tab[v];
			c2 = cgu_tab[u];
			c3 = cgv_tab[v];
			c4 = cbu_tab[u];

			//up-left
			y1 = tab_76309[*py1++];
			*d1++ = clp[384 + ((y1 + c1) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
			*d1++ = clp[384 + ((y1 + c4) >> 16)];

			//down-left
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c1) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
			*d2++ = clp[384 + ((y2 + c4) >> 16)];

			//up-right
			y1 = tab_76309[*py1++];
			*d1++ = clp[384 + ((y1 + c1) >> 16)];
			*d1++ = clp[384 + ((y1 - c2 - c3) >> 16)];
			*d1++ = clp[384 + ((y1 + c4) >> 16)];

			//down-right
			y2 = tab_76309[*py2++];
			*d2++ = clp[384 + ((y2 + c1) >> 16)];
			*d2++ = clp[384 + ((y2 - c2 - c3) >> 16)];
			*d2++ = clp[384 + ((y2 + c4) >> 16)];

		}

		d1 += 3 * width;
		d2 += 3 * width;
		py1 += width;
		py2 += width;

	}
	// BGR --> RGB
	for (i = 0; i < width * height * 3; i += 3)
	{
		unsigned char tmp = dst_ori[i];
		dst_ori[i] = dst_ori[i + 2];
		dst_ori[i + 2] = tmp;
	}
}


//By refer to http://en.wikipedia.org/wiki/YUV
inline BYTE clip(int v)
{
    return (v<0?0:(v>255?255:v));
}

void ConvertYUY2RGBA32(unsigned char *srcYuy,uint32_t uSrcLen, unsigned char *disRGB,uint32_t nDestLen){
    if(srcYuy==NULL||disRGB==NULL||uSrcLen<=0||nDestLen<=0) return ;
    
    int nPoint=0;
    for(int i=0;i<uSrcLen;i+=4){
        
        int Y0  =   srcYuy[i+0];
        int U   =   srcYuy[i+1];
        int Y1  =   srcYuy[i+2];
        int V   =   srcYuy[i+3];
        
        //
        int C = Y0 - 16;
        int D = U - 128;
        int E = V - 128;
    
        disRGB[nPoint] = clip(( 298 * C           + 409 * E + 128) >> 8);
        disRGB[nPoint+1] = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8);
        disRGB[nPoint+2] = clip(( 298 * C + 516 * D           + 128) >> 8);
        disRGB[nPoint+3] = 255;
        nPoint+=4;
        if(nPoint>=nDestLen) break;
        //
        C = Y1 - 16;
        D = U - 128;
        E = V - 128;
        
        disRGB[nPoint] = clip(( 298 * C           + 409 * E + 128) >> 8);
        disRGB[nPoint+1] = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8);
        disRGB[nPoint+2] = clip(( 298 * C + 516 * D           + 128) >> 8);
        disRGB[nPoint+3] = 255;
        nPoint+=4;
        if(nPoint>=nDestLen) break;
    }
}

#if defined(WIN32) && !defined(WP8)
bool SaveQBBlockToBMP(unsigned char *pData, WmeVideoRawType dataType, int width, int height,
	const wchar_t *pathname, int bytesPerPix)
{
	FILE *fp = _wfopen(pathname, L"wb");
	if( fp )
	{
		BITMAPINFOHEADER bminfo = {0};
			
		bminfo.biSize = sizeof(bminfo);
		bminfo.biWidth = width;
		bminfo.biHeight = height;
		bminfo.biBitCount = 24;
		bminfo.biPlanes = 1;
		bminfo.biCompression = BI_RGB;
		bminfo.biXPelsPerMeter = 72;
		bminfo.biYPelsPerMeter = 72;
            
		int bytesPerRow = AS_4BYTES_ALIGN(width * bytesPerPix * 8);
            
		BITMAPFILEHEADER bmfh = {0};
		bmfh.bfType = 'MB';
		bmfh.bfOffBits = sizeof(bmfh)+sizeof(bminfo);
		bmfh.bfSize = bytesPerRow * height + bmfh.bfOffBits;
            
		fwrite( &bmfh, 1, sizeof(bmfh), fp );
		fwrite( &bminfo, 1, sizeof(bminfo), fp );
            
        unsigned char *pSrcLine = NULL;
		unsigned char *buf = new unsigned char[bytesPerRow];
		if( buf  )
		{
			unsigned char *dst, *src;
			for( int i = 0; i < height; i++ ) {
                    
                    
                pSrcLine = pData + i * bytesPerRow;
 
                memcpy(buf, pSrcLine, bytesPerRow);
                    
                    
				for( int j=0; j < width; j++ )
				{
					dst = buf+j*3;
					switch(dataType)
					{
					case WmeRGBA32:
					case WmeBGRA32:
					case WmeRGBA32Flip:
					case WmeBGRA32Flip:
						src = pSrcLine + j * 4;
                        dst[0] = src[0];
                        dst[1] = src[1];
                        dst[2] = src[2];
                        break;
					case WmeARGB32:
					case WmeABGR32:
					case WmeARGB32Flip:
					case WmeABGR32Flip:
						src = pSrcLine + j * 4;
                        dst[0] = src[1];
                        dst[1] = src[2];
                        dst[2] = src[3];
                        break;
					//case WmeRGB24:
					//case WmeBGR24:
					//case WmeRGB24Flip:
					//case WmeBGR24Flip:
					//	src = pSrcLine + j * 3;
     //                   dst[0] = src[0];
     //                   dst[1] = src[1];
     //                   dst[2] = src[2];
     //                   break;
					default:
						break;
					}
				}
				fwrite(buf, 1, bytesPerRow, fp );
			}
		}
		if (buf)
		{
			delete[] buf;
			buf = NULL;
		}

		fclose( fp );
		return true;
	}
	
	return false;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0;                     // number of image encoders   
	UINT size = 0;                   // size of the image encoder array in bytes   
	ImageCodecInfo* pImageCodecInfo = NULL;
	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;     //   Failure   

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;     //   Failure   

	GetImageEncoders(num, size, pImageCodecInfo);
	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;     //   Success   
		}
	}
	free(pImageCodecInfo);
	return -1;     //   Failure   
}

BOOL ConvertBmp2Jpg(const wchar_t* bmp_file, const wchar_t* jpg_file)
{
	CLSID             encoderClsid;
	EncoderParameters encoderParameters;
	ULONG             quality;
	Status            stat;

	// Get an image from the disk.
	Image* image = new Image(bmp_file);

	// Get the CLSID of the JPEG encoder.
	GetEncoderClsid(L"image/jpeg", &encoderClsid);

	// Before we call Image::Save, we must initialize an
	// EncoderParameters object. The EncoderParameters object
	// has an array of EncoderParameter objects. In this
	// case, there is only one EncoderParameter object in the array.
	// The one EncoderParameter object has an array of values.
	// In this case, there is only one value (of type ULONG)
	// in the array. We will let this value be 100.

	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = EncoderQuality;
	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;



	// Save the image as a JPEG with quality level 100.
	quality = 100;
	encoderParameters.Parameter[0].Value = &quality;
	stat = image->Save(jpg_file, &encoderClsid, &encoderParameters);


	delete image;

	return stat == Ok;
}
#endif


CScreenTrackViewAnalyzer::CScreenTrackViewAnalyzer()
{
    m_nQRColumns = 0;
    m_nQRRows = 0;

    
    m_nQRRgnLeft = 0;
    m_nQRRgnTop = 0;
	m_nQRRgnRight = 0;
    m_nQRRgnBottom = 0;


	init_dither_tab();
	m_nTmpBlockIndex = 0;
	m_bReplayTempFiles = false;

#if defined(WIN32) && !defined(WP8)
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#endif

    m_bDumpFrame2File = false;
    m_nPreTickCount = 0;
    m_bScanQRCode = true;
    m_nDump2FileIndex =1;
}

CScreenTrackViewAnalyzer::~CScreenTrackViewAnalyzer()
{
#if defined(WIN32) && !defined(WP8)
	GdiplusShutdown(gdiplusToken);
#endif
}

WMERESULT CScreenTrackViewAnalyzer::IsPushModeSupport(bool& isSupport)
{
    return WME_E_NOTIMPL;
}

WMERESULT CScreenTrackViewAnalyzer::RenderMediaData(IWmeMediaPackage *pPackage)
{
    return WME_E_NOTIMPL;
}

#ifndef WP8
WMERESULT SaveRawDataToJPEG( unsigned char* pData,int width, int height, int nLineBytes, WmeVideoRawType eWmeVideoRawType , const char *pathname,float compressionQuality);
WMERESULT SaveRawDataToPNG( unsigned char* pData,int width, int height, int nLineBytes, WmeVideoRawType eWmeVideoRawType , const char *pathname);
#else
WMERESULT SaveRawDataToJPEG(unsigned char* pData, int width, int height, int nLineBytes, WmeVideoRawType eWmeVideoRawType, const char *pathname, float compressionQuality){
	return WME_E_NOTIMPL;
}
WMERESULT SaveRawDataToPNG(unsigned char* pData, int width, int height, int nLineBytes, WmeVideoRawType eWmeVideoRawType, const char *pathname){
	return WME_E_NOTIMPL;
}

#endif
std::string CScreenTrackViewAnalyzer::DecodeQR(unsigned char *pData, WmeVideoRawType dataType,
                                               int width, int height,
                                               int bytesPerPix)
{
#if defined(WIN32) && !defined(WP8)
    std::string strQRCode;
    wchar_t szTmpPathName[MAX_PATH] = { 0 };
	wchar_t szTmpFileName[MAX_PATH] = { 0 };


	DWORD dwRetVal = GetTempPathW(MAX_PATH, szTmpPathName);
	if (dwRetVal > MAX_PATH || (dwRetVal == 0))
	{
		return "";
	}

	dwRetVal = GetTempFileNameW(szTmpPathName,
		L"QB",     
		0,                
		szTmpFileName);
	if (dwRetVal == 0)
	{
		return "";
	}
	std::string jpg_file;


	char szTmpPathNameA[MAX_PATH] = { 0 };

	GetTempPathA(MAX_PATH, szTmpPathNameA);


	jpg_file = szTmpPathNameA;
    std::ostringstream strFileName;
    if (szTmpPathNameA[strlen(szTmpPathNameA) - 1] == '\\')
        strFileName << "tmp_qrcode_"<< ++m_nTmpBlockIndex << ".jpg";
    else
        strFileName << "\\" <<"tmp_qrcode_" << ++m_nTmpBlockIndex << ".jpg";
	jpg_file += strFileName.str();
	wchar_t jpg_file_w[MAX_PATH] = { 0 };

	MultiByteToWideChar(CP_UTF8, 0, jpg_file.c_str(), jpg_file.length(), jpg_file_w, MAX_PATH);

	if (m_bReplayTempFiles
		|| (SaveQBBlockToBMP(pData, dataType, width, height,
			szTmpFileName, bytesPerPix) && ConvertBmp2Jpg(szTmpFileName, jpg_file_w)) )
    {
		strQRCode = m_zxing.GetBarCodeFromImg(jpg_file.c_str());
    }
    
	DeleteFileW(szTmpFileName);
	//DeleteFileA(jpg_file.c_str());

	return strQRCode;
#elif defined(MACOSX)
    std::string strQRCode = "";
    
    std::string jpg_file = "";
    std::ostringstream strFileName;
    strFileName <<"/tmp/tmp_qrcode_" <<++m_nTmpBlockIndex <<".jpg";
    jpg_file =strFileName.str();
    
    //
    if(m_bReplayTempFiles
       || SaveRawDataToJPEG(pData,width,height,bytesPerPix*width,dataType,jpg_file.c_str(),50) == WME_S_OK)
        strQRCode = m_zxing.GetBarCodeFromImg(jpg_file.c_str());
    return strQRCode;
#endif
        
    return "";
}


std::string CScreenTrackViewAnalyzer::GetOneQRBlockFromFrame(unsigned char *pData, WmeVideoRawType dataType,
                                                             int row, int col,
                                                             int block_w, int block_h, int bytesPerPix, int nFrameWidth)
{
    int nBlockLineSize = AS_4BYTES_ALIGN(block_w * bytesPerPix * 8);
    int nBlockSize = nBlockLineSize * block_h;
    
    
    unsigned char *p = pData + (m_nQRRgnTop + row * block_h)*nFrameWidth * bytesPerPix
                        + (m_nQRRgnLeft + col * block_w) * bytesPerPix;
    unsigned char *pBlock = NULL;
    
    try
    {
        pBlock = new unsigned char[nBlockSize];
    }
    catch (std::bad_alloc)
    {
        return "";
    }
    memset(pBlock, 0, nBlockSize);
    

    
    for (int i = 0; i < block_h; i++)
    {
        memcpy(pBlock + (block_h - 1 - i) * nBlockLineSize, p + nFrameWidth * bytesPerPix * i, block_w * bytesPerPix);
    }
    

    
    std::string strQR = DecodeQR(pBlock, dataType, block_w, block_h, bytesPerPix);
    
    CM_INFO_TRACE_THIS("CScreenTrackViewAnalyzer::GetOneQRBlockFromFrame row=" << row <<
                       " col=" << col << " QR=" << strQR);

    
    delete[] pBlock;
    pBlock = NULL;
    
    return strQR;
}

char GrayScalePixel(const char *pixData, int type)
{
    switch (type) {
            case WmeRGBA32:
            case WmeBGRA32:
            case WmeRGBA32Flip:
            case WmeBGRA32Flip:
            return ((306 * (pixData[0]) & 0xFF) +
             601 * ((pixData[1]) & 0xFF) +
             117 * (pixData[2] & 0xFF) +
             0x200) >> 10;
            break;
            case WmeARGB32:
            case WmeABGR32:
            case WmeARGB32Flip:
            case WmeABGR32Flip:
            return ((306 * (pixData[1]) & 0xFF) +
                    601 * ((pixData[2]) & 0xFF) +
                    117 * (pixData[3] & 0xFF) +
                    0x200) >> 10;
			break;
            case WmeI420:
		{
            return pixData[0];
			break;
		}
            case WmeYV12:
            case WmeNV12:
            case WmeNV21:
            case WmeYUY2:
            break;
            
        default:
            break;
    }
    return 0;
}

WMERESULT CScreenTrackViewAnalyzer::RenderMediaData(uint32_t uTimestamp, WmeMediaFormatType eType, void* pFormat, unsigned char *pData, uint32_t uLength)
{

    // get QR code and analyze QR code by ZXing
    CM_INFO_TRACE_THIS("CScreenTrackViewAnalyzer::RenderMediaData data_len=" << uLength);
    
    if (eType != WmeMediaFormatVideoRaw && eType != WmeMediaFormatVideoRawWithStride)
        return WME_S_FALSE;
    
    if (!pFormat || !pData || uLength == 0)
        return WME_S_FALSE;
    
	CCmMutexGuardT<CCmMutexThreadRecursive> theGuard(m_mutexQRBlocks);
    int bytePerPix = 4;
    
    WmeVideoRawFormat *pVideoRawFormat = (WmeVideoRawFormat*)pFormat;

    if (pVideoRawFormat->iWidth < (m_nQRRgnRight - m_nQRRgnLeft)
        || pVideoRawFormat->iHeight < (m_nQRRgnBottom - m_nQRRgnTop))
    {
        return WME_S_FALSE;
    }

	if (m_nQRRgnRight - m_nQRRgnLeft <= 0)
		m_nQRRgnRight = m_nQRRgnLeft + pVideoRawFormat->iWidth; 
	if (m_nQRRgnBottom - m_nQRRgnTop <= 0)
		m_nQRRgnBottom = m_nQRRgnTop +pVideoRawFormat->iHeight;

    m_vctQRBlocks.clear();

	unsigned char* pCvtData = NULL;
	unsigned char* pWinDataBuffer = NULL;

	WmeVideoRawType dataType = pVideoRawFormat->eRawType;

    switch (dataType)
    {
		case WmeVideoUnknown:
			return WME_S_FALSE;
		case WmeRGB24:
        case WmeBGR24:
        case WmeRGB24Flip:
        case WmeBGR24Flip:
        #if defined(MACOSX)
            try
            {
                pCvtData = new unsigned char[pVideoRawFormat->iWidth * pVideoRawFormat->iHeight * 4];
            }
            catch (std::bad_alloc)
            {
                return WME_S_FALSE;
            }
            if(pCvtData){
                //Convert to 32bit
                for(int i=0;i<pVideoRawFormat->iHeight;i++){
                    int nDestStartLine = i*pVideoRawFormat->iWidth*4;
                    int nSrcStartLine = i*pVideoRawFormat->iWidth*3;
                    for(int j=0;j<pVideoRawFormat->iWidth;j++){
                        int nDestStart = nDestStartLine + j*4;
                        int nSrcStart = nSrcStartLine + j*3;
                        
                        if(dataType == WmeRGB24 || dataType == WmeRGB24Flip){
                            pCvtData[nDestStart]= pData[nSrcStart];
                            pCvtData[nDestStart+1]= pData[nSrcStart+1];
                            pCvtData[nDestStart+2]= pData[nSrcStart+2];
                        }
                        else{
                            pCvtData[nDestStart]= pData[nSrcStart+2];
                            pCvtData[nDestStart+1]= pData[nSrcStart+1];
                            pCvtData[nDestStart+2]= pData[nSrcStart];
                        }
                        pCvtData[nDestStart+3] = 255;
                    }
                }
                
                dataType = WmeRGBA32;
                bytePerPix = 4;
            }
        #else
            bytePerPix = 3;
        #endif
			break;
		case WmeI420:
		{
#if defined(MACOSX) || defined(WIN32)
#if defined(QRCODE_DECODE_RAWDATA)
			bytePerPix = 1;
#else
            try
            {
                pCvtData = new unsigned char[pVideoRawFormat->iWidth * pVideoRawFormat->iHeight * 3];
            }
            catch (std::bad_alloc)
            {
                return WME_S_FALSE;
            }
            
            ConvertYUV2RGB(pData, pData + pVideoRawFormat->iWidth * pVideoRawFormat->iHeight,
                           pData + pVideoRawFormat->iWidth * pVideoRawFormat->iHeight * 5 / 4, pCvtData,
                           pVideoRawFormat->iWidth, pVideoRawFormat->iHeight);
            
            
            dataType = WmeRGB24;
            bytePerPix = 3;
#endif //QRCODE_DECODE_RAWDATA
#endif //MacOSX || WIN32
			break;
		}
        case WmeYUY2:
        {
            int nBufLen =pVideoRawFormat->iWidth * pVideoRawFormat->iHeight * 4;
            try
            {
                pCvtData = new unsigned char[nBufLen];
            }
            catch (std::bad_alloc)
            {
                return WME_S_FALSE;
            }
            
            ConvertYUY2RGBA32(pData, uLength, pCvtData,nBufLen);
            
            
            dataType = WmeRGBA32;
            bytePerPix = 4;
        }
            break;
		case WmeYV12:
		case WmeNV12:
		case WmeNV21:
            return WME_E_NOTIMPL;
        default:
            break;
    }
    if(m_bDumpFrame2File){
        
#if defined(MACOSX) || defined(WIN32)
        
        unsigned long nNow = GetCurTickCount();
        if((nNow-m_nPreTickCount)>1000){//dump one image every 1s
            std::string jpg_file = "";
            std::ostringstream strFileName;
#if defined(WIN32)
            strFileName <<"c:/tmp/screen_share/img-dump-decoded/" <<m_nDump2FileIndex<<".bmp";
#else
            strFileName <<"/tmp/screen_share/img-dump-decoded/" <<m_nDump2FileIndex<<".png";
#endif
            m_nDump2FileIndex++;
            jpg_file =strFileName.str();
        
            int nWidth = (m_nQRRgnRight - m_nQRRgnLeft);
            int nHeight = (m_nQRRgnBottom - m_nQRRgnTop);
            
#if defined(WIN32) && !defined(WP8)
			//
			unsigned char* pShareFrameData = pCvtData ? pCvtData : pData;
			pWinDataBuffer = new unsigned char[bytePerPix*nWidth*nHeight];
			if (pShareFrameData){
				for (int i = 0; i < nHeight; i++){
					int nFrameLineBytes = bytePerPix*nWidth;
					memcpy(pWinDataBuffer + i*nFrameLineBytes,
						pShareFrameData + (nHeight - 1 - i)*nFrameLineBytes,
						nFrameLineBytes);
				}
			}
			if (pWinDataBuffer){
				wchar_t jpg_file_w[MAX_PATH] = { 0 };
				MultiByteToWideChar(CP_UTF8, 0, jpg_file.c_str(), jpg_file.length(), jpg_file_w, MAX_PATH);
				SaveQBBlockToBMP(pWinDataBuffer, dataType, nWidth, nHeight, jpg_file_w, bytePerPix);
			}
#else
            SaveRawDataToPNG(pCvtData ? pCvtData : pData,nWidth,nHeight,bytePerPix*nWidth,dataType,jpg_file.c_str());
#endif
            
        }
        m_nPreTickCount = nNow;
        
#endif
    }
    if(m_bScanQRCode){
        
        if (m_nQRColumns == 0 || m_nQRRows == 0 )
            return WME_S_FALSE; //no init yet
        
#if defined(MACOSX) || defined(WIN32)
#if defined(QRCODE_DECODE_RAWDATA)
    DecodeQRCodeInFrame(m_vctQRBlocks, pCvtData ? pCvtData : pData, pVideoRawFormat->iWidth, pVideoRawFormat->iHeight, dataType, m_nQRRows, m_nQRColumns, bytePerPix);
    return WME_S_OK;
#else
    int block_w = (m_nQRRgnRight - m_nQRRgnLeft) / m_nQRColumns;
    int block_h = (m_nQRRgnBottom - m_nQRRgnTop)  / m_nQRRows;

	m_nTmpBlockIndex = 0; 

	for (int row = 0; row < m_nQRRows; row++)
    {
        for (int col = 0; col < m_nQRColumns; col++)
		{
			std::string strOneQRCode = GetOneQRBlockFromFrame(pCvtData ? pCvtData : pData, dataType, row, col, block_w, block_h,
				bytePerPix, pVideoRawFormat->iWidth);			
				m_vctQRBlocks.push_back(strOneQRCode);
		}
    }
    
	if (pCvtData)
	{
		delete[] pCvtData;
		pCvtData = NULL;
	}
    
	if (pWinDataBuffer){
		delete[]pWinDataBuffer;
		pWinDataBuffer = NULL;
	}
    
    return WME_S_OK;
#endif
#else
    return WME_E_NOTIMPL;
#endif
    }
    return WME_S_OK;
}

WMERESULT CScreenTrackViewAnalyzer::IsPullModeSupport(bool& isSupport)
{
    return WME_E_NOTIMPL;
}

WMERESULT CScreenTrackViewAnalyzer::RegisterRequestAnswerer(IWmeExternalRenderAnswerer* pAnswerer)
{
    return WME_E_NOTIMPL;
}

WMERESULT CScreenTrackViewAnalyzer::UnregisterRequestAnswerer()
{
    return WME_E_NOTIMPL;
}

void CScreenTrackViewAnalyzer::SetQRMatrix(int nRowCount, int nColCount)
{
    m_nQRRows = nRowCount;
    m_nQRColumns = nColCount;
}


void CScreenTrackViewAnalyzer::SetQRRgn(int nQRRgnLeft, int nQRRgnTop, int nQRRgnRight, int nQRRgnBottom)
{
    m_nQRRgnLeft = nQRRgnLeft;
    m_nQRRgnTop = nQRRgnTop;
    m_nQRRgnRight = nQRRgnRight;
    m_nQRRgnBottom = nQRRgnBottom;
}

std::vector<std::string> CScreenTrackViewAnalyzer::GetQRList()
{
	CCmMutexGuardT<CCmMutexThreadRecursive> theGuard(m_mutexQRBlocks);
    return m_vctQRBlocks;
}

void CScreenTrackViewAnalyzer::CleanQRList()
{
	m_vctQRBlocks.clear();
}
void CScreenTrackViewAnalyzer::SetQRCodeContextWithJson(std::string arg){
	//pass input arg
	Json::Reader r;
	Json::Value input;
	r.parse(arg, input, true);

	int x = input["view_x"].asInt();
	int y = input["view_y"].asInt();
	int width = input["view_w"].asInt();
	int height = input["view_h"].asInt();
	int row = input["qrcode_grid_row"].asInt();
	int col = input["qrcode_grid_col"].asInt();

	SetQRRgn(x, y, x + width, y + height);
	SetQRMatrix(row, col);
}

CScreenFileCapturer::CScreenFileCapturer(std::string strFilePath){
    cleanup();
    SetFilePath(strFilePath);
}
CScreenFileCapturer::~CScreenFileCapturer(){
    cleanup();
}
void CScreenFileCapturer::cleanup(){
    m_strFilePath = "";
    if(m_fsRawFile.is_open())
        m_fsRawFile.close();
    memset(&m_fileHeader,0,sizeof(m_fileHeader));
    m_nFileLen = 0;
    SetCaptureSkipCount(0);
}
void CScreenFileCapturer::SetFilePath(std::string strFilePath){
    if(m_strFilePath == strFilePath) return ;
    cleanup();

    m_strFilePath = strFilePath;
    m_fsRawFile.open(m_strFilePath.c_str(),std::fstream::in | std::fstream::binary);
    if(m_fsRawFile.is_open()){
        // get length of file:
        m_fsRawFile.seekg (0, m_fsRawFile.end);
        m_nFileLen = m_fsRawFile.tellg();
        m_fsRawFile.seekg (0, m_fsRawFile.beg);
        
        //get header information
        m_fsRawFile.read((char*)&m_fileHeader,sizeof(m_fileHeader));
        resetfileToDataPos();
    }else{
        cleanup();
    }
}
void CScreenFileCapturer::SetCaptureSkipCount(int nSkipCount)
{
    m_nSkipCapture  = nSkipCount;
    m_nCurStep      = m_nSkipCapture;
}
void CScreenFileCapturer::resetfileToDataPos(){
    if(m_fsRawFile.is_open()){
        m_fsRawFile.seekg(sizeof(m_fileHeader));
    }
}
WMERESULT CScreenFileCapturer::GetCurrentCaptureSourceSize(int32_t &nWidth,int32_t &nHeight){
    if(m_fileHeader.nFrameCount ==0
       || m_fileHeader.nHieght == 0
       || m_fileHeader.nWidth ==0){
        return  WME_E_FAIL;
    }
    nWidth = m_fileHeader.nWidth;
    nHeight = m_fileHeader.nHieght;
    return WME_S_OK;
}
WMERESULT CScreenFileCapturer::GetCurrentCaptureSize(int32_t &nWidth,int32_t &nHeight){
    return GetCurrentCaptureSourceSize(nWidth,nHeight);
}

WMERESULT CScreenFileCapturer::Capture(IWmeMediaPackage *pPackage){
    if(pPackage==NULL) return WME_E_POINTER;
    
    if(m_nSkipCapture>0){
        if(m_nCurStep!=m_nSkipCapture){
            m_nCurStep++;
            return WME_E_FAIL;
        }
        m_nCurStep = 0;
    }
    
    uint32_t uLength = 0,uSize =0;
    int nBytePerPix = 4;

    WmeScreenRawFormat * pWmeScreenRawFormat = NULL;
    unsigned char* pFormatBuffer = NULL;
    pPackage->GetFormatPointer(&pFormatBuffer);
    if(pFormatBuffer==NULL)
        return WME_E_FAIL;
    pWmeScreenRawFormat = (WmeScreenRawFormat *)pFormatBuffer;
    pWmeScreenRawFormat->eRawType = WmeRGBA32;
    if(m_fsRawFile.is_open()){
        pWmeScreenRawFormat->iWidth = m_fileHeader.nWidth;
        pWmeScreenRawFormat->iHeight = m_fileHeader.nHieght;
    }else{
        pWmeScreenRawFormat->iWidth = 1920;
        pWmeScreenRawFormat->iHeight = 1080;
    }
    
    uLength = pWmeScreenRawFormat->iWidth*pWmeScreenRawFormat->iHeight*nBytePerPix;
    unsigned char* pDataBuffer = NULL;
    pPackage->GetDataPointer(&pDataBuffer);
    pPackage->GetDataSize(uSize);
    if(pDataBuffer==NULL || uSize<uLength)//IWmeMediaPackage should support expand
        return WME_E_FAIL;

    if(m_fsRawFile.is_open()){
        int nCurPos = m_fsRawFile.tellg();
        if((nCurPos+uLength)>m_nFileLen)
            resetfileToDataPos();
#if defined(WIN32)
        int bytesPerLine = pWmeScreenRawFormat->iWidth*4;
        int nread=0;
        unsigned char *pLine;
        for (int i=0; i<pWmeScreenRawFormat->iHeight; i++) {
            pLine = pDataBuffer + bytesPerLine*(pWmeScreenRawFormat->iHeight-i-1);
            m_fsRawFile.read((char *)pLine, bytesPerLine);
            if ( m_fsRawFile.eof() )
                break;
        }
#else
        m_fsRawFile.read((char*)pDataBuffer,uLength);
#endif
        CM_DEBUG_TRACE_THIS("CScreenFileCapturer::Capture cur pos=" << m_fsRawFile.tellg());
    }
    else{
        memset(pDataBuffer,0x88888888,uLength/2);
        memset(pDataBuffer+uLength/2,0xbbbbbbbb,uLength/2);
    }
    pPackage->SetDataLength(uLength);

    return WME_S_OK;
}

