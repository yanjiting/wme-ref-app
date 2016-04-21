#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

/// Enum of audio raw format type
typedef enum
{
	WmeAudioUnknown   = 0,
	WmePCM,
}WmeAudioRawType;

/// Struct of audio raw format
typedef struct _tagAudioRawFormat
{
	WmeAudioRawType		eRawType;		///< Identify the audio raw format type
	int				iChannels;		///< number of channels (i.e. mono, stereo...)
	int				iSampleRate;	///< Sample rate
	int				iBitsPerSample;	///< Number of bits per sample of raw data
}WmeAudioRawFormat;

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
	float				fFrameRate;		///< Frame rate
	unsigned int			uTimestamp;		///< Frame timestamp
}WmeVideoRawFormat;

typedef struct _tagFrameHeader {
    unsigned int        iHeaderSize;
    WmeMediaFormatType  eMType;
    unsigned int        iFrameSize;
    union
    {
        WmeVideoRawFormat	sVideoRawFormat;
        WmeAudioRawFormat   sAudioRawFormat;
    };
}SFrameHeader;

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

#define Y_R_IN			0.257
#define Y_G_IN			0.504
#define Y_B_IN			0.098
#define Y_ADD_IN		16

#define U_R_IN			0.148
#define U_G_IN			0.291
#define U_B_IN			0.439
#define U_ADD_IN		128

#define V_R_IN			0.439
#define V_G_IN			0.368
#define V_B_IN			0.071
#define V_ADD_IN		128

#define SCALEBITS_IN    8
#define FIX_IN(x)		((unsigned short) ((x) * (1L<<SCALEBITS_IN) + 0.5))
#define FIX_ROUND		(1L<<(SCALEBITS_IN-1))


#define WELS_RGB_Y(P_SIZE, ROW_NUM, UV, D1,D2,D3,D4)	\
r##UV += r = x_ptr[(ROW_NUM)*x_stride+(D1)];						\
g##UV += g = x_ptr[(ROW_NUM)*x_stride+(D2)];						\
b##UV += b = x_ptr[(ROW_NUM)*x_stride+(D3)];						\
y_ptr[(ROW_NUM)*y_stride] =									\
(unsigned char) ((FIX_IN(Y_R_IN) * r + FIX_IN(Y_G_IN) * g +	\
            FIX_IN(Y_B_IN) * b + FIX_ROUND) >> SCALEBITS_IN) + Y_ADD_IN;	\
r##UV += r = x_ptr[(ROW_NUM)*x_stride+(P_SIZE)+(D1)];				\
g##UV += g = x_ptr[(ROW_NUM)*x_stride+(P_SIZE)+(D2)];				\
b##UV += b = x_ptr[(ROW_NUM)*x_stride+(P_SIZE)+(D3)];				\
y_ptr[(ROW_NUM)*y_stride+1] =									\
(unsigned char) ((FIX_IN(Y_R_IN) * r + FIX_IN(Y_G_IN) * g +	\
            FIX_IN(Y_B_IN) * b + FIX_ROUND) >> SCALEBITS_IN) + Y_ADD_IN;

#define WELS_RGB_UV(ROW_NUM,UV)	\
u_ptr[(ROW_NUM)*uv_stride] =														\
(unsigned char) ((-FIX_IN(U_R_IN) * r##UV - FIX_IN(U_G_IN) * g##UV +			\
            FIX_IN(U_B_IN) * b##UV + 4*FIX_ROUND) >> (SCALEBITS_IN + 2)) + U_ADD_IN;	\
v_ptr[(ROW_NUM)*uv_stride] =														\
(unsigned char) ((FIX_IN(V_R_IN) * r##UV - FIX_IN(V_G_IN) * g##UV -			\
            FIX_IN(V_B_IN) * b##UV + 4*FIX_ROUND) >> (SCALEBITS_IN + 2)) + V_ADD_IN;


#define WELS_RGB_TO_YV12(P_SIZE,D1,D2,D3,D4)	\
unsigned int r, g, b, r0, g0, b0;		\
r0 = g0 = b0 = 0;					\
WELS_RGB_Y(P_SIZE, 0, 0, D1,D2,D3,D4)	\
WELS_RGB_Y(P_SIZE, 1, 0, D1,D2,D3,D4)	\
WELS_RGB_UV(     0, 0)

void	rgb_to_i420(unsigned char * x_ptr, int x_stride, unsigned char * y_ptr, unsigned char * u_ptr, unsigned char * v_ptr,
					  int y_stride, int uv_stride, int width, int height)
{
	int fixed_width = (width + 1) & ~1;
	int x_step = (x_stride << 1) - ((fixed_width <<1) + fixed_width);
	int y_step = (y_stride << 1) - fixed_width;
	int uv_step = uv_stride - (fixed_width >> 1);
	int x, y;
	for (y = 0; y < height; y+=(2))
	{
		for (x = 0; x < fixed_width; x+=(2))
		{
			WELS_RGB_TO_YV12(3,0,1,2,0);
			x_ptr += (2)*(3);
			y_ptr += (2);
			u_ptr += (2)/2;
			v_ptr += (2)/2;
		}
		x_ptr += x_step;
		y_ptr += y_step;
		u_ptr += uv_step;
		v_ptr += uv_step;
	}
}

void yuy2_to_i420(unsigned char *yuy2, unsigned char *y, unsigned char *u, unsigned char *v, int width, int height)
{
    int idx_y, idx_u,idx_v, i, j;
    idx_y = idx_u = idx_v = 0;
    for (j = 0; j < height; j+=2)
    {
        for (i = 0; i<width*2; i+=1)
        {
            int idx = j*width+i;
            y[idx_y++]=yuy2[idx*2];
            if (i<width)
            {
                if (i%2==0)
                    u[idx_u++]=(yuy2[idx*2+1]+yuy2[idx*2+width*2+1])/2;
                else
                    v[idx_v++]=(yuy2[idx*2+1]+yuy2[idx*2+width*2+1])/2;
            }
        }
    }
}

void general_ds_bilinear_accurate_c(unsigned char* dst, const int dst_stride, const int dst_width, const int dst_height,
									unsigned char* src, const int src_stride, const int src_width, const int src_height )
{
    
	const int SCALE_BIT = 15;
	const int SCALE = (1 << SCALE_BIT);
	int fScalex = (int)((float)src_width / (float)dst_width * SCALE);
	int fScaley = (int)((float)src_height / (float)dst_height * SCALE);
	int iStep = (int)(((dst_width-1) / src_width) + 1);		   // for boundary protection
	int jStep = (int)(((dst_height-1) / src_height)  + 1);		// for boundary protection
	long long x;
	int fYInverse, fXInverse;
    
	unsigned char* pbyDst = dst;
	unsigned char* pbyLineDst = dst;
    
	//fYInverse = 1 << (SCALE_BIT - 1);
	fYInverse = 0;
	for(int i = 0; i < dst_height - jStep ; i++)
	{
		int yy = fYInverse >> SCALE_BIT;
		int fv = fYInverse & (SCALE - 1);
        
		unsigned char* pBySrc = src + yy * src_stride;
        
		pbyDst = pbyLineDst;
		//fXInverse = 1 << (SCALE_BIT - 1);
		fXInverse = 0;
		for(int j = 0; j < dst_width - iStep ; j++)
		{
			int xx = fXInverse >> SCALE_BIT;
			int fu = fXInverse & (SCALE - 1);
            
			unsigned char* pbyCurrent = pBySrc + xx;
			unsigned char a, b, c, d;
            
			a = *pbyCurrent;
			b = *(pbyCurrent + 1 );
			c = *(pbyCurrent + src_stride);
			d = *(pbyCurrent + src_stride + 1 );
            
			x = (((long long)(SCALE - 1 - fu))*(SCALE - 1 - fv)*a + ((long long)fu)*(SCALE - 1 -fv)*b + ((long long)(SCALE - 1 -fu))*fv*c +
                 ((long long)fu)*fv*d + (long long)(1 << (2*SCALE_BIT-1)) ) >> (2*SCALE_BIT);
			x = x < 0 ? 0 : x > 255 ? 255 : x;
			*pbyDst++ = (unsigned char)x;
            
			fXInverse += fScalex;
		}
        
		// horizontal boundary value assignment
		for (int k = 0 ; k < iStep; k++)
		{
			int xx = fXInverse >> SCALE_BIT;
			int fu = fXInverse & (SCALE - 1);
            
			unsigned char* pbyCurrent = pBySrc + xx;
			unsigned char a, b, c, d;
            
			int nx = (xx+1)<=(src_width - 1);
            
			a = *pbyCurrent;
			b = *(pbyCurrent  + nx);
			c = *(pbyCurrent + src_stride);
			d = *(pbyCurrent + src_stride +nx );
            
			x = (((long long)(SCALE - 1 - fu))*(SCALE - 1 - fv)*a + ((long long)fu)*(SCALE - 1 -fv)*b + ((long long)(SCALE - 1 -fu))*fv*c +
                 ((long long)fu)*fv*d + (long long)(1 << (2*SCALE_BIT-1)) ) >> (2*SCALE_BIT);
			x = x < 0 ? 0 : x > 255 ? 255 : x;
			*pbyDst++ = (unsigned char)x;
            
			fXInverse += fScalex;
		}
		pbyLineDst += dst_stride;
		fYInverse += fScaley;
	}
    
	// vertical boundary value assignment
	for(int i = 0; i < jStep; i++)
	{
		int yy = fYInverse >> SCALE_BIT;
		int fv = fYInverse & (SCALE - 1);
		unsigned char* pBySrc = src + yy * src_stride;
        
		int ny = (yy+1)<=(src_height - 1);
		pbyDst = pbyLineDst;
		
		//fXInverse = 1 << (SCALE_BIT - 1);
		fXInverse = 0;
		for(int j = 0; j < dst_width; j++)
		{
			int xx = fXInverse >> SCALE_BIT;
			int fu = fXInverse & (SCALE - 1);
            
			unsigned char* pbyCurrent = pBySrc + xx;
			unsigned char a, b, c, d;
            
			int nx = (xx+1)<=(src_width - 1);
            
			a = *pbyCurrent;
			b = *(pbyCurrent  + nx);
			c = *(pbyCurrent + ny*src_stride);
			d = *(pbyCurrent + ny*src_stride +nx );
            
			x = (((long long)(SCALE - 1 - fu))*(SCALE - 1 - fv)*a + ((long long)fu)*(SCALE - 1 -fv)*b + ((long long)(SCALE - 1 -fu))*fv*c +
                 ((long long)fu)*fv*d + (long long)(1 << (2*SCALE_BIT-1)) ) >> (2*SCALE_BIT);
			x = x < 0 ? 0 : x > 255 ? 255 : x;
			*pbyDst++ = (unsigned char)x;
            
			fXInverse += fScalex;
		}
		pbyLineDst += dst_stride;
		fYInverse += fScaley;
	}
}

void general_ds_bilinear_fast_c(unsigned char* dst, const int dst_stride, const int dst_width, const int dst_height,
								unsigned char* src, const int src_stride, const int src_width, const int src_height )
{
    
	const unsigned int SCALE_BIT_WIDTH = 16, SCALE_BIT_HEIGHT = 15;
	const unsigned int SCALE_WIDTH = (1 << SCALE_BIT_WIDTH), SCALE_HEIGHT = (1 << SCALE_BIT_HEIGHT);
	int fScalex = (int)((float)src_width / (float)dst_width * SCALE_WIDTH);
	int fScaley = (int)((float)src_height / (float)dst_height * SCALE_HEIGHT);
	int iStep = (int)(((dst_width-1) / src_width) + 1);		   // for boundary protection
	int jStep = (int)(((dst_height-1) / src_height)  + 1);		// for boundary protection
	long long x;
	int fYInverse, fXInverse;
    
	unsigned char* pbyDst;// = dst;
	unsigned char* pbyLineDst = dst;
    
	int i, j, k;
    
	fYInverse = 0;
	for( i = 0; i < dst_height - jStep; i++)
	{
		int yy = fYInverse >> SCALE_BIT_HEIGHT;
		int fv = fYInverse & (SCALE_HEIGHT - 1);
        
		unsigned char* pBySrc = src + yy * src_stride;
        
		pbyDst = pbyLineDst;
		fXInverse = 0;
		for( j = 0; j < dst_width - iStep; j++)
		{
			int xx = fXInverse >> SCALE_BIT_WIDTH;
			int fu = fXInverse & (SCALE_WIDTH - 1);
            
			unsigned char* pbyCurrent = pBySrc + xx;
			unsigned char a, b, c, d;
            
			a = *pbyCurrent;
			b = *(pbyCurrent + 1 );
			c = *(pbyCurrent + src_stride);
			d = *(pbyCurrent + src_stride + 1 );
            
			x  = (((int)(SCALE_WIDTH - 1 - fu))*(SCALE_HEIGHT - 1 - fv) >> SCALE_BIT_WIDTH) * a;
			x += (((int)(fu))*(SCALE_HEIGHT - 1 - fv) >> SCALE_BIT_WIDTH) * b;
			x += (((int)(SCALE_WIDTH - 1 - fu))*(fv) >> SCALE_BIT_WIDTH) * c;
			x += (((int)(fu))*(fv) >> SCALE_BIT_WIDTH) * d;
			x >>= (SCALE_BIT_HEIGHT - 1);
			x += 1;
			x >>= 1;
			//x = (((__int64)(SCALE_BIG - 1 - fu))*(SCALE_BIG - 1 - fv)*a + ((__int64)fu)*(SCALE_BIG - 1 -fv)*b + ((__int64)(SCALE_BIG - 1 -fu))*fv*c +
			//		 ((__int64)fu)*fv*d + (1 << (2*SCALE_BIT_BIG-1)) ) >> (2*SCALE_BIT_BIG);
			x = x < 0 ? 0 : x > 255 ? 255 : x;
			*pbyDst++ = (unsigned char)x;
            
			fXInverse += fScalex;
		}
		// horizontal boundary value assignment
		for ( k = 0 ; k < iStep; k++)
		{
			int xx = fXInverse >> SCALE_BIT_WIDTH;
			int fu = fXInverse & (SCALE_WIDTH - 1);
            
			unsigned char* pbyCurrent = pBySrc + xx;
			unsigned char a, b, c, d;
            
			int nx = (xx+1)<=(src_width - 1);
			//int ny = (yy+1)<=(src_height - 1);
            
			a = *pbyCurrent;
			b = *(pbyCurrent  + nx);
			c = *(pbyCurrent + src_stride);
			d = *(pbyCurrent + src_stride +nx );
            
			x  = (((int)(SCALE_WIDTH - 1 - fu))*(SCALE_HEIGHT - 1 - fv) >> SCALE_BIT_WIDTH) * a;
			x += (((int)(fu))*(SCALE_HEIGHT - 1 - fv) >> SCALE_BIT_WIDTH) * b;
			x += (((int)(SCALE_WIDTH - 1 - fu))*(fv) >> SCALE_BIT_WIDTH) * c;
			x += (((int)(fu))*(fv) >> SCALE_BIT_WIDTH) * d;
			x >>= (SCALE_BIT_HEIGHT - 1);
			x += 1;
			x >>= 1;
			x = x < 0 ? 0 : x > 255 ? 255 : x;
			*pbyDst++ = (unsigned char)x;
            
			fXInverse += fScalex;
		}
		pbyLineDst += dst_stride;
		fYInverse += fScaley;
	}
    
    
	// vertical boundary value assignment
	for( i = 0; i < jStep; i++)
	{
		int yy = fYInverse >> SCALE_BIT_HEIGHT;
		int fv = fYInverse & (SCALE_HEIGHT - 1);
		unsigned char* pBySrc = src + yy * src_stride;
        
		int ny = (yy+1)<=(src_height - 1);
		pbyDst = pbyLineDst;
		
		fXInverse = 0;
		for( j = 0; j < dst_width; j++)
		{
			int xx = fXInverse >> SCALE_BIT_WIDTH;
			int fu = fXInverse & (SCALE_WIDTH - 1);
            
			unsigned char* pbyCurrent = pBySrc + xx;
			unsigned char a, b, c, d;
            
			int nx = (xx+1)<=(src_width - 1);
            
			a = *pbyCurrent;
			b = *(pbyCurrent  + nx);
			c = *(pbyCurrent + ny*src_stride);
			d = *(pbyCurrent + ny*src_stride +nx );
            
			x  = (((int)(SCALE_WIDTH - 1 - fu))*(SCALE_HEIGHT - 1 - fv) >> SCALE_BIT_WIDTH) * a;
			x += (((int)(fu))*(SCALE_HEIGHT - 1 - fv) >> SCALE_BIT_WIDTH) * b;
			x += (((int)(SCALE_WIDTH - 1 - fu))*(fv) >> SCALE_BIT_WIDTH) * c;
			x += (((int)(fu))*(fv) >> SCALE_BIT_WIDTH) * d;
			x >>= (SCALE_BIT_HEIGHT - 1);
			x += 1;
			x >>= 1;
			x = x < 0 ? 0 : x > 255 ? 255 : x;
			*pbyDst++ = (unsigned char)x;
            
			fXInverse += fScalex;
		}
		pbyLineDst += dst_stride;
		fYInverse += fScaley;
	}
    
}

void upsample(unsigned char *y_src, unsigned char *u_src, unsigned char *v_src, unsigned char *y_dst, unsigned char *u_dst, unsigned char *v_dst, int w_src, int h_src, int w_dst, int h_dst)
{
    int w_src_c = w_src >> 1;
    int h_src_c = h_src >> 1;
    int w_dst_c = w_dst >> 1;
    int h_dst_c = h_dst >> 1;
    general_ds_bilinear_fast_c(y_dst, w_dst, w_dst, h_dst, y_src, w_src, w_src, h_src);
    general_ds_bilinear_accurate_c(u_dst, w_dst_c, w_dst_c, h_dst_c, u_src, w_src_c, w_src_c, h_src_c);
    general_ds_bilinear_accurate_c(v_dst, w_dst_c, w_dst_c, h_dst_c, v_src, w_src_c, w_src_c, h_src_c);
}

int main(int argc, const char **argv)
{
    if (argc < 8)
    {
        printf("[video quality] wrong parameter numbers\n");
        printf("[video quality] Usage:VQ_Testtool dst_file_name src_file_name width height");
        exit(1);
    }
    FILE *dst_file,*src_file;
    dst_file = fopen(argv[1], "rb");
    if (dst_file == NULL)
    {
        printf("[video quality] failed to open file %s\n",argv[1]);
        exit(1);
    }
    src_file = fopen(argv[2], "rb");
    if (src_file == NULL)
    {
        printf("[video quality] failed to open file %s\n",argv[2]);
        exit(1);
    }

    int width = atoi(argv[3]);
    int height = atoi(argv[4]);
    int psnr_received_th = atoi(argv[5]);
    int psnr_average_th = atoi(argv[6]);
    int loss_rate_th = atoi(argv[7]);
    const int frame_size = width*height;
    const int src_frame_size = frame_size;
    int dst_frame_size, dst_width, dst_height;
    long src_file_size = 0;
    fseek(src_file,0,SEEK_END);
    src_file_size = ftell(src_file);
    fseek(src_file,0,SEEK_SET);

    unsigned char *rgb,*yuyv,*y1,*u1,*v1,*y2,*u2,*v2,*y3,*u3,*v3;
    rgb = (unsigned char*)malloc(frame_size*3);
    yuyv = (unsigned char*)malloc(frame_size*2);
    y1 = (unsigned char*)malloc(frame_size);
    u1 = (unsigned char*)malloc(frame_size/4);
    v1 = (unsigned char*)malloc(frame_size/4);
    y2 = (unsigned char*)malloc(frame_size);
    u2 = (unsigned char*)malloc(frame_size/4);
    v2 = (unsigned char*)malloc(frame_size/4);
    y3 = (unsigned char*)malloc(frame_size);
    u3 = (unsigned char*)malloc(frame_size/4);
    v3 = (unsigned char*)malloc(frame_size/4);

    int frame_num_in,frame_num_out;
    frame_num_in = frame_num_out = 0;

	int ts_in, ts_out, ts = 0;
	double psnr_received_frame, psnr_lost_frame, psnr_average;
    psnr_received_frame = psnr_lost_frame = psnr_average = 0;
    bool bSendFlag = false;

    SFrameHeader header;
    WmeVideoRawType dst_type = WmeVideoUnknown;

#ifdef DUMP_FILE
    FILE * dump_file;
    dump_file = fopen("dump.yuv","wb");
#endif

    while (1)
    {
        ts += 100;
        ts_in = ts * 90;
        fread(&header.iHeaderSize, 1, sizeof(unsigned int), dst_file);
        fread(&header.eMType, 1, sizeof(WmeMediaFormatType), dst_file);
        fread(&header.iFrameSize, 1, sizeof(unsigned int), dst_file);
        fread(&header.sVideoRawFormat, 1, sizeof(WmeVideoRawFormat), dst_file);
        ts_out = header.sVideoRawFormat.uTimestamp;
        dst_width = header.sVideoRawFormat.iWidth;
        dst_height = header.sVideoRawFormat.iHeight;
        dst_frame_size = dst_width * dst_height;
        dst_type = header.sVideoRawFormat.eRawType;

        while (ts_in < ts_out)
        {
            fread(y2, 1, src_frame_size, src_file);
            fread(u2, 1, src_frame_size/4, src_file);
            fread(v2, 1, src_frame_size/4, src_file);
            if (ftell(src_file) == src_file_size)
            {
                fseek(src_file,0,SEEK_SET);
            }
            if (bSendFlag)
            {
                frame_num_in++;
                psnr_lost_frame += psnr_calculation(y1, u1, v1, y2, u2, v2, frame_size);
            }
            ts += 100;
            ts_in = ts * 90;
        }

        fread(y2, 1, src_frame_size, src_file);
        fread(u2, 1, src_frame_size/4, src_file);
        fread(v2, 1, src_frame_size/4, src_file);
        frame_num_in++;
        if (ftell(src_file) == src_file_size)
        {
            fseek(src_file,0,SEEK_SET);
        }

        if (WmeYUY2 == dst_type)
        {
            if (dst_frame_size == src_frame_size)
            {
                fread(yuyv, 1, dst_frame_size*2, dst_file);
                yuy2_to_i420(yuyv, y1, u1, v1, width, height);
            }
            else {
                fread(yuyv, 1, dst_frame_size*2, dst_file);
                yuy2_to_i420(yuyv, y3, u3, v3, dst_width, dst_height);
                upsample(y3, u3, v3, y1, u1, v1, dst_width, dst_height, width, height);
            }

            frame_num_out++;
            bSendFlag = true;
        }
        else if (WmeRGB24 == dst_type)
        {
            if (dst_frame_size == src_frame_size)
            {
                fread(rgb, 1, dst_frame_size*3, dst_file);
                rgb_to_i420(rgb, width*3, y1, u1, v1,
                            width, width>>1, width, height);
            }
            else {
                fread(rgb, 1, dst_frame_size*3, dst_file);
                rgb_to_i420(rgb, width*3, y3, u3, v3,
                            dst_width, dst_width>>1, dst_width, dst_height);
                upsample(y3, u3, v3, y1, u1, v1, dst_width, dst_height, width, height);
            }
            frame_num_out++;
            bSendFlag = true;
        }
        else if (WmeI420 == dst_type)
        {
            if (dst_frame_size == src_frame_size)
            {
                fread(y1, 1, dst_frame_size, dst_file);
                fread(u1, 1, dst_frame_size/4, dst_file);
                fread(v1, 1, dst_frame_size/4, dst_file);
            }
            else
            {
                fread(y3, 1, dst_frame_size, dst_file);
                fread(u3, 1, dst_frame_size/4, dst_file);
                fread(v3, 1, dst_frame_size/4, dst_file);
                upsample(y3, u3, v3, y1, u1, v1, dst_width, dst_height, width, height);
            }
            frame_num_out++;
            bSendFlag = true;
        }
        else
        {
            printf("[video quality] wrong file type %s\n",argv[6]);
            exit(1);
        }
#ifdef DEBUG_LOG
        printf("[video quality] type:%d width:%d height:%d ts_in:%d ts_out:%d\n",dst_type,dst_width,dst_height,ts_in,ts_out);
#endif
#ifdef DUMP_FILE
        fwrite(y1,1,frame_size,dump_file);
        fwrite(u1,1,frame_size/4,dump_file);
        fwrite(v1,1,frame_size/4,dump_file);
#endif
        psnr_received_frame += psnr_calculation(y1, u1, v1, y2, u2, v2, frame_size);

        if (feof(dst_file))
            break;
    }
    psnr_average = (psnr_received_frame + psnr_lost_frame) / frame_num_in;
    psnr_received_frame /= frame_num_out;
    psnr_lost_frame /= frame_num_in - frame_num_out;
    printf("[video quality] Send frame %d. Received frame %d.\n", frame_num_in, frame_num_out);
    printf("[video quality] PSNR received frame is %f, PSNR lost frame is %f, Average is %f.\n",psnr_received_frame, psnr_lost_frame, psnr_average);

    free(yuyv);
    free(y1);
    free(u1);
    free(v1);
    free(y2);
    free(u2);
    free(v2);
    free(y3);
    free(u3);
    free(v3);

    fclose(dst_file);
    fclose(src_file);
#ifdef DUMP_FILE
    fclose(dump_file);
#endif

    if (psnr_received_frame < psnr_received_th)
    {
        printf("[video quality] PSNR is too low!\n");
        return 1;
    }
    if (psnr_average < psnr_average_th)
    {
        printf("[video quality] PSNR with frame loss is too low!\n");
        return 1;
    }
	if (1.0 * frame_num_out / frame_num_in < loss_rate_th / 100.0f)
    {
        printf("[video quality] Frame loss rate is too high!\n");
        return 1;
    }

    return 0;
}
