#ifndef _APPSHARE_FAKE_COMMON_H_
#define _APPSHARE_FAKE_COMMON_H_


#include "share_source.h"
#include "share_pointing.h"
#include "WmeUnknownImpTemplate.h"
#include "share_util.h"

class FakeSharePointing : public ISharePointing
{
public:
    FakeSharePointing() {}
    virtual ~FakeSharePointing() {}

    virtual WBXResult Start() { return WBX_SUCCESS;};
    virtual WBXResult Stop() { return WBX_SUCCESS;};

    virtual WBXResult CapturePoint(SHARE_FRAME_FORMAT eSHARE_FRAME_FORMAT=SHARE_FRAME_FORMAT_RGBA) { return WBX_ERR_NO_IMPLEMENT;};

    virtual WBXResult AddSink(ISharePointingSink *pISharePointingSink) {return m_sinkMgr.AddItem(pISharePointingSink);}
    virtual WBXResult RemoveSink(ISharePointingSink *pISharePointingSink) {return m_sinkMgr.RemoveItem(pISharePointingSink);}
    virtual WBXResult CleanSink() {return m_sinkMgr.CleanItems();}
    WBXResult SetBaseDisplayID(DisplayID displayID) { return WBX_SUCCESS; }
protected:
    WbxUniquePointArrayT<ISharePointingSink> m_sinkMgr;
};

#define UT_CAPTURE_WIDTH 1024
#define UT_CAPTURE_HEIGH 768
#define UT_CAPTURE_BUF_LEN UT_CAPTURE_WIDTH*UT_CAPTURE_HEIGH*4

class FakeShareSource : public IShareSource , public wme::CWmeUnknownImpl
{
public:
    IMPLEMENT_WME_REFERENCE
    IMPLEMENT_WME_QUERY_INTERFACE_UNKNOWN

    FakeShareSource() { m_bShared=WBXFalse; }
    virtual ~FakeShareSource() {}
    virtual WBXInt32 GetSourceUniqueId() {return kFullDisplayID;}
    virtual ShareSourceType GetSourceType() {return SST_MONITOR;};
    virtual WBXBool IsShared() {return m_bShared;};
    virtual WBXRect GetSourceRect() {WBXRect rc(0,0,UT_CAPTURE_WIDTH,UT_CAPTURE_HEIGH); return rc;};

    WBXVoid SetShared(WBXBool bShared) {m_bShared=bShared;};
protected:
    WBXBool m_bShared;
};


#endif // _APPSHARE_FAKE_COMMON_H_