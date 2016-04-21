//
//  FileMediaConnection.cpp
//  MediaEngineTestApp
//
//  Created by Sand Pei on 14-6-26.
//  Copyright (c) 2014年 video. All rights reserved.
//
#include "WmeCommonTypes.h"
#include "FileMediaConnection.h"
#include "timer.h"

#define USE_DEBUG_PRINTF_TRACE   0

//////////////////////////////////////////////////////////////////////////
#pragma pack(1)
/*
 * The first record in the file contains saved values for some
 * of the flags used in the printout phases of tcpdump.
 * Many fields here are 32 bit ints so compilers won't insert unwanted
 * padding; these files need to be interchangeable across architectures.
 */
typedef struct TCPDUMP_FILE_HEADER_tag {
	unsigned int		magic; //0xa1b2c3d4
	unsigned short	version_major;
	unsigned short	version_minor;
	int				thiszone; /* gmt to local correction */
	unsigned int		sigfigs; /* accuracy of timestamps */
	unsigned int		snaplen; /* max length saved portion of each pkt */
	unsigned int		linktype; /* data link type (DLT_*)  0x01000000 == ÒÔÌ«Íø*/
}TCPDUMP_FILE_HEADER;

/*
 * This is a timeval as stored in disk in a dumpfile.
 * It has to use the same types everywhere, independent of the actual
 * `struct timeval'  */

typedef struct TCPDUMIMEVAL_tag {
	int tv_sec;   /* seconds */
	int tv_usec;   /* microseconds */
}TCPDUMIMEVAL;

/*
 * How a `pcap_pkthdr' is actually stored in the dumpfile.
 */

typedef struct TCPDUMP_SF_PKTHDR_tag {
	TCPDUMIMEVAL ts; /* time stamp */
	unsigned int caplen;   /* length of portion present */
	unsigned int len;   /* length this packet (off wire) */
}TCPDUMP_SF_PKTHDR;

//ÒÔÌ«Íø°üÍ·14×Ö½Ú
typedef struct ethdr
{
	unsigned char        destination_mac[6];
	unsigned char        source_mac[6];
	unsigned short       type;       //IP-0x0800
}ET_HEADER,*PETHDR;

//Internet Protocol
typedef struct IPHDR_tag
{
	unsigned char   h:4;//Ê×²¿³¤¶È4bit = 5
	unsigned char   v:4;//°æ±¾4bit = 4
	unsigned char        TOS;    // Type Of Service
	unsigned short       TotLen; // Total Length              ×Ü³¤¶È2×Ö½Ú£¬°üÀ¨Êý¾ÝºÍ±¨Í·
	unsigned short       ID;      // Identification
	unsigned short       FlagOff;   // Flags and Fragment Offset
	unsigned char        TTL;      // Time To Live
	/*
     0 (0x00) IPv6 Hop-by-Hop Option
     1 (0x01) ICMP protocol
     2 (0x02) IGMP protocol
     4 (0x04) IP over IP
     6 (0x06) TCP protocol
     17 (0x11) UDP protocol
     41 (0x29) IPv6 protocol*/
	unsigned char        Protocol;   // Protocol               ICMP-1,TCP-6,UDP-17
	unsigned short       Checksum;      // Checksum
	unsigned int        source_ip;   // Source IP
	unsigned int        destination_ip;  // Destination IP
}IP_HEADER, *PIP_HEADER;


//Transmission Control Protocol
typedef struct TCPHDR_tag
{
	unsigned short       source_port;
	unsigned short       destination_port;
	unsigned int        sequence;
	unsigned int        ack_sequence;
	unsigned char        header_reserve:4;              //Ê×²¿³¤¶È,Ö»ÓÃ¸ß4Î»Í¨³£Îª5
	unsigned char        header_length:4;              //Ê×²¿³¤¶È,Ö»ÓÃ¸ß4Î»Í¨³£Îª5
	unsigned char		flags;                                  //±êÖ¾Î»  X | X | URG | ACK | PSH | RST | SYN | FIN
	unsigned short       window_size;
	unsigned short       checksum;
	unsigned short       exigency_pointer;
}TCP_HEADER;

typedef struct UDPHDR_tag
{
	unsigned short src_port;
	unsigned short dst_port;
	unsigned short length;
	unsigned short chksum;
} UDP_HEADER;


typedef struct{
	union{
		struct{
			unsigned short   ph_pt:7;		// payload type
			unsigned short   ph_m:1;		// marker bit
			unsigned short   ph_cc:4;		// CSRC count
			unsigned short   ph_x:1;		// header extension flag
			unsigned short   ph_p:1;		// padding flag
			unsigned short   ph_v:2;		// packet type
		} u;
		unsigned short m_flags;
	};
	unsigned short   ph_seq;		// sequence number
	unsigned int   ph_ts;			// timestamp
	unsigned int   ph_ssrc;		// synchronization source
} RTP_HEADER;

typedef struct {
	union{
		struct{
			unsigned short pt:8;/* RTCP packet type */
			unsigned short count:5;/* varies by packet type */
			unsigned short p:1;/* padding flag */
			unsigned short v:2;/* protocol version */
		} u;
		unsigned short m_flags;
	};
	unsigned short length; /* pkt len in words, w/o this word */
} RTCP_COMMON_HEADER;

#pragma pack()

//////////////////////////////////////////////////////////////////////////
int  CreateMediaConnection(const char *fileName, IMediaConnection **ppConn)
{
    CFileMediaConnection *fileConn = new CFileMediaConnection(fileName);
	if(NULL == fileConn)
	{
		return WME_E_OUTOFMEMORY;
	}
    
    *ppConn = fileConn;
    
	return WME_S_OK;
}

//////////////////////////////////////////////////////////////////////////
CFileMediaConnection::CFileMediaConnection(const char *fileName)
: m_pSink(NULL), m_strFileName(fileName), m_pFile(NULL), m_bConnected(false)
, m_uSourceIP(0), m_uDestinationIP(0), m_uSourcePort(0), m_uDestinationPort(0)
, m_iPacketCount(0), m_pBuffer(NULL), m_iBufferLen(0), m_pPacket(NULL), m_iPacketLen(0)
, m_uPacketTime(0), m_uLastPacketTime(0), m_uFirstPacketTime(0)
, m_bDelivered(true), m_uLastDeliverTime(0), m_uFirstDeliverTime(0)
, m_pTimerThread(NULL)
{
}

CFileMediaConnection::~CFileMediaConnection()
{
    CM_INFO_TRACE_THIS("CFileMediaConnection::~CFileMediaConnection");
    Stop();
    
    m_pSink = NULL;
}

void CFileMediaConnection::SetSink(IMediaConnectionSink* pSink, ConnSinkType sinkType)
{
    m_pSink = pSink;
    m_eConnSinkType = sinkType;
}

int CFileMediaConnection::ConnectFile(const char *sourceIP, unsigned short sourcePort, const char *destinationIP, unsigned short destinationPort)
{
    // save socket info
    if(NULL == sourceIP || NULL == destinationIP)
    {
        return WME_E_INVALIDARG;
    }
    
    m_uSourceIP = inet_addr(sourceIP);
    m_uSourcePort = sourcePort;
    m_uDestinationIP = inet_addr(destinationIP);
    m_uDestinationPort = destinationPort;
    CM_INFO_TRACE_THIS("CFileMediaConnection::ConnectFile, file [" << m_strFileName.c_str() << "], sourceIP = " << m_uSourceIP << " [" << sourceIP << "], suorcePort = " << sourcePort << ", destinationIP = " << m_uDestinationIP << " [" << destinationIP << "], destinationPort = " << destinationPort);
#if USE_DEBUG_PRINTF_TRACE
    printf(("Connect file [%s]: sourceIP = %d [%s], suorcePort = %d, destinationIP = %d [%s], destinationPort = %d\n"), m_strFileName.c_str(), m_uSourceIP, sourceIP, sourcePort, m_uDestinationIP, destinationIP, destinationPort);
#endif
    
    int ret = Connect();
    
    if(m_pSink)
    {
        m_pSink->OnConnected(m_bConnected);
    }
    
    return ret;
}

int CFileMediaConnection::Stop()
{
    return DisConnect();
}

int CFileMediaConnection::Connect()
{
    // check file header
    if(m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
    
    FILE *pFile = fopen(m_strFileName.c_str(), "rb+");
    if(NULL == pFile)
    {
        return WME_E_FAIL;
    }
    
    TCPDUMP_FILE_HEADER fheader = {0};
	int r = fread(&fheader,1,sizeof(fheader),pFile);
	fheader.linktype=ntohl(fheader.linktype);
    
	if(fheader.magic != 0xa1b2c3d4)
    {
        CM_ERROR_TRACE_THIS("CFileMediaConnection::Connect, Only support tcpdump file ,fheader.magic(" << fheader.magic << ") != 0xa1b2c3d4");
#if USE_DEBUG_PRINTF_TRACE
		printf(("Only support tcpdump file ,fheader.magic(0x%08x) != 0xa1b2c3d4 \n"),fheader.magic);
#endif
		fclose(pFile);
		return WME_E_FAIL;
	}
    
	if(r != sizeof(fheader) || 0x01000000 != fheader.linktype)
    {
        CM_ERROR_TRACE_THIS("CFileMediaConnection::Connect, Only support the data captured in ethernet II , " << r << "<->" << sizeof(fheader) << ", linktype = " << fheader.linktype);
#if USE_DEBUG_PRINTF_TRACE
        printf(("Only support the data captured in ethernet II , %d<->%lu,linktype=0x%08x\n"),r,sizeof(fheader), fheader.linktype);
		printf(("magic=0x%08x,v_major=%u,v_minor=%u,thiszone=%d,sigfigs=%u,snaplen=%u,linktype=%u\n"),
               fheader.magic,
               fheader.version_major,
               fheader.version_minor,
               fheader.thiszone,
               fheader.sigfigs,
               fheader.snaplen,
               fheader.linktype
               );
#endif
        fclose(pFile);
        return WME_E_FAIL;
    }
    
    m_pFile = pFile;
    
    // schdule timer
    SchduleTimer();
    
    m_bConnected = true;
    
    return WME_S_OK;
}

int CFileMediaConnection::DisConnect()
{
    CM_INFO_TRACE_THIS("CFileMediaConnection::DisConnect");
    m_bConnected = false;
    
    // cancel timer
    CancelTimer();
    
    if(m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
    
    if(m_pBuffer)
    {
        free(m_pBuffer);
        m_pBuffer = NULL;
    }
    
    m_pPacket = NULL;
    
    if(m_pSink)
    {
        m_pSink->OnDisconnected();
    }
    
    return WME_S_OK;
}

bool CFileMediaConnection::IsConnected()
{
    return m_bConnected;
}

int CFileMediaConnection::SchduleTimer()
{
    // schedule timers
	if(CM_OK == CreateUserTaskThread("fmconn", m_pTimerThread) && NULL != m_pTimerThread)
	{
		ICmEventQueue* pEventQueue = m_pTimerThread->GetEventQueue();
		if(NULL != pEventQueue)
		{
			CTimerEvent *pTimerEvent = new CTimerEvent(&m_TimerId, true, this, 0.01);
			if(NULL == pTimerEvent || CM_OK != pEventQueue->SendEvent(pTimerEvent))
			{
				CM_WARNING_TRACE_THIS("CFileMediaConnection::SchduleTimer, post event fail, m_eConnSinkType = " << m_eConnSinkType);
			}
		}
		else
		{
			CM_WARNING_TRACE_THIS("CFileMediaConnection::SchduleTimer, get event queue fail, m_eConnSinkType = " << m_eConnSinkType);
		}
	}
	else
	{
		CM_WARNING_TRACE_THIS("CFileMediaConnection::SchduleTimer, create timer thread fail, m_eConnSinkType = " << m_eConnSinkType);
	}
    
    return WME_S_OK;
}

int CFileMediaConnection::CancelTimer()
{
    // cancel timers
	if(NULL != m_pTimerThread)
	{
		ICmEventQueue* pEventQueue = m_pTimerThread->GetEventQueue();
		if(NULL != pEventQueue)
		{
			CTimerEvent *pTimerEvent = new CTimerEvent(&m_TimerId, false, this);
			if(NULL == pTimerEvent || CM_OK != pEventQueue->SendEvent(pTimerEvent))
			{
				CM_WARNING_TRACE_THIS("CFileMediaConnection::CancelTimer, post event fail, m_eConnSinkType = " << m_eConnSinkType);
			}
		}
        
		m_pTimerThread->Stop();
		m_pTimerThread->Join();
		m_pTimerThread->Destory(CM_OK);
		m_pTimerThread = NULL;
	}
    
    return WME_S_OK;
}

int CFileMediaConnection::ReadPacket()
{
    if(NULL == m_pFile)
    {
        return WME_E_POINTER;
    }
    
    // check deliver flag
    if(false == m_bDelivered)
    {
        return WME_S_OK;
    }
    
    WMERESULT result = WME_S_OK;
    
    // read packet
    int r = 0;
    if(!feof(m_pFile) && !ferror(m_pFile))
    {
        TCPDUMP_SF_PKTHDR pkthdr={0};
        r = fread(&pkthdr,1,sizeof(pkthdr),m_pFile);
        if( r == sizeof(pkthdr) && pkthdr.len > 0)
        {
            if(pkthdr.len > m_iBufferLen)
            {
                if(m_pBuffer)
                {
                    free(m_pBuffer);
                }
                m_iBufferLen = pkthdr.len;
                m_pBuffer = (unsigned char *)malloc(m_iBufferLen);
            }
            r = fread(m_pBuffer,1, pkthdr.len,  m_pFile);
            unsigned int offset = 0;
            ET_HEADER * ethdr = NULL;
            IP_HEADER * iphdr = NULL;
            UDP_HEADER * udphdr = NULL;
            unsigned char * udp_raw = NULL;
            ethdr = (ET_HEADER*)( m_pBuffer + offset); offset += sizeof(ET_HEADER);
            iphdr = (IP_HEADER*)( m_pBuffer + offset); offset += iphdr->h * 4;
            udphdr = (UDP_HEADER*)( m_pBuffer + offset); offset += sizeof(UDP_HEADER) ;
            udp_raw = m_pBuffer + offset;
            iphdr->TotLen=ntohs(iphdr->TotLen);
            if(ethdr->type == 0x0008 && iphdr->Protocol == 17 /*UDP*/ && iphdr->TotLen > iphdr->h * 4 + sizeof(UDP_HEADER) )
            {
                udphdr->length=ntohs(udphdr->length);
                unsigned int d_len1 = iphdr->TotLen - iphdr->h * 4 - sizeof(UDP_HEADER);
                unsigned int d_len2 = udphdr->length - sizeof(UDP_HEADER);
                if(d_len1 != d_len2)
                {
                    CM_WARNING_TRACE_THIS("CFileMediaConnection::ReadPacket, d_len1 != d_len2! d_len1 = "<< d_len1 << ", d_len2 = " << d_len2);
#if USE_DEBUG_PRINTF_TRACE
                    printf("d_len1 != d_len2! d_len1=%d,d_len2=%d\n",d_len1,d_len2);
#endif
                }
                
                CM_DETAIL_TRACE_THIS("CFileMediaConnection::ReadPacket, Receive packet: sourceIP = " << iphdr->source_ip << ", suorcePort = " << udphdr->src_port << ", destinationIP = " << iphdr->destination_ip << ", destinationPort = " << udphdr->dst_port);
#if USE_DEBUG_PRINTF_TRACE
                printf(("Receive packet: sourceIP = %d, suorcePort = %d, destinationIP = %d, destinationPort = %d\n"), iphdr->source_ip, udphdr->src_port, iphdr->destination_ip, udphdr->dst_port);
#endif
                if(m_uSourceIP == iphdr->source_ip && m_uDestinationIP == iphdr->destination_ip && m_uSourcePort == udphdr->src_port && m_uDestinationPort == udphdr->dst_port)
                {
                    // buffer rtp packet to check
                    m_pPacket = udp_raw;
                    m_iPacketLen = d_len2;
                    m_uPacketTime = pkthdr.ts.tv_sec * 1000 + pkthdr.ts.tv_usec / 1000;
                    m_iPacketCount++;
                    
                    m_bDelivered = false;
                }
            }
        }
        else if(!feof(m_pFile))
        {
            CM_ERROR_TRACE_THIS("CFileMediaConnection::ReadPacket, Reading file error! errnum: "<< ferror(m_pFile));
#if USE_DEBUG_PRINTF_TRACE
            printf(("Reading %s error , errnum: %d \n"), m_strFileName.c_str(), ferror(m_pFile));
#endif
            result = WME_E_FAIL;
        }
        else
        {
            result = WME_E_FAIL;
        }
    }
    else
    {
        CM_INFO_TRACE_THIS("CFileMediaConnection::ReadPacket, Readed end of file [" << m_strFileName.c_str() << "]");
        result = WME_E_FAIL;
    }
    
    return result;
}

int CFileMediaConnection::CheckPacket()
{
    // check deliver flag and packet valid
    if(true == m_bDelivered || NULL == m_pPacket || m_iPacketLen <= 0)
    {
        return WME_E_INVALIDARG;
    }
    
    // check packet deliver time
    unsigned int uDeliverTime = static_cast<unsigned int>(ticker::now()/1000);
    if(0 == m_uLastPacketTime)
    {
        m_uFirstPacketTime = m_uPacketTime;
        m_uLastPacketTime = m_uPacketTime;
    }
    if(0 == m_uLastDeliverTime)
    {
        m_uFirstDeliverTime = uDeliverTime;
        m_uLastDeliverTime = uDeliverTime;
    }
    unsigned int uIntervalPacketTime = m_uPacketTime - m_uFirstPacketTime;
    unsigned int uIntervalDeliverTime = uDeliverTime - m_uFirstDeliverTime;
    if(uIntervalDeliverTime < uIntervalPacketTime)
    {
        return WME_S_OK;
    }
    
    CM_DETAIL_TRACE_THIS("CFileMediaConnection::CheckPacket, Deliver packet: uIntervalPacketTime = " << uIntervalPacketTime << ", uIntervalDeliverTime = " << uIntervalDeliverTime);
#if USE_DEBUG_PRINTF_TRACE
    printf(("Deliver packet, uIntervalPacketTime = %d, uIntervalDeliverTime = %d \n"), uIntervalPacketTime, uIntervalDeliverTime);
#endif
    
    // check packet type by header info
    bool bRTPPacket = true;
    RTP_HEADER rtphdr = {0};
	memcpy(&rtphdr,m_pPacket,sizeof(rtphdr));
	rtphdr.m_flags = ntohs(rtphdr.m_flags);
	rtphdr.ph_seq = ntohs(rtphdr.ph_seq);
	rtphdr.ph_ts = ntohl(rtphdr.ph_ts);
	rtphdr.ph_ssrc = ntohl(rtphdr.ph_ssrc);
	if(rtphdr.u.ph_pt>=72 && rtphdr.u.ph_pt<=76)
	{
        bRTPPacket = false;
    }
    NetDataType dataType = TYPE_DATA_NONE;
    switch (m_eConnSinkType) {
        case CONN_SINK_VIDEO:
        case CONN_SINK_DESKSHARE:
            dataType = bRTPPacket ? TYPE_VIDEO_DATA : TYPE_VIDEO_CONTROL;
            break;
            
        case CONN_SINK_AUDIO:
            dataType = bRTPPacket ? TYPE_AUDIO_DATA : TYPE_AUDIO_CONTROL;
            break;
            
        default:
            break;
    }
    if(TYPE_DATA_NONE != dataType)
    {
        m_pSink->OnReceiveData(m_pPacket, m_iPacketLen, dataType);
    }
    else
    {
        //drop the packet
        CM_ERROR_TRACE_THIS("CFileMediaConnection::CheckPacket, Drop packet for dataType isn't supported, dataType = " << dataType);
#if USE_DEBUG_PRINTF_TRACE
        printf(("Drop packet, dataType[%d] isn't supported\n"), dataType);
#endif
    }
    
    m_bDelivered = true;
    m_uLastPacketTime = m_uPacketTime;
    m_uLastDeliverTime = uDeliverTime;
    
    return WME_S_OK;
}

void CFileMediaConnection::OnTimer(CCmTimerWrapperID *aId)
{
    do
    {
        if(WME_S_OK != ReadPacket())
        {
            break;
        }
        CheckPacket();
    }while (m_bDelivered);
}

//////////////////////////////////////////////////////////////////////////
CFileMediaConnection::CTimerEvent::CTimerEvent(CCmTimerWrapperID *pTimer, bool bSechdule, CCmTimerWrapperIDSink *pSink, double fInterval, int iCount)
: m_pTimer(pTimer), m_bSechdule(bSechdule), m_pSink(pSink), m_fInterval(fInterval), m_iCount(iCount)
{
	CM_INFO_TRACE_THIS("CFileMediaConnection::CTimerEvent::CWmeTimerEvent");
}

CFileMediaConnection::CTimerEvent::~CTimerEvent()
{
	m_pTimer = NULL;
	m_pSink = NULL;
    
	CM_INFO_TRACE_THIS("CFileMediaConnection::CTimerEvent::~CWmeTimerEvent");
}

// ICmEvent interface
CmResult CFileMediaConnection::CTimerEvent::OnEventFire()
{
	 CM_INFO_TRACE_THIS("CFileMediaConnection::CTimerEvent::OnEventFire, pTimer = " << m_pTimer << ", bSechdule = " << m_bSechdule << ", pSink = " << m_pSink << ", fInterval = " << m_fInterval << ", iCount =" << m_iCount);
    
	CmResult cmResult = 0;
    
	if(NULL != m_pTimer)
	{
		if(true == m_bSechdule)
		{
			cmResult = m_pTimer->Schedule(m_pSink, CCmTimeValue(m_fInterval), m_iCount);
		}
		else
		{
			cmResult = m_pTimer->Cancel();
		}
	}
    
	return cmResult;
}
