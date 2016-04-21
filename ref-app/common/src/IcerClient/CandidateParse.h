// CandidateParse.h: interface for the CCandidateParse class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CANDIDATE_PARESE_H__
#define _CANDIDATE_PARESE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>
#include "cstdefine.h"
#include "csttransport.h"
#include <vector>
class CCandidateParse  
{
public:
	CCandidateParse();
	virtual ~CCandidateParse();

	void PackCandidate(eIceRole eRole, std::vector<const IceCandidate*>& vecCandidates, std::string &strXML);
	void ParseCandidate(const char* pszXML, eIceRole& eRole, std::vector<IceCandidate*>& vecCandidates);

};

#endif //#ifndef _CANDIDATE_PARESE_H__
