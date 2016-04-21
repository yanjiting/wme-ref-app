#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// add by chen, for test
#include <sstream>
#include <sys/stat.h>
#include <algorithm>
#include "WaveIO.h"
#include "IAEC.h"
#include "clientwebexagcapi.h"

#define IO_AEC_FILE

#define framesize 10.f
#define sample_rate_hz 8000
#define framelen 10
#define num_render_channels 1

#define ALIGNED_DECLARE( type, var, n ) type var __attribute__((aligned(n)))

int main(int argc, char* argv[])

{
#ifdef debug0
	__int64 kclock=0;
#endif
	///system("dir");
    //ALIGNED_DECLARE( SWavFileHead, wavhead, 2);
	SWavFileHead wavhead;
	short *data_in_s,*data_out_s,*dagc_out_s;
	float *neardata, *fardata,*data_err,*dagc_out_f;
	data_in_s=NULL;
	data_out_s = NULL;
    dagc_out_s = NULL;
	char *nearfile, *farfile;
	nearfile=NULL;
	farfile =NULL;
	char *outfile;
	outfile =NULL;
    char *outfile_dagc;
    outfile_dagc = NULL;
    char default_dagc_outputfile[] = {"DagcOut.wav"};
    
	CWavFileOp *readfarfile;
	CWavFileOp *readnearfile;
	CWavFileOp *writefile;
    CWavFileOp *writedagcfile;
	readfarfile= NULL;
	readnearfile =NULL;
	writefile = NULL;
    writedagcfile = NULL;

//	AudioBuffer* render_audio_;
//	AudioBuffer* capture_audio_;
	
	int samples_per_channel;
	int farfilelen,nearfilelen;
	int processinglen;
	int outfileleng;
	int i;
	 int Mynlpflag, Mydelayflag;
	 int WorkFs;
	 int framecnt;
    int dagc_type = 0;
    float init_db = 26;
    float tar_db = 26;
    float thr_db = 45;
    float limiter_db = 6;
    int mode = 1;
    int low_thr = 200;
    int speed_factor = 3;

//	 DelayFile = NULL; 
    unsigned short a = 65535;
    unsigned short b = 0;
    short c = (short)b - (short)a;

	if(argc<4)
	{
		printf("Not Enough Input Parameters!");
		return 0;
	}
	nearfile=argv[1];
	farfile =argv[2];
	outfile =argv[3];
    
    if (argc >= 5){
        outfile_dagc = argv[4];
    }
    if (argc >= 6) {
        int type = abs(atoi(argv[5]));
        if (4 == type) {
            dagc_type = 1;
        }else{
            dagc_type = 0;
        }
    }
    
    if (argc >= 7){
        init_db = abs(atoi(argv[6]));
    }
    if (argc >= 8){
        tar_db = abs(atoi(argv[7]));
    }
    if (argc >= 9){
        thr_db = abs(atoi(argv[8]));
    }
    if (argc >= 10){
        mode = abs(atoi(argv[9]));
    }
    if (argc >= 11){
        low_thr = abs(atoi(argv[10]));
    }
    if (argc >= 12){
        limiter_db = abs(atoi(argv[11]));
    }
    
	//WorkFs = atoi(argv[4]);
/*
	if (0==strcmp(argv[5],"--NLPOff"))
	{
		Mynlpflag=0;
	}
	else if (0==strcmp(argv[5],"--NLPOn"))
	{
		Mynlpflag =1;
	}
	else
	{
		Mynlpflag =-1;
		printf("wrong nlp write parameters set");
		return -1;
	}
	if (0==strcmp(argv[6],"--DelayOff"))
	{
		Mydelayflag=0;
	}
	else if (0==strcmp(argv[6],"--DelayOn"))
	{
		Mydelayflag=1;
		//DelayFile = fopen("DelayRecord.dat","w");
	}
	else
	{
		Mydelayflag=-1;
		printf("wrong delay write parameters set");
		return -1;
	}
*/
//	framefile=fopen("frmaeAECCPU.txt","w");
//	nearfile="near.wav";
//	farfile = "far.wav";
//	outfile="aecm_NSOut.wav";
	readnearfile=new CWavFileOp(nearfile,"rb");
	readfarfile = new CWavFileOp(farfile,"rb");
	if (readfarfile->m_FileStatus==-2)
	{
		delete readfarfile;
		printf("Can not open the 1st Input wav file!");
		return 0;
	}
	if (readnearfile->m_FileStatus==-2)
	{
		delete readfarfile;
		printf("Can not open the 2nd Input wav file!");
		return 0;
	}

	writefile=new CWavFileOp(outfile,"wb");
	if (writefile->m_FileStatus==-2)
	{
		delete writefile;
		return 0;
	}
    
    if (outfile_dagc) {
        writedagcfile = new CWavFileOp(outfile_dagc, "wb+");
    }else{
        writedagcfile = new CWavFileOp(default_dagc_outputfile, "wb+");
    }
    
    if (writedagcfile->m_FileStatus == -2) {
        delete writedagcfile;
        printf("Can not open the Dagc Output wav file!");
        return 0;
    }

	readfarfile->ReadHeader(&wavhead);

	if (wavhead.NChannels!=1)
	{
		printf("Only mono wav file can be processed!");
		return 0;
	}
	if ((wavhead.SampleRate!=8000)&&(wavhead.SampleRate!=16000))
	{
		printf("only 8k and 16k wav file are supported");
		return -1000;
	}
	farfilelen=wavhead.RawDataFileLength/wavhead.blkAlign*wavhead.NChannels;

	readnearfile->ReadHeader(&wavhead);
	if (wavhead.NChannels!=1)
	{
		printf("only nomo wav file is supported");
		return 0;
	}
	if ((wavhead.SampleRate!=8000)&&(wavhead.SampleRate!=16000))
	{
		printf("only 8k and 16k wav file are supported");
		return -1000;
	}
    
	//nearfilelen=(*((int*)(&wavhead.data[4])))/wavhead.BytesPerSample*wavhead.NChannels;
    nearfilelen= wavhead.RawDataFileLength/wavhead.blkAlign*wavhead.NChannels;
	processinglen = farfilelen>=nearfilelen?nearfilelen:farfilelen;

	int fremaelen=int(framesize*wavhead.SampleRate/1000);
	
	data_in_s=new short[fremaelen*wavhead.NChannels];
	data_out_s=new short[fremaelen*wavhead.NChannels];
    dagc_out_s = new short[fremaelen*wavhead.NChannels];
	memset(data_in_s,0,(fremaelen*wavhead.NChannels)*sizeof(short));
	memset(data_out_s,0,fremaelen*wavhead.NChannels*sizeof(short));
    memset(dagc_out_s,0,fremaelen*wavhead.NChannels*sizeof(short));
	neardata = new float[fremaelen*wavhead.NChannels];
	fardata = new float[fremaelen*wavhead.NChannels];
	data_err = new float[fremaelen*wavhead.NChannels];
    dagc_out_f = new float[fremaelen*wavhead.NChannels];

#ifdef unpreprocess
    preposfil_des=new CPrePosAna(wavhead.SampleRate,10.f,1);
#endif

	#ifdef WEBRTC_AUDIOPROC_DEBUG_DUMP
      debug_file_(FileWrapper::Create()),
      event_msg_(new audioproc::Event()),
	#endif

    //AEC tuning
    AEC_NLP_Param param;
    param.energy_dev_tol=512; ///400;
    param.resolution_supgain=8;   //do not change
    param.supgain_default=256;    //do not change
    param.supgain_err_para_a=6144; /////3072;
    param.supgain_err_para_b=3072; /////1536;
    param.supgain_err_para_c=0;
    param.supgain_err_para_d=768;  /////256; //do not change
    param.supgain_epc_dt=256;  //////200;
    
    IClientWebExAgc* p_dagc = NULL;
    AGCParameters dagc_para;
    dagc_para.sampleRate = wavhead.SampleRate;
    dagc_para.oneFrameSize = wavhead.SampleRate/100;
    dagc_para.oneframeTime = 10;
    CreateIClientWebExAgc(&p_dagc, dagc_para, dagc_type);
    WMEDagcControlParameters newDagc_para;
    newDagc_para.init_db = init_db;
    newDagc_para.target_db = tar_db;
    newDagc_para.thrshold_db = thr_db;
    newDagc_para.dagc_mode = mode;
    newDagc_para.dagc_factor = 3;
    newDagc_para.limiter_thr_db = limiter_db;
    newDagc_para.low_volume_env = low_thr;
    newDagc_para.echo_sup_thr = 500;
    newDagc_para.noise_gate_factor = 30;
    p_dagc->SetNewDagcParameters(newDagc_para);


	IAEC* cap= CreateIAECInst(wavhead.SampleRate,wavhead.SampleRate/100,10,&param,100);
	cap->SetAEC_OnOff(ON);
	cap->SetNR_OnOff(ON);
	cap->SetWriteDebugInfo(true);
	cap->SetOffset(20);		// will not impact

	writefile->WriteHeader(wavhead);
    writedagcfile->WriteHeader(wavhead);
	outfileleng=0;
	framecnt = 0;
	while (outfileleng<(processinglen-fremaelen*wavhead.NChannels))
	{
		framecnt=framecnt+1;		
		/* Read short data. */
        //outfileleng+= readfarfile->ReadSample(data_in_s,fremaelen*wavhead.NChannels);		// fardata
		//readnearfile->ReadSample(data_out_s,fremaelen*wavhead.NChannels);
        
        /* Read float data. */
        outfileleng+= readfarfile->ReadFloatSample(fardata,fremaelen*wavhead.NChannels);		// fardata
        readnearfile->ReadFloatSample(neardata,fremaelen*wavhead.NChannels);
        
        

        
        
#ifdef debug0

#endif

#ifdef debug0
		//kclock=0;
		//kclock-=rdtsc();
#endif
#ifdef unpreprocess
		preposfil_des->preprocess(data_out_f_des,data_in_f_des,0);
#endif
		cap->process(neardata,fardata,data_err,wavhead.SampleRate/100,wavhead.SampleRate/100,1);
        AEC_Out_info aecout_info = cap->GetAecInfo();
#ifdef debug0

#endif
        
		for(i=0;i<fremaelen;i++)
		{
            data_err[i] *= (1.0f/32768.0f);
            //*(data_out_s+i)=(short)(*(data_err+i));
		}
		writefile->WriteFloatSample(data_err,fremaelen);
        //writefile->WriteSample(data_out_s,fremaelen);
        
        p_dagc->DoAgcProcess(data_err, dagc_out_f, wavhead.SampleRate/100, aecout_info.echo_flag);
        for(i=0;i<fremaelen;i++)
        {
            *(dagc_out_s+i)=(short)(*(dagc_out_f+i));
        }
        writedagcfile->WriteSample(dagc_out_s, fremaelen);
		//fwrite(neardata,sizeof(short),fremaelen,);

	}
    
	writefile->UpdateHeader(1,outfileleng);
    fprintf(stderr, "Total %d frame processed.\n", framecnt);
    writedagcfile->UpdateHeader(1, outfileleng);
#ifdef debug0
	//printf("\n Whole data length=%f s",float(farfilelen)/wavhead.SampleRate/wavhead.NChannels);
//	printf("\n Compute time     =%f s\n",kclock/cpu);
//	printf("Compute time percentage= %f%% \n",kclock*wavhead.SampleRate/cpu/float(filelen)*100*wavhead.NChannels);
#endif

	if(NULL !=cap)
	{
		DeleteIAECInst(cap);
		cap = NULL;
	}
    if (NULL != p_dagc) {
        DestroyIClientWebExAgc(p_dagc);
        p_dagc = NULL;
    }
    
	if(NULL!= readfarfile)
	{
		delete readfarfile;
		readfarfile = NULL;
	}
	if(NULL!= readnearfile)
	{
		delete readnearfile;
		readnearfile =NULL;
	}
	if(NULL!= writefile)
	{
		delete writefile;
		writefile =NULL;
	}
    if (NULL != writedagcfile) {
        delete writedagcfile;
        writedagcfile = NULL;
    }

	if(NULL!= data_in_s)
	{
		delete[] data_in_s;
		data_in_s = NULL;
	}
	if(NULL!= data_out_s)
	{
		delete[] data_out_s;
		data_out_s = NULL;
	}
    if (NULL != dagc_out_s) {
        delete[] dagc_out_s;
        dagc_out_s = NULL;
    }
    if (NULL != dagc_out_f) {
        delete[] dagc_out_f;
        dagc_out_f = NULL;
    }
	
	return 0;
}

