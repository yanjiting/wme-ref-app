#include "CustomCommandLineInfo.h"


BOOL CCustomCommandLineInfo::IsHost() { return m_bHost; };
CString CCustomCommandLineInfo::ClientIP() { return m_sHostAddr; };
BOOL CCustomCommandLineInfo::IsClient() { return m_bClient; };
BOOL CCustomCommandLineInfo::IsStunPoc() { return m_bStunPoc; };
BOOL CCustomCommandLineInfo::IsSyslogEnabled() { return m_bSyslog; };
CString CCustomCommandLineInfo::SyslogIP() { return m_sSyslogAddr; };

void CCustomCommandLineInfo::ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast)
{
	if (bFlag) {
		CString sParam(lpszParam);
		if (sParam.Left(2) == "c:") {
			m_sHostAddr = sParam.Right(sParam.GetLength() - 2);
			m_bClient = TRUE;
			return;
		}

		if (sParam.Left(1) == "h") {
			m_bHost = TRUE;
			return;
		}
		if (sParam.Left(3) == "poc") {
			m_bStunPoc = TRUE;
			return;
		}
		if (sParam.Left(7) == "syslog:") {
			m_sSyslogAddr = sParam.Right(sParam.GetLength() - 7);
			m_bSyslog = TRUE;
			return;
		}
	}
}

CCustomCommandLineInfo::CCustomCommandLineInfo()
{
	m_bHost = m_bClient = m_bStunPoc = m_bSyslog = FALSE;

}


CCustomCommandLineInfo::~CCustomCommandLineInfo()
{
}
