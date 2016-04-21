#pragma once

#include "ParserFrame.h"
#include "DownConvert.h"
#include "stdio.h"

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080

/// Enum of media format
typedef enum
{
    WmeMediaFormatAudioRaw,				///< Audio raw type, struct WmeAudioRawFormat
    WmeMediaFormatAudioCodec,			///< Audio codec type, struct WmeAudioCodecFormat
    WmeMediaFormatVideoRaw,				///< Video raw type, struct WmeVideoRawFormat
    WmeMediaFormatVideoCodec,			///< Video codec type, struct WmeVideoCodecFormat
    WmeMediaFormatTextRaw,				///< Text raw type
    WmeMediaFormatTextCodec,			///< Text codec type
    WmeMediaFormatScreenRaw,			///< Screen raw type, struct WmeScreenRawFormat
    WmeMediaFormatScreenRC,				///< Screen remote control type, struct WmeScreenRemoteControlFormat
    WmeMediaFormatScreenAnnotation,		///< Screen annotation type, struct WmeScreenAnnotationFormat
    WmeMediaFormatRTP,					///< RTP type
    WmeMediaFormatRTCP,					///< RTCP type
}WmeMediaFormatType;

/// Enum of video raw format type
typedef enum
{
    WmeVideoUnknown   = 0,
    WmeI420,
    WmeYV12,
    WmeNV12,
    WmeNV21,
    WmeYUY2,
    WmeRGB24,
    WmeBGR24,
    WmeRGB24Flip,
    WmeBGR24Flip,
    WmeRGBA32,
    WmeBGRA32,
    WmeARGB32,
    WmeABGR32,
    WmeRGBA32Flip,
    WmeBGRA32Flip,
    WmeARGB32Flip,
    WmeABGR32Flip,
}WmeVideoRawType;

/// Struct of video raw format
typedef struct _tagVideoRawFormat
{
    WmeVideoRawType		eRawType;		///< Identify the video raw format type
    int				iWidth;			///< Frame width
    int				iHeight;		///< Frame height
    float			fFrameRate;		///< Frame rate
    unsigned int	uTimestamp;		///< Frame timestamp
}WmeVideoRawFormat;

typedef struct _tagFrameHeader {
    unsigned int        iHeaderSize;
    WmeMediaFormatType  eMType;
    unsigned int        iFrameSize;
    WmeVideoRawFormat	sVideoRawFormat;
    unsigned char *pData[3];
}SFrameHeader;  //receiver side fileinfo

typedef unsigned long long element;

typedef struct _tagSourceFrameInfo{
    element ulIndex=0;
    element ulNow;
    element ulSize;
    element codecType;
    element ulTimestamp,prev_timestamp;
    int layer_num;
    int layer_index;
    int layer_width;
    int layer_height;
    int nal_ref_idc;
    int did;
    int tid;
    int max_tid;
    int frameType;
    int frameIdc;
    int priority;
    int max_priority;
}SSourceFrameInfo;

class CQualityCalculation{
public:
    CQualityCalculation();
    ~CQualityCalculation();

    int Init(char * pCaptureFile,WmeVideoRawFormat fmt,char *pSenderDataFile,char *pSenderInfoFile,char *pReceiverDataFile,char *pReceiverInfoFile);
    int Uninit();
    float SenderMatric();
    float ReceiverMatrix();
    int ReadSourceInfo(SSourceFrameInfo *pInfo,FILE *pInfoFile);
    int ReadReceiverInfo(SFrameHeader *pHeader,FILE *pInfofile);
    void OutputResult(int bitrate);
    SSourceFrameInfo sSourceFrameInfo;
    char *m_pCaptureFile;
    char *m_pSenderDataFile;
    char *m_pSenderInfoFile;
    char *m_pReceiverDataFile;
    char *m_pReceiverInfoFile;
    
    int m_iCaptureWidth;
    int m_iCaptureHeight;
    float m_iCaptureFps;
    float m_fPSNR_R;
    float m_fPSNR_S;
    CParserFrame *m_pParserFrame;
    DownConvert *m_pDownConvert;
    long long m_lStartTimeStamp;
    unsigned char * m_pRecYUV;
    unsigned char * m_pSrcYUV;
    unsigned char * m_pRecUpsamplingYUV[3];
};