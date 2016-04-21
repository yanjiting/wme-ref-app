#ifndef _CUSTOM_COMMAND_LINE_INFO_
#define _CUSTOM_COMMAND_LINE_INFO_
//#include <windows.h>
//#include <afxwin.h>
#include <string>

using namespace std;

/*
EngineDemo [/h] [/c:hostIP] [/poc] [/syslog:syslogIP]
*/


class CCustomCommandLineInfo
{
private:
	bool m_bHost;       //for /h
    string m_sHostAddr;  //for /c:hostIP
	string m_sSyslogAddr; //for /syslog:syslogIP
	bool m_bClient;		//for /c:hostIP
	bool m_bStunPoc;	//for /poc
	bool m_bSyslog;		//for /syslog:syslogIP
	bool m_bNoRender;		//for /norender
    
public:
	CCustomCommandLineInfo();
	~CCustomCommandLineInfo();
	bool IsHost();
	string ClientIP();
	string SyslogIP();
	bool IsClient();
	bool IsStunPoc();
	bool IsSyslogEnabled();
    bool IsNoRender();
	virtual void ParseParam(int argc, char * const argv[]);
};
#endif