#ifndef WME_FILE_CAPTURE_ENGINE_IMP_H
#define WME_FILE_CAPTURE_ENGINE_IMP_H

#include <string>
#ifndef WIN32
#include <sys/time.h>
#endif

#include "WmeTrack.h"
#include "CmThread.h"
#include "CmTimerWrapperID.h"
#include "CmMutex.h"

namespace wme
{
	class CFileCaptureEngineImp: public CCmTimerWrapperIDSink
	{
	public:

		CFileCaptureEngineImp();
		~CFileCaptureEngineImp();


		virtual WMERESULT SetVideoFile(const char *pFileName, WmeVideoRawFormat *pFormat) ;
		virtual WMERESULT Start() ;
		virtual WMERESULT Stop() ;
		virtual WMERESULT Init();
		virtual WMERESULT Uninit();
		virtual WMERESULT ResetFrameRate(float fFrameRate) ;

		//timer 
		CmResult StartTimer();
		CmResult StopTimer();
		void OnTimer(CCmTimerWrapperID* aId); 

		//
		void SetExternalInputter(IWmeExternalInputter *pExternalInputter);
	protected:
		WMERESULT ResetTimer();
		WMERESULT OpenVideoFile();
		WMERESULT CloseVideoFile();
		WMERESULT ReadVideoFrame();
		WMERESULT CalcVideoFrameSize(int &nSize);
		
	private:
		std::string m_strFileName;
		WmeVideoRawFormat m_sVideoRawFormat;
		FILE *m_pVideoFile;
		BOOL m_bStarted;
		BOOL m_bStoped;
		BOOL m_bInit;
		//////////////////////////////////////////////////////////////////////////
		//for statistic
		friend class CStartEvent;
		friend class CStopEvent;

		class CStartEvent : public ICmEvent
		{
		public:
			CStartEvent(CFileCaptureEngineImp& scheduler) : m_readingScheduler(scheduler) {}
			~CStartEvent() {}

			// functions from ICmEvent
			virtual CmResult OnEventFire() {
				return m_readingScheduler.StartTimer();
			}

		private:
			CFileCaptureEngineImp& m_readingScheduler;
		};

		class CStopEvent : public ICmEvent
		{
		public:
			CStopEvent(CFileCaptureEngineImp& scheduler) : m_readingScheduler(scheduler) {}
			~CStopEvent() {}

			// functions from ICmEvent
			virtual CmResult OnEventFire() {
				return m_readingScheduler.StopTimer();
			}

		private:
			CFileCaptureEngineImp& m_readingScheduler;
		};

		ACmThread*         m_pReadingThread;
		CCmTimerWrapperID  m_Timer;
		BOOL m_bStartedTimer;
		CCmMutexThreadRecursive  m_ReadingLock;
		char *m_pReadingBuffer;
		int m_nReadingBufferLen;
		int m_nDataLen;

		IWmeExternalInputter *m_pExternalInputter;

	};
}
#endif