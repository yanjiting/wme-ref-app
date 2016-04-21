#ifndef WME_EXTERNAL_RENDERER_IMP_H
#define WME_EXTERNAL_RENDERER_IMP_H

#include <string>
#ifndef WIN32
#include <sys/time.h>
#endif

#include "CmThread.h"
#include "CmTimerWrapperID.h"
#include "CmMutex.h"
#include "WmeExternalRenderer.h"
#include "UnknownInterfaceImp.h"

namespace wme
{
	class CExternalRendererImp: public IWmeExternalRenderer, public CUnknownInterfaceImp
	{
	public:
		IMPLEMENT_UNKNOWN_REFERENCE
		CExternalRendererImp();
		~CExternalRendererImp();

		virtual WMERESULT RenderMediaData(IWmeMediaPackage *pPackage);
		virtual WMERESULT SetVideoFile(const char *pFileName) ;

	protected:
		WMERESULT OpenVideoFile();
		WMERESULT CloseVideoFile();
		WMERESULT WriteVideoData(char *pData, int nLen);
		
	private:
		std::string m_strFileName;
		FILE *m_pVideoFile;
		CCmMutexThreadRecursive  m_WriteLock;
	};
}
#endif