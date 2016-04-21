#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "Syslog.h"


Syslog::~Syslog() {
	if (!initialized)
		return;
	close(sock);

	initialized = false;
}

Syslog::Syslog(const char *dotted_decimal) {

	UTF8_BOM[0] = '\0';
	initialized = false;
	log_mask = 0xFF;
	version = 1;
	mid = 0;

	memset(&sa_logger, 0, sizeof(sa_logger));
	sa_logger.sin_family = AF_INET;
	
	sa_logger.sin_port = htons(SYSLOG_PORT);
	sa_logger.sin_addr.s_addr = inet_addr(dotted_decimal);

	syslog_facility = LOG_USER;


	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == -1) {
		printf("ERROR! Unable to create socket for syslog! \n");
		return;
	}
	sockaddr_in addr;
	memset(&addr, 0, sizeof((addr)));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(0);

	if (bind(sock, (sockaddr *)&addr, sizeof(addr)) == -1) {
		printf("ERROR! Unable to bind to socket for syslog! \n");
		initialized = false;
		close(sock);
	}
	else {
        printf("SUCCESSFUL! Bind socket for syslog! \n");
		initialized = true;
		log_mask = LOG_MASK(LOG_INFO);
        //gethostname(<#char *#>, <#size_t#>)
		//wchar_t wlhost[MAX_COMPUTERNAME_LENGTH+1];
		//DWORD n = sizeof(wlhost);
		//GetComputerName(wlhost, &n);
		//wcstombs(local_hostname, wlhost, sizeof(local_hostname));
	}
}

void Syslog::log(const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsyslog(LOG_INFO, fmt, ap);
	va_end(ap);
}

void Syslog::vsyslog(int pri, const char* fmt, va_list ap)
{
	//static char *month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	//	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	struct timeval stm;
	int len;
	char *p;
	char datagramm[SYSLOG_DGRAM_SIZE];
	long current_mid = 0;
	char timestamp[128];

	if (!(LOG_MASK(LOG_PRI(pri)) & log_mask))
		return;

	if (initialized == false)
		return;

	if (!(pri & LOG_FACMASK)) {
		pri |= syslog_facility;
	}

	//current_mid = InterlockedIncrement(&mid);

	gettimeofday(&stm, NULL);
	sprintf(timestamp, "%ld%ld", stm.tv_sec, stm.tv_sec);

	//snprintf(str_pid, sizeof(str_pid), "%lu", GetCurrentProcessId());

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
	sendto(sock, datagramm, strlen(datagramm), 0, (sockaddr*)&sa_logger, sizeof(sockaddr_in));
}
