#ifndef WME_TRACE_SINK_H
#define WME_TRACE_SINK_H

#include "WmeCommonTypes.h"
#include "WmeError.h"

namespace wme {

class IWmeTraceSink {
public:
	virtual WMERESULT OnTrace(uint32_t trace_level, const char* szInfo) = 0;
};

}		//namespace



#endif	//WME_TRACE_SINK_H