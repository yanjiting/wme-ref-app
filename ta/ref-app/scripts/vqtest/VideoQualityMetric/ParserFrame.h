
#pragma once

#include "codec_api.h"
#include "codec_def.h"
#define  NULL nullptr

typedef long (*CreateH264Decoder)(ISVCDecoder** ppDecoder);
typedef void (*DestroyH264Decoder)(ISVCDecoder* pDecoder);

typedef struct _tagDecodeFrameInfo{
    int width;
    int height;
    int stride;
    unsigned char *pData[3];
}SDecoderFrameInfo;
class CParserFrame{
    
public:
    ~CParserFrame();
    CParserFrame();
    
    int Init();
    int ParserOneFrame(unsigned char *pInput,int size,SDecoderFrameInfo *pFrameInfo);
    int Uninit();
    
    void * m_hModule;
    
    CreateH264Decoder spfuncCreate;
    DestroyH264Decoder spfuncDestroy;
    ISVCDecoder* m_pDecoder;
};