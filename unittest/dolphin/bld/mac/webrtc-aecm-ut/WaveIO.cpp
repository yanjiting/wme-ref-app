#include <string.h>
#include "waveio.h"

CWavFileOp::CWavFileOp(char *name,const char *mode)
{
    char riff[8],wavefmt[8];
    short i;
    
    m_fp = fopen(name,mode);
    if(!m_fp)
        m_FileStatus = -2;  //cann't open the file
    else if(strchr(mode,'r'))
    {
        //fread(&m_head,sizeof(struct SWavFileHead),1,m_fp);
        fread(&m_head.RIFF[0],4,1,m_fp);
        fread(&m_head.FileLength,sizeof(int),1,m_fp);
        fread(&m_head.WAVEfmt_[0],8,1,m_fp);
        for ( i=0;i<4;i++ )
            riff[i]= m_head.RIFF[i];
        for ( i=0;i<8;i++ )
            wavefmt[i]=m_head.WAVEfmt_[i];
        riff[4]='\0';
        wavefmt[7]='\0';
        if ( strcmp(riff,"RIFF")==0 && strcmp(wavefmt,"WAVEfmt")==0 )
        {
            m_FileStatus = 0; // It is WAV file.
            fread(&m_head.fmtSize,sizeof(int),1,m_fp);
            fread(&m_head.FormatCategory,sizeof(short),1,m_fp);
            fread(&m_head.NChannels,sizeof(short),1,m_fp);
            fread(&m_head.SampleRate,sizeof(int),1,m_fp);
            fread(&m_head.byteRate,sizeof(int),1,m_fp);
            fread(&m_head.blkAlign,sizeof(short),1,m_fp);
            fread(&m_head.NBitsPersample,sizeof(short),1,m_fp);
            if (m_head.fmtSize > 16) {  // Skip extra.
                char tmp[32];
                fread(&tmp[0],(m_head.fmtSize - 16),1,m_fp);
            }
            fread(&m_head.data[0],4,1,m_fp);
            fread(&m_head.RawDataFileLength,sizeof(int),1,m_fp);
            //Scan2Data();
        }
        else
            m_FileStatus = -1;
    }
    else
        m_FileStatus = 0;
}

int CWavFileOp::ReadHeader(SWavFileHead *head)
{
    if(m_FileStatus == 0)
        memcpy(head,&m_head,sizeof(struct SWavFileHead));
    
    return m_FileStatus;
    
}
int CWavFileOp::WriteHeader(SWavFileHead head)
{
    if(m_FileStatus == 0) {
        memcpy(&m_head,&head,sizeof(struct SWavFileHead));
        fwrite(&m_head,sizeof(struct SWavFileHead),1,m_fp);
    }
    return(m_FileStatus);
}

int CWavFileOp::ReadSample(short *Data,int len)
{
    int read_len = m_FileStatus;
    if(m_FileStatus == 0)
        read_len = fread(Data,sizeof(short),len,m_fp);

    return read_len;
}

int CWavFileOp::ReadFloatSample(float *Data,int len)
{
    int read_len = m_FileStatus;

    if(m_FileStatus == 0)
        read_len = fread(Data,sizeof(float),len,m_fp);
    for (int i = 0; i < read_len; ++i)
        Data[i] *= 32767.0;
    return read_len;
}

int CWavFileOp::WriteSample(short *Data,int len)
{
    if(m_FileStatus == 0)
        fwrite(Data,sizeof(short),len,m_fp);
    return (m_FileStatus);
}

int CWavFileOp::WriteFloatSample(float *Data,int len)
{
    if(m_FileStatus == 0)
        fwrite(Data,sizeof(float),len,m_fp);
    return (m_FileStatus);
}


CWavFileOp::~CWavFileOp()
{
    if(m_fp)
        fclose(m_fp);
}

int CWavFileOp::UpdateHeader(int outCh,int DataNum)
{
    if(m_FileStatus == 0) {
        int BitsPerSample = m_head.NBitsPersample;
        m_head.fmtSize		= 16;
        //m_head.FormatCategory		= 1;
        m_head.NChannels		= outCh;
        m_head.blkAlign	    = outCh* BitsPerSample/8;
        //m_head.SampleRate  = m_head.BytesPerSample*outCh*BitsPerSample/8;
        m_head.RawDataFileLength		= DataNum*BitsPerSample/8*outCh;
        m_head.FileLength		= m_head.RawDataFileLength + 36;
        
        rewind(m_fp);
        fwrite(&m_head,sizeof(SWavFileHead),1,m_fp);
    }
    return(m_FileStatus);
}

int CWavFileOp::Scan2Data()
{
    int i;
    char chunktype[5],temp;
    //int read_len = m_FileStatus;
    if(m_FileStatus != 0)
        return m_FileStatus;
    for ( i=0;i<4;i++ )
        chunktype[i]= m_head.data[i];
    chunktype[4]='\0';
    if ( strcmp(chunktype,"data")==0)
    {
        return m_FileStatus;// 
    }
    else
    {
        for(i=0;i<m_head.RawDataFileLength;i++)
            fread(&temp,1,1,m_fp);
        fread(&m_head.data,4,1,m_fp); 
        fread(&m_head.RawDataFileLength,4,1,m_fp);
        Scan2Data();
        return m_FileStatus;
    }
    
}
