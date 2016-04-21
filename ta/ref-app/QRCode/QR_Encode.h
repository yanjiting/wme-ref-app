#if !defined _QR_ENCODE_H_
#define _QR_ENCODE_H_

#include <sstream>

#ifdef __MACOS__
typedef unsigned char 	    BYTE;
typedef const char* 	    LPCSTR;
typedef unsigned short      WORD;
#elif defined(WIN32)
#include <windows.h>
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////

#define QRCODE_DEFAULT_LEVEL			1
#define QRCODE_DEFAULT_VERSION			0
#define QRCODE_DEFAULT_MASKINGNO		-1

#define QR_LEVEL_L	0
#define QR_LEVEL_M	1
#define QR_LEVEL_Q	2
#define QR_LEVEL_H	3

#define QR_MODE_NUMERAL		0
#define QR_MODE_ALPHABET	1
#define QR_MODE_8BIT		2
#define QR_MODE_KANJI		3

#define QR_VRESION_S	0 
#define QR_VRESION_M	1 
#define QR_VRESION_L	2 

#define MAX_ALLCODEWORD	 3706 
#define MAX_DATACODEWORD 2956 
#define MAX_CODEBLOCK	  153 
#define MAX_MODULESIZE	  177 

#define QR_MARGIN	4


/////////////////////////////////////////////////////////////////////////////
typedef struct tagRS_BLOCKINFO
{
	int ncRSBlock;		
	int ncAllCodeWord;
	int ncDataCodeWord;	

} RS_BLOCKINFO, *LPRS_BLOCKINFO;

typedef struct tagQR_VERSIONINFO
{
	int nVersionNo;	 
	int ncAllCodeWord; 

	// (0 = L, 1 = M, 2 = Q, 3 = H) 
	int ncDataCodeWord[4];

	int ncAlignPoint;
	int nAlignPoint[6];	

	RS_BLOCKINFO RS_BlockInfo1[4]; 
	RS_BLOCKINFO RS_BlockInfo2[4];

} QR_VERSIONINFO, *LPQR_VERSIONINFO;

class CQR_Encode
{
public:
	CQR_Encode();
	virtual ~CQR_Encode();

public:
	virtual void ShowImage(void* wnd) {}
    virtual void MakeImage() {}
	bool EncodeData(int nLevel, int nVersion, bool bAutoExtent, int nMaskingNo, LPCSTR lpsSource, int ncSource = 0);

private:
	int  GetEncodeVersion(int nVersion, LPCSTR lpsSource, int ncLength);
	bool EncodeSourceData(LPCSTR lpsSource, int ncLength, int nVerGroup);

	int  GetBitLength(BYTE nMode, int ncData, int nVerGroup);
	int  SetBitStream(int nIndex, unsigned short wData, int ncData);

	bool IsNumeralData(unsigned char c);
	bool IsAlphabetData(unsigned char c);
	bool IsKanjiData(unsigned char c1, unsigned char c2);

	BYTE AlphabetToBinaly(unsigned char c);
	unsigned short KanjiToBinaly(WORD wc);

	void GetRSCodeWord(BYTE* lpbyRSWork, int ncDataCodeWord, int ncRSCodeWord);

	void FormatModule();

	void SetFunctionModule();
	void SetFinderPattern(int x, int y);
	void SetAlignmentPattern(int x, int y);
	void SetVersionPattern();
	void SetCodeWordPattern();
	void SetMaskingPattern(int nPatternNo);
	void SetFormatInfoPattern(int nPatternNo);
	int  CountPenalty();

    
public:
    int  m_nLevel;
	int  m_nVersion;
	int  m_nMaskingNo;
	int  m_nSymbleSize;
	BYTE m_byModuleData[MAX_MODULESIZE][MAX_MODULESIZE];


private:
	int  m_ncDataCodeWordBit; 
	BYTE m_byDataCodeWord[MAX_DATACODEWORD];

	int  m_ncDataBlock;
	BYTE m_byBlockMode[MAX_DATACODEWORD];
	int  m_nBlockLength[MAX_DATACODEWORD];

	int  m_ncAllCodeWord; 
	BYTE m_byAllCodeWord[MAX_ALLCODEWORD]; 
	BYTE m_byRSWork[MAX_CODEBLOCK]; 


	std::string   m_strPreSouce;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined _QR_ENCODE_H_
