//#include <windows.h>
#include <afxwin.h>

/*
EngineDemo [/h] [/c:hostIP] [/poc] [/syslog:syslogIP]
*/


class CCustomCommandLineInfo :
	public CCommandLineInfo
{
private:
	BOOL m_bHost;       //for /h
	CString m_sHostAddr;  //for /c:hostIP
	CString m_sSyslogAddr; //for /syslog:syslogIP
	BOOL m_bClient;		//for /c:hostIP
	BOOL m_bStunPoc;	//for /poc
	BOOL m_bSyslog;		//for /syslog:syslogIP

public:
	CCustomCommandLineInfo();
	~CCustomCommandLineInfo();
	BOOL IsHost();
	CString ClientIP();
	CString SyslogIP();
	BOOL IsClient();
	BOOL IsStunPoc();
	BOOL IsSyslogEnabled();
	virtual void ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast);
};
