#ifndef _WME_MEDIA_SDK_MOCK_H_
#define _WME_MEDIA_SDK_MOCK_H_

#include "gmock/gmock.h"
#include "WmeInterface.h"

class MockWmeMediaPackage: public wme::IWmeMediaPackage
{
public:
	uint32_t WMECALLTYPE AddRef() { return 1; }
	uint32_t WMECALLTYPE Release() { return 1; }
	WMERESULT WMECALLTYPE QueryInterface(const wme::WMEIID &iid, void **ppvObject) { return WME_E_NOTIMPL; }
	MOCK_METHOD1(GetTimestamp, WMERESULT(uint32_t &uTimestamp));
	MOCK_METHOD1(SetTimestamp, WMERESULT(uint32_t uTimestamp));
	MOCK_METHOD1(GetFormatType, WMERESULT(wme::WmeMediaFormatType &eType));
	MOCK_METHOD1(GetFormatSize, WMERESULT(uint32_t &uSize));
	MOCK_METHOD1(GetFormatPointer, WMERESULT(unsigned char **ppBuffer));
	MOCK_METHOD1(GetDataSize, WMERESULT(uint32_t &uSize));
	MOCK_METHOD1(GetDataPointer, WMERESULT(unsigned char **ppBuffer));
	MOCK_METHOD1(GetDataOffset, WMERESULT(uint32_t &uOffset));
	MOCK_METHOD1(GetDataLength, WMERESULT(uint32_t &uLength));
	MOCK_METHOD1(SetDataOffset, WMERESULT(uint32_t uOffset));
	MOCK_METHOD1(SetDataLength, WMERESULT(uint32_t uLength));
};

#endif // _WME_MEDIA_SDK_MOCK_H_