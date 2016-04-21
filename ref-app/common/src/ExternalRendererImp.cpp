#include "ExternalRendererImp.h"
#include "CmDebug.h"

namespace wme
{
	CExternalRendererImp::CExternalRendererImp()
	{
		m_strFileName = "";
		m_pVideoFile = NULL;
	}

	CExternalRendererImp::~CExternalRendererImp()
	{
		CloseVideoFile();
	}

	WMERESULT CExternalRendererImp::SetVideoFile(const char *pFileName)
	{
		if(!pFileName)
		{
			CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::SetVideoFile, invalid file name or video format ");
			return WME_E_INVALIDARG;
		}

		m_strFileName = pFileName;


		CloseVideoFile();

		OpenVideoFile();

		return WME_S_OK;
	}

	WMERESULT CExternalRendererImp::OpenVideoFile()
	{
		if(!m_pVideoFile)
		{
			if(m_strFileName.length() == 0)
			{
				CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::Start(): no video file!");
				return WME_E_FAIL;
			}

			m_WriteLock.Lock();
#ifdef CM_WIN32
			errno_t err = fopen_s(&m_pVideoFile, m_strFileName.c_str(), "a+b");
#else
			m_pVideoFile = fopen(m_strFileName.c_str(), "a+b");
#endif
			m_WriteLock.UnLock();

			if(!m_pVideoFile)
			{
				CM_ERROR_TRACE_THIS("CFileCaptureEngineImp::Start(): failed to open the given video file!");
				return WME_E_FAIL;
			}

		}

		return WME_S_OK;
	}

	WMERESULT CExternalRendererImp::CloseVideoFile()
	{
		if(!m_pVideoFile)
		{
			return WME_S_OK;
		}

		m_WriteLock.Lock();

		fclose(m_pVideoFile);
		m_pVideoFile = NULL;

		m_WriteLock.UnLock();

		return WME_S_OK;
	}

	WMERESULT CExternalRendererImp::WriteVideoData(char *pData, int nLen)
	{
		WMERESULT ret = WME_S_OK;

		if(!m_pVideoFile)
		{
			ret = OpenVideoFile();
			if(!m_pVideoFile)
			{
				return WME_E_FAIL;
			}
		}

		if(!pData)
		{
			return WME_E_INVALIDARG;
		}
	
		m_WriteLock.Lock();
		int nWriteLen = fwrite(pData, 1, nLen, m_pVideoFile);
		fflush(m_pVideoFile);
		m_WriteLock.UnLock();

		if(nWriteLen != nLen)
		{
			CM_ERROR_TRACE_THIS("CExternalRendererImp::WriteVideoFrame(): failed to write data to video file!");
			return WME_E_FAIL;
		}

		return WME_S_OK;
	}

	WMERESULT CExternalRendererImp::RenderMediaData(IWmeMediaPackage *pPackage)
	{
		if(!pPackage)
		{
			CM_ERROR_TRACE_THIS("CExternalRendererImp::RenderMediaData(): pPackage = NULL!");
			return WME_E_INVALIDARG;
		}

		unsigned char *pData = NULL;
		WMERESULT ret = pPackage->GetDataPointer(&pData);

		if(WME_FAILED(ret))
		{
			CM_ERROR_TRACE_THIS("CExternalRendererImp::RenderMediaData(): Failed to get data point, pPackage = " << pPackage);
			return WME_E_INVALIDARG;
		}

		uint32_t nDataLen = 0;
		nDataLen = pPackage->GetDataLength(nDataLen);

		WmeMediaFormatType eType;
		ret = pPackage->GetFormatType(eType);

		uint32_t nFormatSize = 0;
		ret = pPackage->GetFormatSize(nFormatSize);

		
		unsigned char* pFormatBuffer = NULL;
		ret = pPackage->GetFormatPointer(&pFormatBuffer);

		uint32_t nType = eType;

		ret = WriteVideoData((char *)&nType, sizeof(uint32_t));
		ret = WriteVideoData((char *)pFormatBuffer, nFormatSize);
		ret = WriteVideoData((char *)pData, nDataLen);

		return ret;
	}
}