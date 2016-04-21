#ifndef TEST_SET_AQE_HEADER
#define TEST_SET_AQE_HEADER



#define  PROCESS_32K
#define  RUN_TIME_S            (30)


#define  FRAME_LEN_8K_10MS     (8*10)
#define  FRAME_LEN_8K_20MS     (8*20)
#define  FRAME_LEN_8K_30MS     (8*30)
#define  FRAME_LEN_12K_10MS    (12*10)
#define  FRAME_LEN_12K_20MS    (12*20)
#define  FRAME_LEN_12K_30MS    (12*30)
#define  FRAME_LEN_16K_10MS    (16*10)
#define  FRAME_LEN_16K_20MS    (16*20)
#define  FRAME_LEN_16K_30MS    (16*30)
#define  FRAME_LEN_24K_10MS    (24*10)
#define  FRAME_LEN_24K_20MS    (24*20)
#define  FRAME_LEN_24K_30MS    (24*30)
#define  FRAME_LEN_32K_10MS    (32*10)
#define  FRAME_LEN_32K_20MS    (32*20)
#define  FRAME_LEN_32K_30MS    (32*30)
#define  FRAME_LEN_44K_10MS    (441*1)
#define  FRAME_LEN_44K_20MS    (441*2)
#define  FRAME_LEN_44K_30MS    (441*3)
#define  FRAME_LEN_48K_10MS    (48*10)
#define  FRAME_LEN_48K_20MS    (48*20)
#define  FRAME_LEN_48K_30MS    (48*30)


#define	 PLAYBACK_8K        ("Playback_8k.pcm")
#define  CAPTURE_8K		    ("Capture_8k.pcm")
#define	 PLAYBACK_12K       ("Playback_12k.pcm")
#define  CAPTURE_12K		("Capture_12k.pcm")
#define	 PLAYBACK_16K       ("Playback_16k.pcm")
#define  CAPTURE_16K		("Capture_16k.pcm")
#define	 PLAYBACK_24K       ("Playback_24k.pcm")
#define  CAPTURE_24K		("Capture_24k.pcm")
#define	 PLAYBACK_32K       ("Playback_32k.pcm")
#define  CAPTURE_32K		("Capture_32k.pcm")
#define	 PLAYBACK_44K       ("Playback_44k.pcm")
#define  CAPTURE_44K		("Capture_44k.pcm")
#define	 PLAYBACK_48K       ("Playback_48k.pcm")
#define  CAPTURE_48K		("Capture_48k.pcm")


#ifdef  PROCESS_8K
FILE* pPlaybackFile = fopen(PLAYBACK_8K,"rb");
FILE* pCaptureFile  = fopen(CAPTURE_8K,"rb");
#define PLAYBACK_SAMPLE_NUM     FRAME_LEN_8K_20MS
#define PLAYBACK_SAMPLE_RATE    AE_SAMPLE_RATE_8K
#define CAPTURE_SAMPLE_NUM		FRAME_LEN_8K_10MS
#define CAPTURE_SAMPLE_RATE		AE_SAMPLE_RATE_8K
#define PLAYBACK_INTERVAL		(20)
#define CAPTURE_INTERVAL		(10)
#endif

#ifdef  PROCESS_12K
FILE* pPlaybackFile = fopen(PLAYBACK_12K,"rb");
FILE* pCaptureFile  = fopen(CAPTURE_12K,"rb");
#define PLAYBACK_SAMPLE_NUM     FRAME_LEN_12K_20MS
#define PLAYBACK_SAMPLE_RATE    AE_SAMPLE_RATE_12K
#define CAPTURE_SAMPLE_NUM		FRAME_LEN_12K_10MS
#define CAPTURE_SAMPLE_RATE		AE_SAMPLE_RATE_12K
#define PLAYBACK_INTERVAL		(20)
#define CAPTURE_INTERVAL		(10)
#endif

#ifdef  PROCESS_16K
FILE* pPlaybackFile = fopen(PLAYBACK_16K,"rb");
FILE* pCaptureFile  = fopen(CAPTURE_16K,"rb");
#define PLAYBACK_SAMPLE_NUM     FRAME_LEN_16K_20MS
#define PLAYBACK_SAMPLE_RATE    AE_SAMPLE_RATE_16K
#define CAPTURE_SAMPLE_NUM		FRAME_LEN_16K_10MS
#define CAPTURE_SAMPLE_RATE		AE_SAMPLE_RATE_16K
#define PLAYBACK_INTERVAL		(20)
#define CAPTURE_INTERVAL		(10)
#endif

#ifdef  PROCESS_24K
FILE* pPlaybackFile = fopen(PLAYBACK_24K,"rb");
FILE* pCaptureFile  = fopen(CAPTURE_24K,"rb");
#define PLAYBACK_SAMPLE_NUM     FRAME_LEN_24K_20MS
#define PLAYBACK_SAMPLE_RATE    AE_SAMPLE_RATE_24K
#define CAPTURE_SAMPLE_NUM		FRAME_LEN_24K_10MS
#define CAPTURE_SAMPLE_RATE		AE_SAMPLE_RATE_24K
#define PLAYBACK_INTERVAL		(20)
#define CAPTURE_INTERVAL		(10)
#endif

#ifdef  PROCESS_32K
FILE* pPlaybackFile = fopen(PLAYBACK_32K,"rb");
FILE* pCaptureFile  = fopen(CAPTURE_32K,"rb");
#define PLAYBACK_SAMPLE_NUM     FRAME_LEN_32K_20MS
#define PLAYBACK_SAMPLE_RATE    AE_SAMPLE_RATE_32K
#define CAPTURE_SAMPLE_NUM		FRAME_LEN_32K_10MS
#define CAPTURE_SAMPLE_RATE		AE_SAMPLE_RATE_32K
#define PLAYBACK_INTERVAL		(20)
#define CAPTURE_INTERVAL		(10)
#endif

#ifdef  PROCESS_44K
FILE* pPlaybackFile = fopen(PLAYBACK_44K,"rb");
FILE* pCaptureFile  = fopen(CAPTURE_44K,"rb");
#define PLAYBACK_SAMPLE_NUM     FRAME_LEN_44K_20MS
#define PLAYBACK_SAMPLE_RATE    AE_SAMPLE_RATE_44K
#define CAPTURE_SAMPLE_NUM		FRAME_LEN_44K_10MS
#define CAPTURE_SAMPLE_RATE		AE_SAMPLE_RATE_44K
#define PLAYBACK_INTERVAL		(20)
#define CAPTURE_INTERVAL		(10)
#endif

#ifdef  PROCESS_48K
FILE* pPlaybackFile = fopen(PLAYBACK_48K,"rb");
FILE* pCaptureFile  = fopen(CAPTURE_48K,"rb");
#define PLAYBACK_SAMPLE_NUM     FRAME_LEN_48K_20MS
#define PLAYBACK_SAMPLE_RATE    AE_SAMPLE_RATE_48K
#define PLAYBACK_INTERVAL		(20)
#define CAPTURE_SAMPLE_NUM		FRAME_LEN_48K_20MS
#define CAPTURE_SAMPLE_RATE		AE_SAMPLE_RATE_48K
#define CAPTURE_INTERVAL		(20)
#endif



#endif

