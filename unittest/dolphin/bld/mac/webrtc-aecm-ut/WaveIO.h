#ifndef CONFLUENCE_WAVEIO_H
#define CONFLUENCE_WAVEIO_H

#include <stdio.h>

#if 0
struct SWavDataChk
{
    //Data Flag  "data"
    char data[4];
    //Raw Data File Length
    int RawDataFileLength;
    // Void *audioData;
};

struct SWavRiffChk
{
    char RIFF[4];
    //File Length ( not include 8 bytes from the beginning ) (4-7)
    int FileLength;
    //WAVE File Flag (8-15) "WAVEfmt "
    char WAVE[4];
};


struct SWavFmtChk
{
    char fmt[4];
    //Transitory Byte ( normally it is 10H 00H 00H 00H ) (16-19)
    //Format chunk size - 8. (16-19)
    unsigned int fmtSize;
    //Format Category ( normally it is 1 means PCM-u Law ) (20-21)
    short FormatCategory;
    //NChannels (22-23)
    short NChannels;
    //Sample Rate (24-27)
    int SampleRate;
    //l=NChannels*SampleRate*NBitsPersample/8 (28-31)
    int byteRate;
    //i=NChannels*NBitsPersample/8 (32-33)
    short blkAlign;
    //NBitsPersample (34-35)
    short NBitsPersample;
    //Extension
    short ExtraPara;
};


struct SWavFileHead
{
    SWavRiffChk riff;
    SWavFmtChk fmt;
    SWavDataChk data;
};

#endif

struct SWavFileHead 
{
	//Resource Interchange File Flag (0-3) "RIFF"
	char RIFF[4];
	//File Length ( not include 8 bytes from the beginning ) (4-7)
	int FileLength;
	//WAVE File Flag (8-15) "WAVEfmt "
	char WAVEfmt_[8];
	//Transitory Byte ( normally it is 10H 00H 00H 00H ) (16-19)
    //Format chunk size - 8. (16-19)
	unsigned int fmtSize;
	//Format Category ( normally it is 1 means PCM-u Law ) (20-21)
	short FormatCategory;
	//NChannels (22-23)
	short NChannels;
	//Sample Rate (24-27)
	int SampleRate;
    //l=NChannels*SampleRate*NBitsPersample/8 (28-31)
    int byteRate;
    //i=NChannels*NBitsPersample/8 (32-33)
    short blkAlign;
	//NBitsPersample (34-35)
	short NBitsPersample;
	//Data Flag  "data"
	char data[4];
	//Raw Data File Length 
	int RawDataFileLength;
	
};



class CWavFileOp 
{
	private:
		SWavFileHead m_head;
		char *m_pData;
		FILE *m_fp;

	public:
		int m_FileStatus;
		CWavFileOp(char *name,const char *mode = "r");
		int ReadHeader(SWavFileHead *head);
		int WriteHeader(SWavFileHead head);
		int ReadSample(short *Data,int len);
        int ReadFloatSample(float *Data,int len);
		int WriteSample(short *Data,int len );
        int WriteFloatSample(float *Data,int len );
		int UpdateHeader(int outCh,int DataNum);
		int Scan2Data();
		~CWavFileOp();	
};
#endif