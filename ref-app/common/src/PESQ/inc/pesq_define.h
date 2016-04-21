#ifndef _pesq_define_h
#define _pesq_define_h
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#define LINIIR 60

#define MAXNUTTERANCES 50

#define WHOLE_SIGNAL -1

#define LINIIR 60
#define LSMJ 20
#define LFBANK 35

#define DATAPADDING_MSECS 320
#define SEARCHBUFFER 75  

#define EPS 1E-12

#define MINSPEECHLGTH 4

#define JOINSPEECHLGTH 50

#define MINUTTLENGTH 50

#define SATDB 90.31
#define FIXDB -32.0

#define TWOPI 6.28318530717959

extern int Nb ;

#define Nfmax 512

#define Sp_8k   2.764344e-5
#define Sl_8k   1.866055e-1

#define Sp_16k  6.910853e-006
#define Sl_16k  1.866055e-001

extern float Sp;
extern float Sl;

#define Dz 0.312

#define gamma 0.001

#define Tl 10000.0f

#define Ts 10000000.0f

#define Tt 0.02f

#define Tn 0.01f


#ifndef min
  #define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
  #define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#define NB_MODE 0
#define WB_MODE 1

typedef struct {
  char  path_name[512];
  char  file_name [128];
  long  Nsamples;
  long  apply_swap;
  long  input_filter;

  float * data;
  float * VAD;
  float * logVAD;
} SIGNAL_INFO;

typedef struct {
  long Nutterances;
  long Largest_uttsize;
  long Nsurf_samples;

  long  Crude_DelayEst;
  float Crude_DelayConf;
  long  UttSearch_Start[MAXNUTTERANCES];
  long  UttSearch_End[MAXNUTTERANCES];
  long  Utt_DelayEst[MAXNUTTERANCES];
  long  Utt_Delay[MAXNUTTERANCES];
  float Utt_DelayConf[MAXNUTTERANCES];
  long  Utt_Start[MAXNUTTERANCES];
  long  Utt_End[MAXNUTTERANCES];

  float pesq_mos;
  float mapped_mos;

  short mode;

} ERROR_INFO;


extern long Fs;
extern long Downsample;
extern float * InIIR_Hsos;
extern long Align_Nfft;

extern long Fs_8k;
extern long Downsample_8k;
extern long InIIR_Nsos_8k;
extern long Align_Nfft_8k;

extern long Fs_16k;
extern long Downsample_16k;
extern long InIIR_Nsos_16k;
extern long Align_Nfft_16k;

extern float * InIIR_Hsos;

void input_filter(
       SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info, float * ftmp );
void apply_filters( float * data, long Nsamples );
void make_stereo_file (char *, SIGNAL_INFO *, SIGNAL_INFO *);
void make_stereo_file2 (char *, SIGNAL_INFO *, float *);
void select_rate( long sample_rate,
     long * Error_Flag, char ** Error_Type );
int  file_exist( char * fname );
void load_src( long * Error_Flag, char ** Error_Type,
     SIGNAL_INFO * sinfo);
void alloc_other( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info, 
    long * Error_Flag, char ** Error_Type, float ** ftmp);
void calc_VAD( SIGNAL_INFO * pinfo );
int  id_searchwindows( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
       ERROR_INFO * err_info );
void id_utterances( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
       ERROR_INFO * err_info );
void utterance_split( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
       ERROR_INFO * err_info, float * ftmp );
void utterance_locate( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
       ERROR_INFO * err_info, float * ftmp );
void auditory_transform( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
       ERROR_INFO * err_info, long Utt_id, float * ftmp);
void calc_err( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info, 
       ERROR_INFO * err_info, long Utt_id);
void extract_params( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
    ERROR_INFO * err_info, long Utt_id, float * ftmp );
void utterance_process(SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
       ERROR_INFO * err_info, long Utt_id, float * ftmp);
void DC_block( float * data, long Nsamples );
void apply_filter ( float * data, long Nsamples, int, double [][2] );
double pow_of (const float * const , long , long, long);
void apply_VAD(
     SIGNAL_INFO * pinfo, float * data, float * VAD, float * logVAD );
void crude_align(
     SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info, ERROR_INFO * err_info,
     long Utt_id, float * ftmp);
void time_align(
     SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info, ERROR_INFO * err_info,
     long Utt_id, float * ftmp );
void split_align( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
     ERROR_INFO * err_info, float * ftmp,
     long Utt_Start, long Utt_SpeechStart, long Utt_SpeechEnd, long Utt_End,
     long Utt_DelayEst, float Utt_DelayConf,
     long * Best_ED1, long * Best_D1, float * Best_DC1,
     long * Best_ED2, long * Best_D2, float * Best_DC2,
     long * Best_BP );
void pesq_psychoacoustic_model(
SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
ERROR_INFO * err_info, float * ftmp);
void apply_pesq( float * x_data, float * ref_surf,
float * y_data, float * deg_surf, long NVAD_windows, float * ftmp,
ERROR_INFO * err_info );



#define     D_POW_F     2
#define     D_POW_S     6
#define     D_POW_T     2

#define     A_POW_F     1
#define     A_POW_S     6
#define     A_POW_T     2


#define     D_WEIGHT    0.1
#define     A_WEIGHT    0.0309

#endif
