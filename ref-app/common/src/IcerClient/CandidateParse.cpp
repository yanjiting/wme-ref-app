// CandidateParse.cpp: implementation of the CCandidateParse class.
//
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "tinyxml.h"
#include "CandidateParse.h"
//.For each candidate, the following information needs to be exchanged:
//		component ID
//		candidate type (i.e. host, srflx, or relay)
//		foundation ID
//		priority
//		transport type (only UDP is supported for now)
//		transport address (address family, IP address, and port)

//optional related address (e.g. for srflx/STUN candidate,
// the related address is the local address where STUN request is sent from).
// This would only be used for troubleshooting purposes and is not required by ice_strans.
// sample of SDP:
//		a=ice-ufrag:2b2c6196 
//		a=ice-pwd:06ea0fa8 
//		a=candidate:Sc0a8000e 1 UDP 1694498815 81.178.x.y 4808 typ srflx raddr 192.168.0.14 rport 4808
//		a=candidate:Hc0a8000e 1 UDP 2130705151 192.168.0.14 4808 typ host
//		a=candidate:Sc0a8000e 2 UDP 1694498814 81.178.x.y 4809 typ srflx raddr 192.168.0.14 rport 4809
//		a=candidate:Hc0a8000e 2 UDP 2130705150 192.168.0.14 4809 typ host


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
const char* NODE_CONTROL_ROLE      = "ctrl_role"; //1. controlling 2. controlled
const char* VALUE_CONTROLLING       = "controlling";
const char* VALUE_CONTROLLED        = "controlled";
const char* NODE_CANDIDATES			= "candidates";
const char* NODE_ITEM				= "item";
const char* NODE_COMPONENT_ID		= "comp_id";
const char* NODE_CANDIDATE_TYPE		= "candi_type";
const char* NODE_FOUNDATION_ID		= "foundation";
const char* NODE_PRIORITY			= "priority";
const char* NODE_TRANS_TYPE         = "trans_type";
const char* NODE_PORT				= "port";
const char* NODE_ADDR				= "addr";

const char* NODE_RELATED_ADDR       = "raddr";
const char* NODE_RELATED_PORT       = "rport";

const char* CANDIDATE_TYPE_HOST     = "host";
const char* CANDIDATE_TYPE_SRFLX    = "srflx";
const char* CANDIDATE_TYPE_RELAY    = "relay";

const char* TRANS_TYPE_UDP          ="UDP";
const char* TRANS_TYPE_TCP          ="TCP";
const char* TRANS_TYPE_BOTH         ="BOTH";

const char* NODE_PAYLOAD_TYPE            ="PAYLOAD";
const char* NODE_PAYLOAD_TYPE_VIDEO      ="VIDEO";


CCandidateParse::CCandidateParse()
{

}

CCandidateParse::~CCandidateParse()
{

}


void CCandidateParse::PackCandidate(eIceRole eRole, std::vector<const IceCandidate*>& vecCandidates, std::string &strXML)
{
    if( vecCandidates.empty())
        return;
   /*
    typedef struct
    {
	CCmInetAddr         m_addr;
	
	eTransType			m_transType;
	
	CCmInetAddr     	m_relAddr;
    
	int					m_iComponentId;
	CCmString       	m_strFoundation;
	unsigned long		m_ulFoundationLen;
	int					m_iPriority;
	eIceCandidateType	m_candidateType;
    
    }IceCandidate;
    
    
    */
    TiXmlDocument xml;
	TiXmlDeclaration *pDeclaration = new TiXmlDeclaration(("1.0"),("UTF-8"),(""));
    
    xml.LinkEndChild(pDeclaration);
    
    char szTmp[256] = {0};
    
//     TiXmlNode* roleNode = new TiXmlElement( NODE_CONTROL_ROLE );
//     
//     snprintf( szTmp, sizeof(szTmp), "%d", eRole);
//     xml.LinkEndChild( roleNode)->InsertEndChild(TiXmlText( szTmp));
//     memset(szTmp, sizeof(szTmp), 0);
//     

//     TiXmlNode* roleNode = new TiXmlElement( NODE_PAYLOAD_TYPE );
// 	xml.LinkEndChild( roleNode)->InsertEndChild(TiXmlText( NODE_PAYLOAD_TYPE_VIDEO));

    TiXmlNode* writeRoot = new TiXmlElement( NODE_CANDIDATES );
    xml.LinkEndChild( writeRoot );
    
    std::vector<const IceCandidate*>::iterator it = vecCandidates.begin();
    
    
	for( ; it != vecCandidates.end(); ++it )
	{
        const IceCandidate* pICECandidate = *it;
        if( !pICECandidate )
            continue;
		TiXmlNode* newNode = new TiXmlElement( NODE_ITEM );
		
		writeRoot->LinkEndChild( newNode );
        
		const TiXmlNode* comp_idNode  = new TiXmlElement( NODE_COMPONENT_ID );
        
        snprintf(szTmp, sizeof(szTmp), "%d", pICECandidate->m_iComponentId);
        newNode->InsertEndChild(*comp_idNode)->InsertEndChild(TiXmlText(szTmp));

		const TiXmlNode* type4NewNode = new TiXmlElement( NODE_CANDIDATE_TYPE );
        
        snprintf( szTmp, sizeof(szTmp), "%d", pICECandidate->m_candidateType);
		newNode->InsertEndChild(*type4NewNode)->InsertEndChild(TiXmlText(szTmp));
        memset(szTmp, sizeof(szTmp), 0);
        
        
		const TiXmlNode* found_idNode = new TiXmlElement( NODE_FOUNDATION_ID );
		newNode->InsertEndChild(*found_idNode)->InsertEndChild(TiXmlText(pICECandidate->m_strFoundation.c_str()));
        
		const TiXmlNode* priorityNode = new TiXmlElement( NODE_PRIORITY);
        
        snprintf( szTmp, sizeof(szTmp), "%d", pICECandidate->m_iPriority);
		newNode->InsertEndChild(*priorityNode)->InsertEndChild(TiXmlText(szTmp));
        memset(szTmp, sizeof(szTmp), 0);
        
		const TiXmlNode* trans_type4NewNode = new TiXmlElement( NODE_TRANS_TYPE );
        

        snprintf( szTmp, sizeof(szTmp), "%d", pICECandidate->m_transType);
		newNode->InsertEndChild(*trans_type4NewNode)->InsertEndChild(TiXmlText(szTmp));
        memset(szTmp, sizeof(szTmp), 0);
        
		const TiXmlNode* addr4NewNode = new TiXmlElement( NODE_ADDR );
        CCmString strHostIp = pICECandidate->m_addr.GetIpDisplayName();
		newNode->InsertEndChild(*addr4NewNode)->InsertEndChild(TiXmlText( strHostIp.c_str()));
		
		const TiXmlNode* port4NewNode = new TiXmlElement( NODE_PORT );
        
        snprintf( szTmp, sizeof(szTmp), "%d", pICECandidate->m_addr.GetPort());
		newNode->InsertEndChild(*port4NewNode)->InsertEndChild(TiXmlText(szTmp));
        memset(szTmp, sizeof(szTmp), 0);
        
		//related address and port are optional
		const TiXmlNode* raddr4NewNode = new TiXmlElement( NODE_RELATED_ADDR );
        CCmString strRHostIp = pICECandidate->m_relAddr.GetIpDisplayName();
		newNode->InsertEndChild(*raddr4NewNode)->InsertEndChild(TiXmlText(strRHostIp.c_str()));
		
		const TiXmlNode* rport4NewNode = new TiXmlElement( NODE_RELATED_PORT );
        snprintf( szTmp, sizeof(szTmp), "%d", pICECandidate->m_relAddr.GetPort());
		newNode->InsertEndChild(*rport4NewNode)->InsertEndChild(TiXmlText(szTmp));
        memset(szTmp, sizeof(szTmp), 0);
  		
	}

	TiXmlPrinter printer;
	printer.SetIndent( "\t" );
	
    
    
	xml.Accept( &printer );
	fprintf( stdout, "%s", printer.CStr() );
    strXML = printer.CStr();

}

void CCandidateParse::ParseCandidate(const char* pszXML, eIceRole& eRole, std::vector<IceCandidate*>& vecCandidates)
{
	TiXmlDocument xml;
	xml.Parse(pszXML);
	if ( xml.Error() )
	{
		printf( "Error in %s: %s\n", xml.Value(), xml.ErrorDesc() );
		return;
	}

	TiXmlNode*  root = xml.RootElement();
    TiXmlNode*  role_node = root->FirstChild(NODE_CONTROL_ROLE);
    if ( role_node )
    {
        const char* pszValue = NULL;
        role_node->ToElement()->GetText();
        if ( pszValue)
        {
            eRole = (eIceRole)atoi(pszValue);
        }
    }
	TiXmlNode*  node = root->FirstChild( NODE_ITEM );
	for(; node; node = node->NextSibling())
	{
		//should new Candidate class instance at here
        IceCandidate* pIceCandidate = new IceCandidate();

		const char* pszValue = NULL;

		TiXmlNode* child = node->FirstChild( NODE_COMPONENT_ID );  
		if( child )
		{
			pszValue = child->ToElement()->GetText();
            
            if( pszValue)
            {
                int id = atoi( pszValue);
                pIceCandidate->m_iComponentId = id;
            }
            
		}

		child = node->FirstChild( NODE_CANDIDATE_TYPE );
		if( child )
		{
			pszValue = child->ToElement()->GetText();
            if( pszValue)
            {
                eIceCandidateType eType = (eIceCandidateType)atoi(pszValue);
                pIceCandidate->m_candidateType = eType;
            }
		}

		child = node->FirstChild( NODE_FOUNDATION_ID );
		if(child)
		{
			pszValue = child->ToElement()->GetText();
            if( pszValue)
            {
                pIceCandidate->m_strFoundation = pszValue;
            }
		}

		child = node->FirstChild( NODE_PRIORITY );
		if(child)
		{
			pszValue = child->ToElement()->GetText();
            if( pszValue)
            {
                int iPriority = atoi( pszValue );
                pIceCandidate->m_iPriority = iPriority;
            }
		}

		child = node->FirstChild( NODE_TRANS_TYPE );
		if(child)
		{
			pszValue = child->ToElement()->GetText();
            if( pszValue)
            {
                eTransType eType = (eTransType)atoi( pszValue);
                pIceCandidate->m_transType = eType;
            }
		}

		child = node->FirstChild( NODE_ADDR );
        CCmString strIp;
		if(child)
		{
			pszValue = child->ToElement()->GetText();
            strIp = pszValue;
		}

		child = node->FirstChild( NODE_PORT );
        int nPort = 0;
		if(child)
		{
			pszValue = child->ToElement()->GetText();
            if( pszValue)
            {
                nPort = atoi( pszValue);
            }
		}

        pIceCandidate->m_addr.Set( strIp.c_str(), nPort);
        
		child = node->FirstChild( NODE_RELATED_ADDR );
		if(child)
		{
			pszValue = child->ToElement()->GetText();
            strIp = pszValue;
		}

		child = node->FirstChild( NODE_RELATED_PORT );
        nPort = 0;
		if(child)
		{
			pszValue = child->ToElement()->GetText();
            if( pszValue)
            {
                nPort = atoi(pszValue);
            }
		}
        
        pIceCandidate->m_relAddr.Set( strIp.c_str(), nPort);
        vecCandidates.push_back( pIceCandidate );
	}

}


