#include <getopt.h>
#include "CustomCommandLineInfo.h"


bool CCustomCommandLineInfo::IsHost() { return m_bHost; };
string CCustomCommandLineInfo::ClientIP() { return m_sHostAddr; };
bool CCustomCommandLineInfo::IsClient() { return m_bClient; };
bool CCustomCommandLineInfo::IsStunPoc() { return m_bStunPoc; };
bool CCustomCommandLineInfo::IsSyslogEnabled() { return m_bSyslog; };
string CCustomCommandLineInfo::SyslogIP() { return m_sSyslogAddr; };
bool CCustomCommandLineInfo::IsNoRender() { return m_bNoRender; };

void CCustomCommandLineInfo::ParseParam(int argc, char * const argv[])
{
    int option;
    const char* const shortOptions = "hc:ps:n";
    const struct option longOptions[] =
    {
        {"host", 0, NULL, 'h'},
        {"client", 1, NULL, 'c'},
        {"poc", 0, NULL, 'p'},
        {"syslog", 1, NULL, 's'},
        {"norender", 0, NULL, 'n'}
    };
    
    
    //while ((option = getopt_long(argc, argv, shortOptions, longOptions, NULL)) != -1)
    while ((option = getopt(argc, argv, shortOptions)) != -1)
    {
        switch (option) {
            case 'h':
                m_bHost = true;
                break;
            case 'c':
                m_bClient = true;
                m_sHostAddr = optarg;
                break;
            case 'p':
                m_bStunPoc = true;
                break;
            case 's':
                m_bSyslog = true;
                m_sSyslogAddr = optarg;
                break;
            case 'n':
                m_bNoRender = true;
                break;
            default:
                break;
        }
    }
    
    //if (opterr != -1)
    //{
    //    printf("error command line");
    //}

}

CCustomCommandLineInfo::CCustomCommandLineInfo()
{
	m_bHost = m_bClient = m_bStunPoc = m_bSyslog = m_bNoRender = false;

}


CCustomCommandLineInfo::~CCustomCommandLineInfo()
{
}
