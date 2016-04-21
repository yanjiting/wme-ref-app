#include "FileCaptureEngineImp.h"
#include "CmDebug.h"
#include "timer.h"


namespace wme
{
#define DEFAULT_BUFFER_LEN (1280*720*4)
	CFileCaptureEngineImp::CFileCaptureEngineImp()
	{
		m_strFileName = "";
		memset(&m_sVideoRawFormat, 0, sizeof(m_sVideoRawFormat));
		m_sVideoRawFormat.eRawType = WmeVideoUnknown;

		m_pVideoFile = NULL;
		m_bStarted = FALSE;
		m_bStoped = TRUE;
		m_bInit = FALSE;
		m_pReadingThread = NULL;
		m_bStartedTimer = FALSE;
		m_pReadingBuffer = NULL;
		m_nReadingBufferLen = DEFAULT_BUFFER_LEN;

		m_nDataLen = 0;
		m_pExternalInputter = NULL;

	}

	CFileCaptureEngineImp::~CFileCaptureEngineImp()
	{
		Uninit();
	}

	WMERESULT CFileCaptureEngineImp::SetVideoFile(const char *pFileName, WmeVideoRawFormat *pFormat)
	{
		if(!pFileName || !pFormat || pFormat->eRawType == WmeVideoUnknown)
		{
            CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::SetVideoFile, invalid file name or video format ");
			return WME_E_INVALIDARG;
		}

		m_strFileName = pFileName;
		m_sVideoRawFormat = *pFormat;

		if(m_bStarted)
		{
			CloseVideoFile();
			ResetTimer();
		}

		return WME_S_OK;
	}

	WMERESULT CFileCaptureEngineImp::Init()
	{
		if (m_pReadingThread == NULL) {
			CmResult rettimer = ::CreateUserTaskThread("v-fcap", m_pReadingThread, TF_JOINABLE, TRUE);
			if (CM_FAILED(rettimer)) {
				CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::Init(): failed to create a statics thread.");

				m_pReadingThread = NULL;
				return WME_E_FAIL;
			}
		}

		m_bInit = TRUE;

		return WME_S_OK;
	}

	WMERESULT CFileCaptureEngineImp::Uninit()
	{
		CmResult ret=CM_OK;
		if(m_bInit == FALSE)
			return WME_S_OK;
		CM_INFO_TRACE_THIS("CFileCaptureEngineImp::Uninit()");

		Stop();	

		///destroy thread
		if (m_pReadingThread) {

			m_pReadingThread->Stop();
			ret = m_pReadingThread->Join();
			if (CM_FAILED(ret)) {
				CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::Uninit(): failed to join the thread.");
			}
			m_pReadingThread->Destory(CM_OK);
			m_pReadingThread = NULL;
		}

		if(m_pReadingBuffer)
		{
			delete []m_pReadingBuffer;
			m_pReadingBuffer = NULL;
		}

		m_bInit = FALSE;
		return WME_S_OK;
	}

	WMERESULT CFileCaptureEngineImp::Start()
	{
		WMERESULT ret=WME_E_FAIL;
		CmResult rettimer =CM_OK;

		if(!m_bInit)
		{
			CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::Start(): not inited!");
			return WME_E_FAIL;
		}

		if(m_bStarted)
		{
			return WME_S_OK;
		}

		ret = OpenVideoFile();

		if (ret==WME_S_OK)
		{
			////////////statistics timer start
			if (m_pReadingThread) {
				ICmEvent* startEvent = new CStartEvent(*this);
				if (startEvent) {
					rettimer = m_pReadingThread->GetEventQueue()->SendEvent(startEvent);
					if (CM_FAILED(rettimer))
					{
						CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::Start(): failed to send start event to statics thread.");
						return WME_E_FAIL;
					}
				}
			}
		}

		m_bStarted = TRUE;
		m_bStoped = FALSE;
		
		CM_INFO_TRACE_THIS( "CFileCaptureEngineImp::Start end");

		return ret;	

	}

	WMERESULT CFileCaptureEngineImp::Stop()
	{
		if(!m_bInit)
		{
			CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::Stop(): not inited!");
			return WME_E_FAIL;
		}

		if ( m_bStoped )
		{
			return WME_S_OK;
		}
		///stop statistics thread
		CmResult ret = CM_OK;
		if (m_pReadingThread) {
			ICmEvent* stopEvent = new CStopEvent(*this);
			if (stopEvent) {
				ret = m_pReadingThread->GetEventQueue()->SendEvent(stopEvent);
				if (CM_FAILED(ret)) 
				{
					CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::Stop(): failed to send stop event to sending thread.");
				}
			}
		}

		CloseVideoFile();

		m_bStoped = TRUE ;
		m_bStarted = FALSE;

		return WME_S_OK;
	}


	WMERESULT CFileCaptureEngineImp::ResetFrameRate(float fFrameRate)
	{
		if(fFrameRate <=0 || fFrameRate > 100)
		{
			CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::RetFrameRate, invalid frame rate " << fFrameRate);
			return WME_E_INVALIDARG;
		}

		if(m_sVideoRawFormat.fFrameRate != fFrameRate)
		{
			m_sVideoRawFormat.fFrameRate = fFrameRate;
			ResetTimer();
		}

		return WME_S_OK;
	}

	//timer 

	void CFileCaptureEngineImp::OnTimer(CCmTimerWrapperID* aId)
	{
		WMERESULT ret = ReadVideoFrame();
		if(WME_FAILED(ret))
		{
			CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::OnTimer, failed to read video frame!");
			return;
		}

		DWORD dwNow = static_cast<DWORD>(ticker::now()/1000);
		if(m_pExternalInputter)
		{
			m_sVideoRawFormat.uTimestamp = dwNow;
			m_pExternalInputter->InputMediaData(dwNow, WmeMediaFormatVideoRaw, &m_sVideoRawFormat, (unsigned char *)m_pReadingBuffer, m_nDataLen);
		}

		return;
		
//		CM_INFO_TRACE_THIS("CFileCaptureEngineImp::OnTimer():statistics BitRate="<<m_uAvgBitRateLastSecond);

	}

	CmResult CFileCaptureEngineImp::StartTimer()
	{

		if (m_bStartedTimer) {
			return CM_OK;
		}
		
		if(m_sVideoRawFormat.fFrameRate <= 0 || m_sVideoRawFormat.fFrameRate - 100.0 > 0)
		{
			CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::StartTimer, m_sVideoRawFormat.fFrameRate is not suitable, reset it to 15! ");
			m_sVideoRawFormat.fFrameRate = 15.0;
		}

		long uSec = (long)(1000000/m_sVideoRawFormat.fFrameRate);

		CmResult ret = m_Timer.Schedule(this, CCmTimeValue(0, uSec));
		if (CM_FAILED(ret)) {
			CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::StartTimer(): failed to schedule timer.");
		} else {
			m_bStartedTimer = TRUE;
		}

		return ret;
	}
	CmResult CFileCaptureEngineImp::StopTimer()
	{
		if (!m_bStartedTimer) {
			return CM_OK;
		}

		CmResult ret = m_Timer.Cancel();
		if (CM_FAILED(ret)) {
			CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::StopTimer(): failed to cancel timer.");
		} else {
			m_bStartedTimer = FALSE;
		}

		return ret;
	}

	WMERESULT CFileCaptureEngineImp::ResetTimer()
	{
		if(!m_bInit || !m_pReadingThread)
		{
			CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::ResetTimer(): not inited!");
			return WME_E_FAIL;
		}

		if(!m_bStarted)
		{
			return WME_S_OK;
		}

		ICmEvent* stopEvent = new CStopEvent(*this);
		if (stopEvent) 
		{
			WMERESULT ret = m_pReadingThread->GetEventQueue()->SendEvent(stopEvent);
			if (CM_FAILED(ret)) 
			{
				CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::ResetFrameRate(): failed to send stop event to sending thread.");
				return ret;
			}
		}

		ICmEvent* startEvent = new CStartEvent(*this);
		if (startEvent) {
			WMERESULT ret = m_pReadingThread->GetEventQueue()->SendEvent(startEvent);
			if (CM_FAILED(ret))
			{
				CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::ResetFrameRate(): failed to send start event to statics thread.");
				return ret;
			}
		}

		return WME_S_OK;		
	}

	WMERESULT CFileCaptureEngineImp::OpenVideoFile()
	{
		if(!m_pVideoFile)
		{
			if(m_strFileName.length() == 0)
			{
				CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::Start(): no video file!");
				return WME_E_FAIL;
			}

			m_ReadingLock.Lock();
#ifdef CM_WIN32
			errno_t err = fopen_s(&m_pVideoFile, m_strFileName.c_str(), "rb");
#else
			m_pVideoFile = fopen(m_strFileName.c_str(), "rb");
#endif
			m_ReadingLock.UnLock();

			if(!m_pVideoFile)
			{
				CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::Start(): failed to open the given video file!");
				return WME_E_FAIL;
			}

		}

		return WME_S_OK;
	}

	WMERESULT CFileCaptureEngineImp::CloseVideoFile()
	{
		if(!m_pVideoFile)
		{
			return WME_S_OK;
		}

		m_ReadingLock.Lock();

		fclose(m_pVideoFile);
		m_pVideoFile = NULL;

		m_ReadingLock.UnLock();

		return WME_S_OK;
	}

	WMERESULT CFileCaptureEngineImp::ReadVideoFrame()
	{
		WMERESULT ret = WME_S_OK;

		if(!m_pVideoFile)
		{
			ret = OpenVideoFile();
			if(WME_FAILED(ret))
			{
				return WME_E_FAIL;
			}
		}

		int nFrameSize = 0;
		CalcVideoFrameSize(nFrameSize);

		if(nFrameSize <= 0)
		{
			CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::ReadVideoFrame(): frame size error!");
			return WME_E_FAIL;
		}

		if(m_nReadingBufferLen < nFrameSize)
		{
			if(m_pReadingBuffer)
			{
				delete []m_pReadingBuffer;
				m_pReadingBuffer = NULL;
			}
			m_nReadingBufferLen = nFrameSize;
		}

		if(!m_pReadingBuffer)
		{
			m_pReadingBuffer = new char[m_nReadingBufferLen];

			if(!m_pReadingBuffer)
			{
				CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::ReadVideoFrame(): failed to allocate memory!");
				return WME_E_FAIL;
			}
		}

		int nReadLen = fread(m_pReadingBuffer, 1, nFrameSize, m_pVideoFile);
		if(nReadLen != nFrameSize)
		{
			fseek (m_pVideoFile , 0 , SEEK_SET);
			nReadLen = fread(m_pReadingBuffer, 1, nFrameSize, m_pVideoFile);
		}

		if(nReadLen != nFrameSize)
		{
			CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::ReadVideoFrame(): failed to read video file!");
			return WME_E_FAIL;
		}

		m_nDataLen = nFrameSize;

		return WME_S_OK;

	}

	WMERESULT CFileCaptureEngineImp::CalcVideoFrameSize(int &nSize)
	{
		nSize = 0;
		float nFactor = 0;
		switch (m_sVideoRawFormat.eRawType)
		{
		case WmeVideoUnknown:
			{
				nFactor = 0;
			}
			break;
		case WmeI420:
		case WmeYV12:
		case WmeNV12:
		case WmeNV21:
			{
				nFactor = 1.5;
			}
			break;
		case WmeYUY2:
			{
				nFactor = 2;
			}
			break;

		case WmeRGB24:
		case WmeBGR24:
		case WmeRGB24Flip:
		case WmeBGR24Flip:
			{
				nFactor = 3;
			}
			break;

		case WmeRGBA32:
		case WmeBGRA32:
		case WmeARGB32:
		case WmeABGR32:	
		case WmeRGBA32Flip:
		case WmeBGRA32Flip:
		case WmeARGB32Flip:
		case WmeABGR32Flip:
			{
				nFactor = 4;
			}
			break;

		default:
			nFactor = 0;
			break;
		}

		nSize = (int)(m_sVideoRawFormat.iWidth*m_sVideoRawFormat.iHeight*nFactor);

		return WME_S_OK;
	}

	void CFileCaptureEngineImp::SetExternalInputter(IWmeExternalInputter *pExternalInputter)
	{
		if(m_pExternalInputter)
		{
			m_pExternalInputter->Release();
			m_pExternalInputter = NULL;
		}

		if(pExternalInputter)
		{
			m_pExternalInputter = pExternalInputter;
			m_pExternalInputter->AddRef();
		}
		CM_INFO_TRACE_THIS("CFileCaptureEngineImp::SetExternalInputter(): m_pExternalInputter = " << m_pExternalInputter << ", ");

		return;
	}
}
