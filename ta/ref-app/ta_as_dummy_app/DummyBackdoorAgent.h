#ifndef _DUMMY_BACKDOOR_AGENT_H_
#define _DUMMY_BACKDOOR_AGENT_H_

#pragma once

#include <string>
#include "BackdoorAgentSink.h"
#include "TAHTTPServer.h"
#include "CmInetAddr.h"
#include "CmThreadManager.h"

//=====================================================================================================
//
template<class T>
class CBackdoorAgentBaseT : public IBackdoorSink
{
public:
	typedef std::string(T::*BackDoorFunc)(std::string arg);
	CBackdoorAgentBaseT(){
		m_pTAServer = NULL;
	}
	~CBackdoorAgentBaseT(){
		Finit();
	}

	bool Init(int port){

		CCmThreadManager * pCCmThreadManager = CCmThreadManager::Instance();
		if (!CM_SUCCEEDED(pCCmThreadManager->SocketStartup()))
			return false;

		if (m_pTAServer == NULL)
			m_pTAServer = new TAServer(this);

		CCmInetAddr addr("127.0.0.1", port);
		return CM_SUCCEEDED(m_pTAServer->Init(addr));
	}
	void Finit(){
		if (m_pTAServer) delete m_pTAServer;
		m_pTAServer = NULL;

		CCmThreadManager * pCCmThreadManager = CCmThreadManager::Instance();
		if (pCCmThreadManager)
			pCCmThreadManager->SocketCleanup();
	}

	virtual bool OnRequest(std::string selector, std::string arg, std::string &result){
		T* pT = static_cast<T*>(this);
		auto method = m_mapBackdoors.find(selector);
		if (method == m_mapBackdoors.end()){
			CM_INFO_TRACE("CBackdoorAgent::OnRequest, method not found.");
			result += "Backdoor: ";
			result += selector + "not found.";
			return false;
		}
		result = (pT->*(method->second))(arg);
		if (result.compare("Failed") == 0)
			return false;
		return true;
	}
protected:
	TAServer * m_pTAServer;
	std::map<std::string, BackDoorFunc> m_mapBackdoors;
};


//=====================================================================================================
//
class CDummyBackdoorAgent : public CBackdoorAgentBaseT<CDummyBackdoorAgent>{
public:
	CDummyBackdoorAgent();
	~CDummyBackdoorAgent();

	//
	std::string backdoorShowQRCodeView(std::string arg); 
	std::string backdoorHideQRCodeView(std::string arg);
	std::string backdoorStopDummyApp(std::string arg);
	
};


//=====================================================================================================
//
#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

	void InitAgentByInputParams(int argc, const char ** argv);
	void CleanupAgent();
	char* getCmdOption(char ** begin, char ** end, const std::string & option);
	void ShowQRCodeView(int x, int y, int w, int h, int row, int column, std::vector<std::string>& v_qr_contents);
	void HideQRCodeView();
#ifdef __cplusplus
}
#endif //__cplusplus


#endif //_DUMMY_BACKDOOR_AGENT_H_