#include "ParserFrame.h"
#include <CoreFoundation/CoreFoundation.h>
#include <dlfcn.h>
#include <string>



#define PATH_MAX 1024
#define MAX_MODULE_NAME_LEN 1024

const char szDecoderCreate[] = "WelsCreateDecoder";
const char szDecoderDestroy[] = "WelsDestroyDecoder";


using namespace::std;
int GetCurrentModulePath(char* lpModulePath, const int iPathMax)
{
    if(lpModulePath == NULL || iPathMax <= 0)
    {
        return -1;
    }
    
    memset(lpModulePath, 0, iPathMax);
    
    char cCurrentPath[PATH_MAX];
    memset(cCurrentPath, 0, PATH_MAX);
    
    Dl_info 	dlInfo;
    static int  sDummy;
    dladdr((void*)&sDummy, &dlInfo);
    
    strlcpy(cCurrentPath, dlInfo.dli_fname, PATH_MAX);
    
    // whether is self a framework ?
    int locateNumber = 1;
    
    std::string strPath(cCurrentPath);
    int pos = std::string::npos;
    for(int i = 0; i < locateNumber; i++)
    {
        pos = strPath.rfind('/');
        if(std::string::npos == pos)
        {
            break;
        }
        strPath.erase(pos);
    }
    if(std::string::npos == pos)
    {
        return -2;
    }
    cCurrentPath[pos] = 0;
    
    strlcpy(lpModulePath, cCurrentPath, iPathMax);
    strlcat(lpModulePath, "/", iPathMax);
    
    return 0;
}

int    GenerateModulePathName(char strPathName[], int nMaxSize = MAX_MODULE_NAME_LEN)
{
    GetCurrentModulePath(strPathName, nMaxSize);
    strlcat(strPathName, "libopenh264.dylib", nMaxSize);
    return 0;
}

CParserFrame::~CParserFrame(){
    
}

CParserFrame::CParserFrame(){
    
}

int CParserFrame::Init(){

    char  strPathName[MAX_MODULE_NAME_LEN] = { 0 };
    GenerateModulePathName(strPathName, MAX_MODULE_NAME_LEN);
    //m_hModule = LoadBundle(strPathName);
    m_hModule = dlopen((char*)strPathName, RTLD_LAZY);
    if( NULL == m_hModule ){
        printf("open =%s, fail, error = %s",strPathName,dlerror());
        return -1;
    }
    spfuncCreate = (CreateH264Decoder)dlsym(m_hModule, (char*)szDecoderCreate);
    spfuncDestroy = (DestroyH264Decoder)dlsym(m_hModule, (char*)szDecoderDestroy);

    if(!spfuncCreate || !spfuncDestroy)
        return -1;
    
    
    long ret =	0;
    if (spfuncCreate)
    {
        ret = spfuncCreate(&m_pDecoder/* , this*/);
        if(ret!=0)
            return -1;
        if(!m_pDecoder)
            return -1;
    }

    SDecodingParam dec_param;
    memset(&dec_param, 0, sizeof(SDecodingParam));
    dec_param.uiTargetDqLayer = 255;
    dec_param.uiCpuLoad = 0;
    dec_param.eEcActiveIdc = ERROR_CON_SLICE_MV_COPY_CROSS_IDR_FREEZE_RES_CHANGE;
    dec_param.sVideoProperty.size = sizeof(SVideoProperty);

    dec_param.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
    ret = m_pDecoder->Initialize(&dec_param);

    
    return 0;
}

int CParserFrame::Uninit(){
    int ret = 0;
    if (m_pDecoder)
    {
        ret = m_pDecoder->Uninitialize();
        if(!ret)
            return -1;
        if (NULL != spfuncDestroy)
            spfuncDestroy(m_pDecoder);
        m_pDecoder = NULL;
    }
    dlclose(m_hModule);
    return 0;
}


int CParserFrame::ParserOneFrame(unsigned char *pInput,int size,SDecoderFrameInfo *pFrameInfo){
   unsigned char *pDstData[3] = {nullptr};
   SBufferInfo sDstInfo;
   memset(&sDstInfo,0,sizeof(SBufferInfo));

   m_pDecoder->DecodeFrameNoDelay(pInput,size, pDstData, &sDstInfo);
  if(sDstInfo.iBufferStatus == 1)//there is one frame output
  {
     pFrameInfo->width = sDstInfo.UsrData.sSystemBuffer.iWidth;
     pFrameInfo->height = sDstInfo.UsrData.sSystemBuffer.iHeight;
     pFrameInfo->stride = sDstInfo.UsrData.sSystemBuffer.iStride[0];
     pFrameInfo->pData[0] = pDstData[0];
     pFrameInfo->pData[1] = pDstData[1];
     pFrameInfo->pData[2] = pDstData[2];
     return 0;
  }
  else{
    return 1;
  }
}
