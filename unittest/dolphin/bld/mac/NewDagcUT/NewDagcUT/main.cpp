//
//  main.cpp
//  NewDagcUT
//
//  Created by Ark Li on 7/28/15.
//  Copyright (c) 2015 Ark Li. All rights reserved.
//

#include <iostream>
#include "clientwmedagc.h"
#include "stdlib.h"
#include <string.h>
#define FRAME_LEN_10MS_MAX 480
/////#define DAGCUT

typedef struct
{
    char                szRiffID[4];  // 'R','I','F','F'
    unsigned int        dwRiffSize;
    char                szRiffFormat[4]; // 'W','A','V','E'
}RIFF_HEADER;


typedef struct
{
    unsigned short      wFormatTag;
    unsigned short      wChannels;
    unsigned int        dwSamplesPerSec;
    unsigned int        dwAvgBytesPerSec;
    unsigned short      wBlockAlign;
    unsigned short      wBitsPerSample;
    ///unsigned short      pack;        //additional information
}WAVE_FORMAT;

typedef struct
{
    char                szFmtID[4]; // 'f','m','t',' '
    unsigned int        dwFmtSize;
    WAVE_FORMAT         wavFormat;
}FMT_BLOCK;

typedef struct
{
    char            szFactID[4]; // 'f','a','c','t'
    unsigned int    dwFactSize;
    char            szFactData[4];
}FACT_BLOCK;

typedef struct
{
    char            szDataID[4]; // 'd','a','t','a'
    unsigned int    dwDataSize;
}DATA_BLOCK;

typedef union
{
    int  nValue;
    char charBuf[4];
}WAVE_SIZE_HELP;


int main(int argc, const char * argv[]) {
    
    FILE* finput = NULL;
    FILE* foutput = NULL;
    float init_db = 26;
    float tar_db = 26;
    float thr_db = 45;
    float limiter_db = 6;
    int nFileLen = 0;
    int output_Flen = 0;
    int mode = 1;
    int longtime = 5;
    int longtime_factor = 3;
    int temp_factor = 3;
    int low_thr = 200;
    int sample_rate = 16000;
    float max_up_setp = MAX_UP_GAIN;
    float max_down_step = MAX_DOWN_GAIN;
    short input_buf[FRAME_LEN_10MS_MAX];
    short output_buf[FRAME_LEN_10MS_MAX];
    float input_fdata[FRAME_LEN_10MS_MAX];
    float output_fdata[FRAME_LEN_10MS_MAX];
    
    if (argc >= 2){
        finput = fopen(argv[1], "r+b");
        ///finput = fopen("Capture-short16-Chn1-16000.pcm", "r+b");
    }
    if (argc >= 3){
        foutput = fopen(argv[2], "w+b");
        ///foutput = fopen("-15dBov_m2_2x7-5sec_out.pcm", "r+b");
    }
    if (argc >= 4){
        init_db = abs(atoi(argv[3]));
    }
    if (argc >= 5){
        tar_db = abs(atoi(argv[4]));
    }
    if (argc >= 6){
        thr_db = abs(atoi(argv[5]));
    }
    if (argc >= 7) {
        mode = abs(atoi(argv[6]));
        if (mode == 1){
            temp_factor = 3;
        }
        else if (mode == 2){
            temp_factor = 5;
        }
    }
    if (argc >= 8) {
        low_thr = abs(atoi(argv[7]));
    }
    if (argc >= 9) {
        limiter_db = abs(atoi(argv[8]));
    }
    if (argc >= 10) {
        sample_rate = abs(atoi(argv[8]));
    }
    
    /*
    if (argc >= 7){
        max_up_setp = powf(10, (float)(abs(atoi(argv[6]))) / 2000);
    }
    if (argc >= 8){
        max_down_step = powf(10, (float)(abs(atoi(argv[7]))) / (-2000));
    }
    if (argc >= 9){
        mode = abs(atoi(argv[8]));
        if (mode == 1){
            temp_factor = 3;
        }
        else if (mode == 2){
            temp_factor = 5;
        }
    }
    if (argc >= 10){
        limiter_db = abs(atoi(argv[9]));
    }
     
    if (argc >= 11){
        ///if (mode == 2){
        ///longtime = abs(atoi(argv[7]));
        ///myDagcDemo->init(init_db, tar_db, thr_db, mode, longtime);
        ///}
        ///else if (mode == 1){
        ///	longtime_factor = abs(atoi(argv[7]));
        ///	myDagcDemo->init(init_db, tar_db, thr_db, mode, longtime_factor);
        ///}
        temp_factor = abs(atoi(argv[10]));
    }*/
    
    CWmeDagc* myDagcDemo = new CWmeDagc();
    myDagcDemo->init(init_db, tar_db, thr_db, mode, temp_factor);
    myDagcDemo->set_limiter_para(limiter_db);
    
    WMEDagcControlParameters gainParas;
    gainParas.init_db = init_db;
    gainParas.target_db = tar_db;
    gainParas.thrshold_db = thr_db;
    gainParas.dagc_mode = mode;
    gainParas.dagc_factor = temp_factor;
    gainParas.low_volume_env = low_thr;
    gainParas.limiter_thr_db = limiter_db;
    gainParas.noise_gate_factor = 30;
    gainParas.echo_sup_thr = 1;
    gainParas.fast_tracking_num = 200;
    
    myDagcDemo->SetNewDagcParameters(gainParas);
    
    //// init wav head
    RIFF_HEADER input_riff;
    FMT_BLOCK input_fmt;
    DATA_BLOCK input_data;
    
    fread(&input_riff, sizeof(RIFF_HEADER), 1, finput);
    fread(&input_fmt, sizeof(FMT_BLOCK), 1, finput);
    if (input_fmt.dwFmtSize > 16){
        long offset = sizeof(RIFF_HEADER) + sizeof(FMT_BLOCK) + input_fmt.dwFmtSize - 16;
        fseek(finput, offset, SEEK_SET);
    }
    else if (input_fmt.dwFmtSize < 16){
        printf("Error on reading wav files!!");
    }
    fread(&input_data, sizeof(DATA_BLOCK), 1, finput);
    int input_len = input_data.dwDataSize;
    
    
    //// init wav head
    RIFF_HEADER m_riff;
    WAVE_FORMAT m_wavformat;
    FMT_BLOCK m_fmt;
    DATA_BLOCK m_data;
    WAVE_SIZE_HELP m_tempvalue;
    
    strncpy(m_riff.szRiffID, "RIFF", 4);
    strncpy(m_riff.szRiffFormat, "WAVE", 4);
    
    m_wavformat.wFormatTag = input_fmt.wavFormat.wFormatTag;
    m_wavformat.wChannels = 1;
    m_wavformat.dwSamplesPerSec = input_fmt.wavFormat.dwSamplesPerSec;
    m_wavformat.wBlockAlign = 2;
    m_wavformat.wBitsPerSample = 16;
    m_wavformat.dwAvgBytesPerSec = m_wavformat.dwSamplesPerSec * m_wavformat.wBlockAlign;
    
    strncpy(m_fmt.szFmtID, "fmt ", 4);
    m_fmt.wavFormat = m_wavformat;
    m_fmt.dwFmtSize = 16;
    
    strncpy(m_data.szDataID, "data", 4);
    
    ///add wav head
    fwrite(&m_riff, sizeof(RIFF_HEADER), 1, foutput);
    fwrite(&m_fmt, sizeof(FMT_BLOCK), 1, foutput);
    fwrite(&m_data, sizeof(DATA_BLOCK), 1, foutput);
    
    int wavhead = 0;
    wavhead = sizeof(RIFF_HEADER);
    wavhead += sizeof(FMT_BLOCK);
    wavhead += sizeof(DATA_BLOCK);
    
    int headlen = sizeof(RIFF_HEADER) + sizeof(FMT_BLOCK) + sizeof(DATA_BLOCK);
    
    int inputwav_type = input_fmt.wavFormat.wFormatTag;
    int inputwav_rate = input_fmt.wavFormat.dwSamplesPerSec;
//    if (inputwav_rate != 16000){
//        return 0;
//    }
    
    long frame_num = 0;
    int framelen_10ms = sample_rate/100;
    
    ////read file and using dagc to modify the signal
    while (!feof(finput)){
        if (inputwav_type == 3){
            unsigned long nRead = fread(input_fdata, sizeof(float), framelen_10ms, finput);
            frame_num++;
            for (int i = 0; i < framelen_10ms; i++){
                input_fdata[i] *= 32767;
            }
            nFileLen += nRead * sizeof(float);
        }
        else if (inputwav_type == 1){
            unsigned long nRead = fread(input_buf, sizeof(short), framelen_10ms, finput);
            frame_num++;
            nFileLen += nRead * sizeof(short);
            for (int i = 0; i < framelen_10ms; i++){
                input_fdata[i] = input_buf[i];
            }
        }
        
        
        if (nFileLen >= input_len){
            break;
        }
        myDagcDemo->DoAgcProcess(input_fdata, output_fdata, framelen_10ms,0);
        for (int i = 0; i < framelen_10ms; i++){
            output_buf[i] = (int)output_fdata[i];
            ///output_buf[i] = (short)input_fdata[i];
        }
        unsigned long nWrite = fwrite(output_buf, sizeof(short), framelen_10ms, foutput);
        output_Flen += sizeof(short) * nWrite;
    }
    
    /////update the wav head
    fseek(foutput, 4L, SEEK_SET);
    m_tempvalue.nValue = headlen + output_Flen - 8;
    fwrite(m_tempvalue.charBuf, 4, 1, foutput);
    fseek(foutput, 40L, SEEK_SET);
    m_tempvalue.nValue = output_Flen;
    fwrite(m_tempvalue.charBuf, 4, 1, foutput);
    
    m_data.dwDataSize = output_Flen;
    
    if (finput){
        fclose(finput);
    }
    
    if (foutput){
        fclose(foutput);
    }
    
    if (myDagcDemo){
        delete myDagcDemo;
    }

    return 0;
}
