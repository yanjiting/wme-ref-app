#include "CmMyThread.h"

#include "CmDebug.h"
#include "CmUtilMisc.h"

CCmMyThread::CCmMyThread()
{
	m_isStop = FALSE;
	m_status = STATUS_INIT;	
}


CmResult CCmMyThread::Stop(CCmTimeValue* aTimeout)
{
	m_isStop = TRUE;
	return CM_OK;
}

void CCmMyThread::OnThreadRun()
{
	while (!m_isStop) {

		m_status = STATUS_RUN;
		
		CM_INFO_TRACE_THIS("OnThreadRun is called");	

        SleepMs(50);

	}

	m_status = STATUS_STOP;
}

