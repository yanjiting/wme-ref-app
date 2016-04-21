#include "Syslog.h"
#include <stdio.h>

Syslog::~Syslog() {
	if (!initialized)
		return;
	closesocket(sock);
	WSACleanup();
	initialized = FALSE;
}

Syslog::Syslog(const char *dotted_decimal) {

	UTF8_BOM[0] = '\0';
	initialized = FALSE;
	log_mask = 0xFF;
	version = 1;
	mid = 0;

	memset(&sa_logger, 0, sizeof(sa_logger));
	sa_logger.sin_family = AF_INET;
	
	sa_logger.sin_port = htons(SYSLOG_PORT);
	sa_logger.sin_addr.s_addr = inet_addr(dotted_decimal);

	syslog_facility = LOG_USER;

	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	int err;

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		MessageBox(NULL, _T("You asked for syslog but there was a failure starting winsock."), L"error", MB_OK);
		return;
	}


	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) {
		MessageBox(NULL, _T("ERROR! Unable to create socket for syslog!"), L"bla", MB_OK);
		return;
	}
	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof((addr)));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(0);

	if (bind(sock, (SOCKADDR *)&addr, sizeof(addr)) == SOCKET_ERROR) {
		MessageBox(NULL, _T("ERROR! Unable to bind to socket for syslog!"), L"bla", MB_OK);
		initialized = FALSE;
		closesocket(sock);
	}
	else {
		initialized = TRUE;
		log_mask = LOG_MASK(LOG_INFO);
		wchar_t wlhost[MAX_COMPUTERNAME_LENGTH+1];
		DWORD n = sizeof(wlhost);
		GetComputerName(wlhost, &n);
		wcstombs(local_hostname, wlhost, sizeof(local_hostname));
	}
}

void Syslog::log(char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsyslog(LOG_INFO, fmt, ap);
	va_end(ap);
}

void Syslog::vsyslog(int pri, char* fmt, va_list ap)
{
	static char *month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	SYSTEMTIME stm;
	int len;
	char *p;
	char datagramm[SYSLOG_DGRAM_SIZE];
	LONG current_mid = 0;
	char timestamp[128];

	if (!(LOG_MASK(LOG_PRI(pri)) & log_mask))
		return;

	if (!initialized)
		return;

	if (!(pri & LOG_FACMASK)) {
		pri |= syslog_facility;
	}

	current_mid = InterlockedIncrement(&mid);

	GetLocalTime(&stm);
	sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02d.%dZ", stm.wYear, stm.wMonth, stm.wDay, stm.wHour,
		stm.wMinute, stm.wSecond, stm.wMilliseconds);

	_snprintf(str_pid, sizeof(str_pid), "%lu", GetCurrentProcessId());

	len = sprintf(datagramm, "<%d>%d %s %s %s %s %ld %s %s%s ",
		pri, version, timestamp, local_hostname, APP_NAME, str_pid, current_mid, "", UTF8_BOM, "jwpl");

	vsnprintf(datagramm + len, datagramm_size - len, fmt, ap);
	p = strchr(datagramm, '\n');
	if (p)
		*p = 0;
	p = strchr(datagramm, '\r');
	if (p)
		*p = 0;

	//printf("log: %s", datagramm);
	sendto(sock, datagramm, strlen(datagramm), 0, (SOCKADDR*)&sa_logger, sizeof(SOCKADDR_IN));
}
