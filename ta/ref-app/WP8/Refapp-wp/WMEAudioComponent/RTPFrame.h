#ifndef _RTP_FRAME_H__
#define _RTP_FRAME_H__




#define RTP_FRAME_MAX_LEN	65535
#define RTP_HEAD_SIZE           12



class RTPFrame  
{
private:
	RTPFrame(const RTPFrame&);

public:

	RTPFrame(); 
	RTPFrame(BYTE * buffer, int len) { m_frame = new BYTE[len]; m_iBufSize = len ; FromBuffer_Copy(buffer, len) ; };
	RTPFrame(int len) { m_frame = new BYTE[len+1]; m_iBufSize = len+1;m_iPayloadSize = 0;};
	virtual ~RTPFrame() { delete[] m_frame; };

	void FromBuffer(BYTE * buffer, int len)
	{ /*ASSERT(RTP_FRAME_MAX_LENGTH>len);*/
        //memmove(m_frame, buffer, len);
        memmove_s(m_frame, m_iBufSize, buffer, len);
        m_iPayloadSize = len - GetHeaderSize();
    }

	void FromBuffer_Copy(BYTE * buffer, int len)
	{ /*ASSERT(RTP_FRAME_MAX_LENGTH>len);*/
        //memcpy(m_frame,buffer,len);
        memcpy_s(m_frame, m_iBufSize, buffer, len);
    }


    unsigned GetVersion()  ;
    //void SetVersion(unsigned i);

    BOOL GetExtension()    { return (m_frame[0]&0x10) != 0; }
    void SetExtension(BOOL ext);

    BOOL GetMarker()  { return (m_frame[1]&0x80) != 0; }
    void SetMarker(BOOL m);

    unsigned GetPayloadType()  { return  m_frame[1]&0x7f; };
	static unsigned GetPayloadType(const char* frame)  { return  frame[1] & 0x7f; };
    void     SetPayloadType(unsigned char t) {  /*ASSERT(t <= 0x7f);*/ 	m_frame[1] &= 0x80;  m_frame[1] |= t;};

    WORD GetSequenceNumber()  { short t = *(unsigned short*)&m_frame[2]; Swap(&t, sizeof(t)); return t; }
    void SetSequenceNumber(WORD n) { Swap(&n, sizeof(WORD)); *(unsigned short *)&m_frame[2] = n; }
	static WORD GetSequenceNumber(const char* frame)  
	{
		if ( !frame ) return 0;
		short t = *(unsigned short*)&frame[2]; Swap(&t, sizeof(t)); return t; 
	}

    DWORD GetTimestamp()   { unsigned int ts = *(unsigned int *)&m_frame[4]; Swap(&ts, sizeof(ts)); return ts; }
    void  SetTimestamp(DWORD t) { Swap(&t, sizeof(DWORD)); *(unsigned int *)&m_frame[4] = t; }
	static DWORD GetTimestamp(const char* frame) 
	{
		if ( !frame ) return 0;
		unsigned int ts = *(unsigned int *)&frame[4]; Swap(&ts, sizeof(ts)); return ts;
	}

    DWORD GetSyncSource()   { unsigned int ss = *(unsigned int *)&m_frame[8]; Swap(&ss, sizeof(ss)); return ss; }
    void  SetSyncSource(DWORD s) { Swap(&s, sizeof(DWORD)); *(unsigned int *)&m_frame[8] = s; }

    int GetContribSrcCount()  { return m_frame[0]&0xf; }
    DWORD  GetContribSource(int idx) ;
    void   SetContribSource(int idx, DWORD src);

	static DWORD GetContribSource(const char* frame, int idx)
	{
		DWORD	csc = ((DWORD *)&frame[12])[idx];
		Swap(&csc, sizeof(csc));
		return csc;
	}

	static void SetContribSource(char* frame, int idx, DWORD csrc) { Swap(&csrc, sizeof(csrc)); ((DWORD *)&frame[12])[idx] = csrc; }

    int GetHeaderSize()      { return 12 + 4*GetContribSrcCount(); }

    int GetPayloadSize()  { return m_iPayloadSize; }
    void   SetPayloadSize(int size) { m_iPayloadSize = size; };
    BYTE * GetPayloadPtr()      { return (BYTE *)(m_frame+GetHeaderSize()); }

	BYTE * GetBuffer() { return m_frame; }
	int GetBufferSize() { return m_iBufSize; }
	int GetBufferSize_Active() { return m_iPayloadSize+GetHeaderSize(); }
	RTPFrame& operator=(RTPFrame& frame);
private:
	static void Swap(void* pDataIn, int size)
	{
		static int s_nDummy = 1;
		
		if(*(char*)(&s_nDummy) != 0)
		{
			if ( size == 4 )
			{
				BYTE* pData = (BYTE*)pDataIn;
				BYTE tmp ;
				tmp = pData[0];
				pData[0] = pData[3];
				pData[3] = tmp;
				tmp = pData[1];
				pData[1] = pData[2];
				pData[2] = tmp;
			}
			else if ( size == 2)
			{
				BYTE* pData = (BYTE*)pDataIn;
				BYTE tmp ;
				tmp = pData[0];
				pData[0] = pData[1];
				pData[1] = tmp;
			}
			else
			{	
				unsigned char* pch = (unsigned char*)pDataIn;
				for(int i = 0; i < size / 2; i++)
				{
					unsigned char chTemp;
					chTemp = pch[i];
					pch[i] = pch[size-1-i];
					pch[size-1-i] = chTemp;
				}
			}
		}
	};

private:

	BYTE *		m_frame;
	int			m_iPayloadSize;
	int			m_iBufSize;
};
//#endif //#ifdef _GIPS_NEED_BUFFER 


#endif // !defined(__RTPFRAME_H__)

