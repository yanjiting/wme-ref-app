#pragma once
#include "CmDebug.h"


#define DUMMY_ERROR_TRACE(str) CM_ERROR_TRACE_EX("DummyApp",str)     
#define DUMMY_WARNING_TRACE_EX(str) CM_WARNING_TRACE_EX("DummyApp",str)   
#define DUMMY_INFO_TRACE_EX(str) CM_INFO_TRACE_EX("DummyApp",str)   


class DummyConfig{
public:
	static DummyConfig& Instance() { return dummyConfig; }
	std::string m_strShowQRCode;

protected:
	DummyConfig(){
		m_strShowQRCode = "";
	}
	~DummyConfig(){
	};
	static DummyConfig dummyConfig;
};
