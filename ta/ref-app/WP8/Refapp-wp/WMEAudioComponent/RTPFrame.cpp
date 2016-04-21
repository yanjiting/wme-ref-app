
#ifdef WIN32
#if _MSC_VER > 1200
#define POINTER_64 __ptr64
//#define _WIN32_WINNT 0x0600
#else
#define _WIN32_WINNT 0x0400
#endif

#if _MSC_VER <= 1200
#include <windows.h>
#include <winsock2.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif

#endif //WIN32

#include "CmBase.h"
#include "RTPFrame.h"



//#ifdef _GIPS_NEED_BUFFER 


RTPFrame::RTPFrame()
{ 
    m_frame = new BYTE[RTP_FRAME_MAX_LEN];

    //memset(m_frame, 0, RTP_FRAME_MAX_LEN);
#if !defined(MACOS)
    memset(m_frame, RTP_FRAME_MAX_LEN, 0);
#else
    cs_memset_s(m_frame, RTP_FRAME_MAX_LEN, 0);
#endif
    m_iBufSize = RTP_FRAME_MAX_LEN;
    m_iPayloadSize = 0;
    //SetVersion(2);

}

void RTPFrame::SetExtension(BOOL ext)
{
  if (ext)
    m_frame[0] |= 0x10;
  else
    m_frame[0] &= 0xef;
}


void RTPFrame::SetMarker(BOOL m)
{
  if (m)
    m_frame[1] |= 0x80;
  else
    m_frame[1] &= 0x7f;
}


// void RTPFrame::SetPayloadType(unsigned char t)
// {
//   ASSERT(t <= 0x7f); // InvalidParameter
// 
//   m_frame[1] &= 0x80;
//   m_frame[1] |= t;
// }


DWORD RTPFrame::GetContribSource(int idx) 
{
//  ASSERT(idx < GetContribSrcCount()); // InvalidParameter

  DWORD	csc = ((DWORD *)&m_frame[12])[idx];
  Swap(&csc, sizeof(csc));

  return csc;
}


void RTPFrame::SetContribSource(int idx, DWORD src)
{
//  ASSERT(idx <= 15); // InvalidParameter

    if (idx >= GetContribSrcCount())
    {
        BYTE * oldPayload = GetPayloadPtr();
        m_frame[0] &= 0xf0;
        m_frame[0] |= idx+1;
        //SetSize(GetHeaderSize()+m_iPayloadSize);
        //memmove(GetPayloadPtr(), oldPayload, m_iPayloadSize);
        memmove_s(GetPayloadPtr(), GetPayloadSize(), oldPayload, m_iPayloadSize);
    }

    Swap(&src, sizeof(DWORD));
    ((DWORD *)&m_frame[12])[idx] = src;
}

//
//void RTPFrame::SetVersion(unsigned i)
//{
//  tag_rtphdr* rtphdr = reinterpret_cast<tag_rtphdr*>(m_frame);
//  rtphdr->rh_flags.version = i;
//
//}

unsigned  RTPFrame::GetVersion()  
{
    return (m_frame[0]>>6)&3; 
}

RTPFrame& RTPFrame::operator= (RTPFrame& frame)
{
	if(this == &frame)
		return *this;

	delete m_frame;
	m_iBufSize = frame.GetPayloadSize() + 12;
	m_frame = new BYTE[m_iBufSize+1];
	
	if(m_iBufSize >0 && m_frame)
		memcpy_s((void*)m_frame, m_iBufSize + 1, frame.GetBuffer(), m_iBufSize);

	m_iPayloadSize = frame.GetPayloadSize();
	SetPayloadSize(m_iPayloadSize);
	
	return *this;
}

//#endif //#ifdef _GIPS_NEED_BUFFER 
