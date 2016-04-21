// Copyright (C) 1999-2005 Open Source Telecom Corporation.
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however    
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.    
//
// This exception applies only to the code released under the name GNU
// ccAudio.  If you copy code from other releases into a copy of GNU
// ccAudio, as the General Public License permits, the exception does
// not apply to the code that you add in this way.  To avoid misleading
// anyone as to the status of such modified files, you must delete
// this exception notice from them.
//
// If you write modifications of your own for GNU ccAudio, it is your choice
// whether to permit this exception to apply to your modifications.
// If you do not wish that, delete this exception notice.
//

/**
 * @file audio2.h
 * @short Framework for portable audio processing and file handling classes.
 **/


#ifndef	CCXX_AUDIO_H_
#define	CCXX_AUDIO_H_

#ifndef	W32
#if defined(_WIN32) && defined(_MSC_VER)
#pragma warning(disable: 4996)
#define	W32
#endif
#if defined(__BORLANDC__) && defined(__Windows)
#define	W32
#endif
#endif
#ifdef	W32
#include <windows.h>
#ifndef	ssize_t
#define	ssize_t	int
#endif
#else
#include <cstddef>
#include <cstdlib>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <ctime>

#ifdef	W32
	typedef	short	Sample;
	typedef	short	*Linear;
	typedef	short	Level;
	typedef	DWORD	timeout_t;
	typedef	WORD	snd16_t;
	typedef	DWORD	snd32_t;
#else
	typedef	int16_t snd16_t;
	typedef	int32_t	snd32_t;
	typedef	int16_t	Level;
	typedef	int16_t	Sample;
	typedef	int16_t	*Linear;
	typedef	unsigned long	timeout_t;
#endif



#define	AUDIO_SIGNED_LINEAR_RAW	1
#define	AUDIO_LINEAR_CONVERSION 1
#define	AUDIO_CODEC_MODULES	1
#define	AUDIO_LINEAR_FRAMING	1
#define	AUDIO_NATIVE_METHODS	1



typedef struct
{
float v2;
	float v3;
	float fac;
} goertzel_state_t;

typedef struct
{
	int hit1;
	int hit2;
	int hit3;
	int hit4;
	int mhit;

	goertzel_state_t row_out[4];
	goertzel_state_t col_out[4];
	goertzel_state_t row_out2nd[4];
	goertzel_state_t col_out2nd[4];
	goertzel_state_t fax_tone;
	goertzel_state_t fax_tone2nd;
	float energy;

	int current_sample;
	char digits[129];
	int	detected_digits_position[129];
	int current_digits;
	int detected_digits;
	int lost_digits;
	int digit_hits[16];
	int fax_hits;
} dtmf_detect_state_t;

typedef struct
{
	float fac;
} tone_detection_descriptor_t;

enum	Rate
{
	rateUnknown,
	rate6khz = 6000,
	rate8khz = 8000,
	rate16khz = 16000,
	rate44khz = 44100
};

typedef	enum Rate Rate;

/**
 * File processing mode, whether to skip missing files, etc.
 */
enum	Mode
{
	modeRead,
	modeReadAny,
	modeReadOne,
	modeWrite,
	modeCache,
	modeInfo,
	modeFeed,

	modeAppend,	// app specific placeholders...
	modeCreate
};

typedef enum Mode Mode;


/**
 * Audio container file format.
 */
enum Format
{
	raw,
	snd,
	riff,
	mpeg,
	wave
};
typedef enum Format Format;


/**
 * Audio error conditions.
 */
enum Error
{
	errSuccess = 0,
	errReadLast,
	errNotOpened,
	errEndOfFile,
	errStartOfFile,
	errRateInvalid,
	errEncodingInvalid,
	errReadInterrupt,
	errWriteInterrupt,
	errReadFailure,
	errWriteFailure,
	errReadIncomplete,
	errWriteIncomplete,
	errRequestInvalid,
	errTOCFailed,
	errStatFailed,
	errInvalidTrack,
	errPlaybackFailed,
	errNotPlaying,
	errNoCodec
};
typedef enum Error Error;


/**
 * DTMFDetect is used for detecting DTMF tones in a stream of audio.
 * It currently only supports 8000Hz input.
 */
class  DTMFDetect
{
public:
        DTMFDetect();
        ~DTMFDetect();

	/**
	 * This routine is used to push linear audio data into the
	 * dtmf tone detection analysizer.  It may be called multiple
	 * times and results fetched later.
	 *
	 * @param buffer of audio data in native machine endian to analysize.
	 * @param count of samples to analysize from buffer.
	 */
        int putSamples(Linear buffer, int count);

	/**
	 * Copy detected dtmf results into a data buffer.
	 *
	 * @param data buffer to copy into.
	 * @param size of data buffer to copy into.
	 */
        int getResult(char *data,int*position,int size);

protected:
        void goertzelInit(goertzel_state_t *s, tone_detection_descriptor_t *t);
        void goertzelUpdate(goertzel_state_t *s, Sample x[], int samples);
        float goertzelResult(goertzel_state_t *s);

private:
        dtmf_detect_state_t *state;
        tone_detection_descriptor_t dtmf_detect_row[4];
        tone_detection_descriptor_t dtmf_detect_col[4];
        tone_detection_descriptor_t dtmf_detect_row_2nd[4];
        tone_detection_descriptor_t dtmf_detect_col_2nd[4];
        tone_detection_descriptor_t fax_detect;
        tone_detection_descriptor_t fax_detect_2nd;
};


#endif

