#include "WMEInterfaceMock.h"
#include "rtputils.h"
#include "gtest/gtest.h"


using namespace testing;

CWmeMediaPackageAllocatorMock CWmeMediaPackageAllocatorMock::s_mediaPackageAllocator;

CWmeMediaPackageAllocatorMock *CWmeMediaPackageAllocatorMock::GetInstance()
{
    return &s_mediaPackageAllocator;
}

void ConvertMediaPackageToMessageBlock(IRTPMediaPackage *rtpPackage, CCmMessageBlock *&mb)
{
    ASSERT_TRUE(rtpPackage != nullptr);
    mb = nullptr;

    WMERESULT wmeRet = WME_S_OK;
    unsigned char *pBuffer = nullptr;
    uint32_t bufferOffset = 0;
    uint32_t bufferLen = 0;

    wmeRet = rtpPackage->GetDataPointer(&pBuffer);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));

    wmeRet = rtpPackage->GetDataOffset(bufferOffset);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));

    wmeRet = rtpPackage->GetDataLength(bufferLen);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));

    CCmMessageBlock tmpMB(bufferLen, (LPCSTR)pBuffer+bufferOffset, 0, bufferLen);
    mb = tmpMB.DuplicateChained();
}

void ConvertMessageBlockToMediaPackage(CCmMessageBlock &mb, IRTPMediaPackage **rtpPackage, bool isRTPPacket)
{
    ASSERT_TRUE(rtpPackage != nullptr);
    *rtpPackage = nullptr;

    uint32_t formatSize = 0;
    uint32_t dataSize   = mb.GetTopLevelLength();
    IRTPMediaPackage *mediaPackage = 0;
    RTPMediaFormatType formatType = isRTPPacket ? RTPMediaFormatRTP : RTPMediaFormatRTCP;

    WMERESULT wmeRet = WME_E_FAIL;
    wmeRet = CWmeMediaPackageAllocatorMock::GetInstance()->GetPackage(formatType, formatSize, dataSize, &mediaPackage);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));
    mediaPackage->SetTimestamp(TickNowMS());

    unsigned char *buffer = nullptr;
    wmeRet = mediaPackage->GetDataPointer(&buffer);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRet));

    memcpy(buffer, mb.GetTopLevelReadPtr(), dataSize);
    mediaPackage->SetDataLength(dataSize);

    *rtpPackage = mediaPackage;
}