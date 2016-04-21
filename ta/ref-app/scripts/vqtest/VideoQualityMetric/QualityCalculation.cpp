#include "QualityCalculation.h"
#include "stdio.h"
#include <iostream>
#include <math.h>


double psnr_calculation(unsigned char *y1, unsigned char *u1, unsigned char *v1,
                        unsigned char *y2, unsigned char *u2, unsigned char *v2, int frame_size)
{
    double err_y, err_u, err_v;
    err_y = err_u = err_v = 0.0f;
    int i;
    for (i=0; i<frame_size; i++)
    {
        err_y += (y1[i]-y2[i])*(y1[i]-y2[i]);
    }
    err_y /= frame_size;
    for (i=0; i<frame_size/4; i++)
    {
        err_u += (u1[i]-u2[i])*(u1[i]-u2[i]);
        err_v += (v1[i]-v2[i])*(v1[i]-v2[i]);
    }
    err_u /= frame_size/4;
    err_v /= frame_size/4;
    double psnr_y = 10 * log10(255*255/err_y);
    double psnr_u = 10 * log10(255*255/err_u);
    double psnr_v = 10 * log10(255*255/err_v);
#ifdef DEBUG_LOG
    printf("%f,%f,%f\n",psnr_y,psnr_u,psnr_v);
#endif
    return 0.8 * psnr_y + 0.1 * psnr_u + 0.1 * psnr_v;
}

CQualityCalculation::CQualityCalculation(){
    m_pParserFrame = nullptr;
    m_pCaptureFile = nullptr;
    m_pSenderDataFile = nullptr;
    m_pSenderInfoFile = nullptr;
    m_pReceiverDataFile = nullptr;
    m_pReceiverInfoFile = nullptr;
    
    m_pRecYUV = nullptr;
    m_pSrcYUV = nullptr;
    m_pRecUpsamplingYUV[0] = nullptr;
    m_pRecUpsamplingYUV[1] = nullptr;
    m_pRecUpsamplingYUV[2] = nullptr;
}

CQualityCalculation::~CQualityCalculation(){
    
}


int CQualityCalculation::Init(char * pCaptureFile,WmeVideoRawFormat fmt,char *pSenderDataFile,char *pSenderInfoFile,char *pReceiverDataFile,char *pReceiverInfoFile){
    
    m_pCaptureFile = pCaptureFile;
    m_pSenderDataFile = pSenderDataFile;
    m_pSenderInfoFile = pSenderInfoFile;
    m_pReceiverDataFile = pReceiverDataFile;
    m_pReceiverInfoFile = pReceiverInfoFile;

    m_iCaptureWidth = fmt.iWidth;
    m_iCaptureHeight = fmt.iHeight;
    m_iCaptureFps = fmt.fFrameRate;
    m_fPSNR_S = 0;
    m_fPSNR_R = 0;
    
    m_pParserFrame = new CParserFrame();
    m_pParserFrame->Init();
    
    m_pDownConvert = new DownConvert;
    m_pDownConvert->init(   MAX_WIDTH, MAX_WIDTH );
    
    m_pRecYUV = (unsigned char *)malloc(MAX_WIDTH*MAX_HEIGHT*1.5);
    m_pSrcYUV = (unsigned char *)malloc(MAX_WIDTH*MAX_HEIGHT*1.5);
    m_pRecUpsamplingYUV[0] = (unsigned char *)malloc(MAX_WIDTH*MAX_HEIGHT);
    m_pRecUpsamplingYUV[1] = (unsigned char *)malloc(MAX_WIDTH*MAX_HEIGHT/4);
    m_pRecUpsamplingYUV[2] = (unsigned char *)malloc(MAX_WIDTH*MAX_HEIGHT/4);
    m_lStartTimeStamp = -1;
    return 0;
}
int CQualityCalculation::Uninit(){
    if(m_pParserFrame){
        m_pParserFrame->Uninit();
        delete m_pParserFrame;
        m_pParserFrame = nullptr;
    }
    if(m_pDownConvert){
        m_pDownConvert->destroy();
        delete m_pDownConvert;
        m_pDownConvert = nullptr;
    }
    if(m_pRecYUV){
        free(m_pRecYUV);
        m_pRecYUV = nullptr;
    }
    if(m_pSrcYUV){
        free(m_pSrcYUV);
        m_pSrcYUV = nullptr;
    }
    for(int i =0;i<3;i++){
      if(m_pRecUpsamplingYUV[i]){
        free(m_pRecUpsamplingYUV[i]);
        m_pRecUpsamplingYUV[i] = nullptr;
      }
    }
    
    return 0;
}

int CQualityCalculation::ReadSourceInfo(SSourceFrameInfo *pInfo,FILE *pInfoFile){
    fread(&pInfo->ulIndex, sizeof(pInfo->ulIndex), 1,pInfoFile);
    fread(&pInfo->ulNow, sizeof(pInfo->ulNow), 1, pInfoFile);
    fread(&pInfo->ulSize, sizeof(pInfo->ulSize), 1, pInfoFile);
    fread(&pInfo->codecType, sizeof(pInfo->codecType), 1, pInfoFile);
    fread(&pInfo->ulTimestamp, sizeof(pInfo->ulTimestamp), 1,pInfoFile);
    fread(&pInfo->layer_num, sizeof(pInfo->layer_num), 1,pInfoFile);
    fread(&pInfo->layer_index, sizeof(pInfo->layer_index), 1,pInfoFile);
    fread(&pInfo->layer_width, sizeof(pInfo->layer_width), 1,pInfoFile);
    fread(&pInfo->layer_height, sizeof(pInfo->layer_height), 1, pInfoFile);
    fread(&pInfo->nal_ref_idc, sizeof(pInfo->nal_ref_idc), 1,pInfoFile);
    fread(&pInfo->did, sizeof(pInfo->did), 1,pInfoFile);
    fread(&pInfo->tid, sizeof(pInfo->tid), 1,pInfoFile);
    fread(&pInfo->max_tid, sizeof(pInfo->max_tid), 1,pInfoFile);
    fread(&pInfo->frameType, sizeof(pInfo->frameType), 1,pInfoFile);
    fread(&pInfo->frameIdc, sizeof(pInfo->frameIdc), 1,pInfoFile);
    fread(&pInfo->priority, sizeof(pInfo->priority), 1,pInfoFile);
    fread(&pInfo->max_priority, sizeof(pInfo->max_priority), 1,pInfoFile);
   // printf("timestmap = %lld,frametype = %d,size = %d\n",pInfo->ulTimestamp,pInfo->frameType,pInfo->ulSize);
    return 0;
}

void CQualityCalculation::OutputResult(int bitrate){
    FILE *fResult = nullptr;
    fResult = fopen("vqtest_result.csv", "a");
    if (!fResult) {
        return;
    }

    fprintf(fResult, "%d,%f,%f,\n", bitrate,m_fPSNR_S,m_fPSNR_R);
    fclose(fResult);
}

float CQualityCalculation::SenderMatric(){
    float psnr = 0.0;
    FILE * fCapture = nullptr;
    FILE * fsrc = nullptr;
    FILE * fsrcInfo = nullptr;
    int iSrcWidth = m_iCaptureWidth;
    int iSrcHeight = m_iCaptureHeight;
    float fFramerate = m_iCaptureFps = 30;
    int iSrcYSize = iSrcWidth*iSrcHeight;
    
    int iSrcFrameSize = iSrcWidth *iSrcHeight*1.5;
    
    int iCapFrameCount = 0;
    int iSrcFrameCount = 0;
    bool bCanBeRead = false;
    unsigned long long iSrcTimeStamp = 0;
    unsigned long long iRecTimeSTamp = 0;
    SSourceFrameInfo sInfo;
    SDecoderFrameInfo sDecoderFrameInfo;
    bool bStartFlag = false;
    bool bEndFlag = false;
    unsigned char *pDumpData = (unsigned char *)malloc(iSrcFrameSize);
    if(!pDumpData){
        goto ERR_RET;
    }
    
    fCapture = fopen(m_pCaptureFile,"rb");
    if(!fCapture){
        goto ERR_RET;
    }
    
    fsrc = fopen(m_pSenderDataFile,"rb");
    fsrcInfo = fopen(m_pSenderInfoFile,"rb");
    if(!fsrc || !fsrcInfo){
        goto ERR_RET;
    }
    
    ReadSourceInfo(&sInfo,fsrcInfo);
    iSrcTimeStamp = 0;
    do{
        bCanBeRead = (fread (m_pSrcYUV, 1, iSrcFrameSize, fCapture) == iSrcFrameSize);
        if (!bCanBeRead){
            fseek(fCapture,0,SEEK_SET);
            bCanBeRead = (fread (m_pSrcYUV, 1, iSrcFrameSize, fCapture) == iSrcFrameSize);
            if(!bCanBeRead)
                break;
        }
        
        if(iSrcTimeStamp == sInfo.ulTimestamp){ //find the first frame
            bStartFlag = true;
        }
        
        if(bStartFlag){
            if((sInfo.ulTimestamp<=(iSrcTimeStamp+2))&&(sInfo.ulTimestamp>=(iSrcTimeStamp-2)))
            {
                int iDecRet = 0;
                int iRecFrameSize = 0;
                int iReadLen = 0;
                do{
                    iRecFrameSize = sInfo.layer_width*sInfo.layer_height*1.5;
                    iReadLen = fread (pDumpData, 1, sInfo.ulSize, fsrc);
                    iDecRet = m_pParserFrame->ParserOneFrame(pDumpData,sInfo.ulSize,&sDecoderFrameInfo);
                    ReadSourceInfo(&sInfo,fsrcInfo);
                    
                }while(iDecRet);
                
                if(iRecFrameSize != iSrcFrameSize){
                    ResizeParameters rParameters;
                    rParameters.m_iRefLayerFrmWidth = sInfo.layer_width; //src width
                    rParameters.m_iRefLayerFrmHeight = sInfo.layer_height;
                    rParameters.m_iFrameWidth = iSrcWidth;
                    rParameters.m_iFrameHeight = iSrcHeight;
                    rParameters.m_iScaledRefFrmWidth = iSrcWidth;
                    rParameters.m_iScaledRefFrmHeight = iSrcHeight;
                    m_pDownConvert->upsampling6tapBilin(sDecoderFrameInfo.pData[0],sDecoderFrameInfo.pData[1],sDecoderFrameInfo.pData[2], sDecoderFrameInfo.stride,sDecoderFrameInfo.stride/2,
                                                                  m_pRecUpsamplingYUV[0],m_pRecUpsamplingYUV[1],m_pRecUpsamplingYUV[2], iSrcWidth,iSrcWidth/2,&rParameters);
                    
                }else{
                    for(int h = 0;h<sInfo.layer_height;h++)
                        memcpy(m_pRecUpsamplingYUV[0]+iSrcWidth*h,sDecoderFrameInfo.pData[0]+h*sDecoderFrameInfo.stride,sInfo.layer_width);
                    for(int h = 0;h<sInfo.layer_height/2;h++)
                        memcpy(m_pRecUpsamplingYUV[1]+iSrcWidth/2*h,sDecoderFrameInfo.pData[1]+h*(sDecoderFrameInfo.stride/2),sInfo.layer_width/2);
                    for(int h = 0;h<sInfo.layer_height/2;h++)
                        memcpy(m_pRecUpsamplingYUV[2]+iSrcWidth/2*h,sDecoderFrameInfo.pData[2]+ h*(sDecoderFrameInfo.stride/2),sInfo.layer_width/2);
                    
                }
            }
           
            if(iSrcTimeStamp>=m_lStartTimeStamp){
               psnr += psnr_calculation(m_pRecUpsamplingYUV[0],m_pRecUpsamplingYUV[1],m_pRecUpsamplingYUV[2],
                                         m_pSrcYUV,m_pSrcYUV+ iSrcYSize,m_pSrcYUV+iSrcYSize*5/4,iSrcYSize);
            
               iSrcFrameCount++;
            }
            if (feof(fsrc)||feof(fsrcInfo)||(iSrcTimeStamp>sInfo.ulTimestamp+100)){
                break;
            }
        }
        
        iCapFrameCount++;
        iSrcTimeStamp =(1000*iCapFrameCount/fFramerate);
        //printf("ts_in:%ld,ts_out:%ld,width = %d,height = %d\n",iSrcTimeStamp,sInfo.ulTimestamp,sInfo.layer_width,sInfo.layer_height);
        
    }while(bCanBeRead==true);

    m_fPSNR_S = psnr = psnr/iSrcFrameCount;
    printf("iSrcFrameCount = %d,psnr = %f\n",iSrcFrameCount,psnr);
ERR_RET:
    if(!fCapture){
        fclose(fCapture);
        fCapture = nullptr;
    }
    
    if(fsrc){
        fclose(fsrc);
        fsrc =nullptr;
    }
    
    if(fsrcInfo){
        fclose(fsrcInfo);
        fsrcInfo =nullptr;
    }
    if(pDumpData){
        free(pDumpData);
        pDumpData = nullptr;
    }
    return psnr;
}

int CQualityCalculation::ReadReceiverInfo(SFrameHeader *pHeader,FILE *pInfofile){

    fread(&pHeader->iHeaderSize, 1, sizeof(unsigned int), pInfofile);
    fread(&pHeader->eMType, 1, sizeof(WmeMediaFormatType), pInfofile);
    fread(&pHeader->iFrameSize, 1, sizeof(unsigned int), pInfofile);
    fread(&pHeader->sVideoRawFormat, 1, sizeof(WmeVideoRawFormat), pInfofile);

    return 0;
}

float CQualityCalculation::ReceiverMatrix(){
    float psnr = 0.0;
    SFrameHeader header;
    WmeVideoRawType dst_type = WmeVideoUnknown;
    
    FILE * fCapture = nullptr;
    FILE * fdst = nullptr;
    
    int iSrcWidth = m_iCaptureWidth;
    int iSrcHeight = m_iCaptureHeight;
    float fFramerate = m_iCaptureFps;
    int iSrcYSize = iSrcWidth*iSrcHeight;
    
    int iSrcFrameSize = iSrcWidth *iSrcHeight*1.5;
    int frame_num_in,frame_num_out,frame_num_reduced;
    frame_num_in = frame_num_out = frame_num_reduced = 0;
    unsigned long long ts_in, ts_out;
    double psnr_received_frame, psnr_lost_frame, psnr_average;
    psnr_received_frame = psnr_lost_frame = psnr_average = 0;
    bool bSendFlag = false;
    unsigned long long iSrcFileSize=0;
    
    header.pData[0] = (unsigned char *)malloc(iSrcYSize);
    header.pData[1] = (unsigned char *)malloc(iSrcYSize);
    header.pData[2] = (unsigned char *)malloc(iSrcYSize);
    
    if(!header.pData[0]||!header.pData[1]||!header.pData[2]){
        goto ERR_RET;
    }
    
    fCapture = fopen(m_pCaptureFile,"rb");
    if(!fCapture){
        goto ERR_RET;
    }
    fseek(fCapture, 0, SEEK_END);
    iSrcFileSize = ftell(fCapture);
    fseek(fCapture, 0, SEEK_SET);
    
    fdst = fopen(m_pReceiverDataFile,"rb");
    if(!fdst){
        goto ERR_RET;
    }
    
    
    while (1)
    {
        ReadReceiverInfo(&header,fdst);
        if (feof(fdst))
            break;
        
        ts_out = header.sVideoRawFormat.uTimestamp/90;
        if(m_lStartTimeStamp == -1)
            m_lStartTimeStamp = ts_out;
        
        fread(m_pSrcYUV, 1, iSrcFrameSize, fCapture);
        if (ftell(fCapture) == iSrcFileSize)
        {
            fseek(fCapture,0,SEEK_SET);
        }
        ts_in = 1000/fFramerate*frame_num_in;
        frame_num_in++;
        //printf("ts_in:%d,ts_out:%d,width = %d,height = %d\n",ts_in,ts_out,header.sVideoRawFormat.iWidth,header.sVideoRawFormat.iHeight);
        while (ts_in+1 < ts_out)
        {
            fread(m_pSrcYUV, 1, iSrcFrameSize, fCapture);
            if (ftell(fCapture) == iSrcFileSize)
            {
                fseek(fCapture,0,SEEK_SET);
            }
            ts_in = 1000/fFramerate*frame_num_in;
            frame_num_in++;
            
            if (bSendFlag)
            {
                //printf("ts_in:%d,ts_out:%d,width = %d,height = %d\n",ts_in,ts_out,header.sVideoRawFormat.iWidth,header.sVideoRawFormat.iHeight);
                psnr += psnr_calculation(m_pSrcYUV, m_pSrcYUV+iSrcYSize, m_pSrcYUV+iSrcYSize*5/4, m_pRecUpsamplingYUV[0],m_pRecUpsamplingYUV[1],m_pRecUpsamplingYUV[2], iSrcWidth *iSrcHeight);
            }
            else
                frame_num_reduced++;
        }

        int iDstFrameSize = header.sVideoRawFormat.iWidth * header.sVideoRawFormat.iHeight *1.5;
        int iDstYSize = header.sVideoRawFormat.iWidth*header.sVideoRawFormat.iHeight;
        if (iDstFrameSize == iSrcFrameSize)
        {
            fread(m_pRecUpsamplingYUV[0], 1, iDstYSize, fdst);
            fread(m_pRecUpsamplingYUV[1], 1, iDstYSize/4, fdst);
            fread(m_pRecUpsamplingYUV[2], 1, iDstYSize/4, fdst);
        }
        else
        {
            
            fread(m_pRecYUV, 1, iDstFrameSize, fdst);
            ResizeParameters rParameters;
            rParameters.m_iRefLayerFrmWidth = header.sVideoRawFormat.iWidth; //src width
            rParameters.m_iRefLayerFrmHeight =  header.sVideoRawFormat.iHeight;
            rParameters.m_iFrameWidth = iSrcWidth;
            rParameters.m_iFrameHeight = iSrcHeight;
            rParameters.m_iScaledRefFrmWidth = iSrcWidth;
            rParameters.m_iScaledRefFrmHeight = iSrcHeight;
            m_pDownConvert->upsampling6tapBilin(m_pRecYUV,m_pRecYUV+iDstYSize,m_pRecYUV+iDstYSize*5/4, header.sVideoRawFormat.iWidth,header.sVideoRawFormat.iWidth/2,m_pRecUpsamplingYUV[0],m_pRecUpsamplingYUV[1],m_pRecUpsamplingYUV[2], iSrcWidth,iSrcWidth/2,&rParameters);
        }
        frame_num_out++;
        bSendFlag = true;
        
        psnr += psnr_calculation(m_pSrcYUV, m_pSrcYUV+iSrcYSize, m_pSrcYUV+iSrcYSize*5/4, m_pRecUpsamplingYUV[0],m_pRecUpsamplingYUV[1],m_pRecUpsamplingYUV[2], iSrcWidth *iSrcHeight);
    }
    m_fPSNR_R = psnr = psnr/(frame_num_in-frame_num_reduced);
    printf("iDstFrameCount = %d, psnr = %f\n",frame_num_in-frame_num_reduced,psnr);
ERR_RET:
    
    if(header.pData[0]){
        free(header.pData[0]);
        header.pData[0] = nullptr;
    }
    if(header.pData[1]){
        free(header.pData[1]);
        header.pData[1] = nullptr;
    }
    if(header.pData[2]){
        free(header.pData[2]);
        header.pData[2] = nullptr;
    }
    
    if(!fCapture){
        fclose(fCapture);
        fCapture = nullptr;
    }
    
    if(fdst){
        fclose(fdst);
        fdst =nullptr;
    }
    return psnr;
}