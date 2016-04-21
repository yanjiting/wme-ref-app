#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::StrictMock;
using namespace std;

#include <string>
#include "wrtpclientapi.h"
#include "wrtptrace.h"
#include "mari/wqos/qostrace.h"
#include "rtpinternal.h"
#include "CmDebug.h"
#include "mari/rsfec/fectrace.h"

#if 0
//annotate by Joyce--these cases will redirect wrtp trace sink and set it by Null
class CWme : public IWrtpTraceSink
{
public:
    CWme() {};
    virtual ~CWme() {};

public:
    virtual long SinkTrace(unsigned long trace_level, const char *trace_info, int len)
    {
        std::cout << "Trace_level:" << trace_level << ", Length:" << len << ", Info:" << trace_info <<std::endl;
        return 0;
    }
};


class CMockWme : public CWme
{
public:
    CMockWme() {}
    virtual ~CMockWme() {}
public:
    MOCK_METHOD3(SinkTrace, long(unsigned long trace_level, const char *trace_info, int len));
};

class CNewTraceInterfaceTest : public testing::Test
{
public:
    CNewTraceInterfaceTest() {}

protected:
    //will be called before running every case
    virtual void SetUp()
    {
    }
    //will be called after running every case
    virtual void TearDown()
    {
    }

    //will be called before running the first case
    static void SetUpTestCase()
    {
    }

    //will be called after running the last case
    static void TearDownTestCase()
    {
    }
private:

};

const char *GetObjectKey()
{
    return "";
}

TEST(CNewTraceInterfaceTest, TestWebexSquareLogInterface_WRTP)
{
    SetWRTPExternalTraceSink(nullptr);
    static NiceMock<CMockWme> wme_client;
    const char *str = "Hello World!";
    SetWRTPExternalTraceSink(&wme_client);

    SetWRTPExternalTraceMask(0);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(1);

    WRTP_INFOXTRACE(str);
    WRTP_WARNXTRACE(str);
    WRTP_ERRXTRACE(str);
    WRTP_DEBUGXTRACE(str);
    WRTP_DTLXTRACE(str);

    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(1);

    WRTP_INFOTRACE(str);
    WRTP_WARNTRACE(str);
    WRTP_ERRTRACE(str);
    WRTP_DEBUGTRACE(str);
    WRTP_DTLTRACE(str);

    SetWRTPExternalTraceMask(10);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(2);

    WRTP_INFOXTRACE(str);
    WRTP_WARNXTRACE(str);
    WRTP_ERRXTRACE(str);
    WRTP_DEBUGXTRACE(str);
    WRTP_DTLXTRACE(str);

    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(2);

    WRTP_INFOTRACE(str);
    WRTP_WARNTRACE(str);
    WRTP_ERRTRACE(str);
    WRTP_DEBUGTRACE(str);
    WRTP_DTLTRACE(str);

    SetWRTPExternalTraceMask(20);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(3);

    WRTP_INFOXTRACE(str);
    WRTP_WARNXTRACE(str);
    WRTP_ERRXTRACE(str);
    WRTP_DEBUGXTRACE(str);
    WRTP_DTLXTRACE(str);

    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(3);

    WRTP_INFOTRACE(str);
    WRTP_WARNTRACE(str);
    WRTP_ERRTRACE(str);
    WRTP_DEBUGTRACE(str);
    WRTP_DTLTRACE(str);

    SetWRTPExternalTraceMask(27);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(4);

    WRTP_INFOXTRACE(str);
    WRTP_WARNXTRACE(str);
    WRTP_ERRXTRACE(str);
    WRTP_DEBUGXTRACE(str);
    WRTP_DTLXTRACE(str);

    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(4);

    WRTP_INFOTRACE(str);
    WRTP_WARNTRACE(str);
    WRTP_ERRTRACE(str);
    WRTP_DEBUGTRACE(str);
    WRTP_DTLTRACE(str);

    SetWRTPExternalTraceMask(25);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(5);

    WRTP_INFOXTRACE(str);
    WRTP_WARNXTRACE(str);
    WRTP_ERRXTRACE(str);
    WRTP_DEBUGXTRACE(str);
    WRTP_DTLXTRACE(str);

    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(5);

    WRTP_INFOTRACE(str);
    WRTP_WARNTRACE(str);
    WRTP_ERRTRACE(str);
    WRTP_DEBUGTRACE(str);
    WRTP_DTLTRACE(str);
    SetWRTPExternalTraceSink(nullptr);
}

TEST(CNewTraceInterfaceTest, TestWebexSquareLogInterface_QoS)
{
    SetWRTPExternalTraceSink(nullptr);
    static NiceMock<CMockWme> wme_client;
    const char *str = "Hello World!";
    SetWRTPExternalTraceSink(&wme_client);

    SetWRTPExternalTraceMask(0);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(1);

    QOS_INFOTRACE(str);
    QOS_WARNTRACE(str);
    QOS_ERRTRACE(str);
    QOS_DEBUGTRACE(str);
    QOS_DTLTRACE(str);

    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(1);
    QOS_INFOTRACE_THIS(str);
    QOS_WARNTRACE_THIS(str);
    QOS_ERRTRACE_THIS(str);
    QOS_DEBUGTRACE_THIS(str);
    QOS_DTLTRACE_THIS(str);


    SetWRTPExternalTraceMask(10);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(2);

    QOS_INFOTRACE(str);
    QOS_WARNTRACE(str);
    QOS_ERRTRACE(str);
    QOS_DEBUGTRACE(str);
    QOS_DTLTRACE(str);

    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(2);
    QOS_INFOTRACE_THIS(str);
    QOS_WARNTRACE_THIS(str);
    QOS_ERRTRACE_THIS(str);
    QOS_DEBUGTRACE_THIS(str);
    QOS_DTLTRACE_THIS(str);


    SetWRTPExternalTraceMask(20);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(3);

    QOS_INFOTRACE(str);
    QOS_WARNTRACE(str);
    QOS_ERRTRACE(str);
    QOS_DEBUGTRACE(str);
    QOS_DTLTRACE(str);

    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(3);
    QOS_INFOTRACE_THIS(str);
    QOS_WARNTRACE_THIS(str);
    QOS_ERRTRACE_THIS(str);
    QOS_DEBUGTRACE_THIS(str);
    QOS_DTLTRACE_THIS(str);



    SetWRTPExternalTraceMask(27);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(4);

    QOS_INFOTRACE(str);
    QOS_WARNTRACE(str);
    QOS_ERRTRACE(str);
    QOS_DEBUGTRACE(str);
    QOS_DTLTRACE(str);

    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(4);
    QOS_INFOTRACE_THIS(str);
    QOS_WARNTRACE_THIS(str);
    QOS_ERRTRACE_THIS(str);
    QOS_DEBUGTRACE_THIS(str);
    QOS_DTLTRACE_THIS(str);



    SetWRTPExternalTraceMask(25);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(5);

    QOS_INFOTRACE(str);
    QOS_WARNTRACE(str);
    QOS_ERRTRACE(str);
    QOS_DEBUGTRACE(str);
    QOS_DTLTRACE(str);

    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(5);
    QOS_INFOTRACE_THIS(str);
    QOS_WARNTRACE_THIS(str);
    QOS_ERRTRACE_THIS(str);
    QOS_DEBUGTRACE_THIS(str);
    QOS_DTLTRACE_THIS(str);
    SetWRTPExternalTraceSink(nullptr);
}

TEST(CNewTraceInterfaceTest, TestWebexSquareLogInterface_TP)
{
    SetWRTPExternalTraceSink(nullptr);
    static NiceMock<CMockWme> wme_client;
    const char *str = "Hello World!";
    SetWRTPExternalTraceSink(&wme_client);
    SetWRTPExternalTraceMask(0);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(1);
    CM_INFO_TRACE(str);
    CM_WARNING_TRACE(str);
    CM_ERROR_TRACE(str);
    CM_DETAIL_TRACE(str);
    CM_DEBUG_TRACE(str);

    SetWRTPExternalTraceMask(10);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(2);
    CM_INFO_TRACE(str);
    CM_WARNING_TRACE(str);
    CM_ERROR_TRACE(str);
    CM_DETAIL_TRACE(str);
    CM_DEBUG_TRACE(str);

    SetWRTPExternalTraceMask(20);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(3);
    CM_INFO_TRACE(str);
    CM_WARNING_TRACE(str);
    CM_ERROR_TRACE(str);
    CM_DETAIL_TRACE(str);
    CM_DEBUG_TRACE(str);

    SetWRTPExternalTraceMask(27);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(4);
    CM_INFO_TRACE(str);
    CM_WARNING_TRACE(str);
    CM_ERROR_TRACE(str);
    CM_DETAIL_TRACE(str);
    CM_DEBUG_TRACE(str);

    SetWRTPExternalTraceMask(25);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(5);
    CM_INFO_TRACE(str);
    CM_WARNING_TRACE(str);
    CM_ERROR_TRACE(str);
    CM_DETAIL_TRACE(str);
    CM_DEBUG_TRACE(str);
    SetWRTPExternalTraceSink(nullptr);
}

TEST(CNewTraceInterfaceTest, TestWebexSquareLogInterface_FEC)
{
    SetWRTPExternalTraceSink(nullptr);
    static NiceMock<CMockWme> wme_client;
    SetWRTPExternalTraceSink(&wme_client);
    const char *str = "Hello World!";
    SetWRTPExternalTraceMask(0);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(1);

    FEC_INFO_TRACE(str);
    FEC_WARNING_TRACE(str);
    FEC_ERROR_TRACE(str);
    FEC_DEBUG_TRACE(str);
    FEC_DETAIL_TRACE(str);

    SetWRTPExternalTraceMask(10);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(2);

    FEC_INFO_TRACE(str);
    FEC_WARNING_TRACE(str);
    FEC_ERROR_TRACE(str);
    FEC_DEBUG_TRACE(str);
    FEC_DETAIL_TRACE(str);

    SetWRTPExternalTraceMask(20);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(3);

    FEC_INFO_TRACE(str);
    FEC_WARNING_TRACE(str);
    FEC_ERROR_TRACE(str);
    FEC_DEBUG_TRACE(str);
    FEC_DETAIL_TRACE(str);

    SetWRTPExternalTraceMask(27);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(4);

    FEC_INFO_TRACE(str);
    FEC_WARNING_TRACE(str);
    FEC_ERROR_TRACE(str);
    FEC_DEBUG_TRACE(str);
    FEC_DETAIL_TRACE(str);

    SetWRTPExternalTraceMask(25);
    EXPECT_CALL(wme_client, SinkTrace(_, _, _)).Times(5);

    FEC_INFO_TRACE(str);
    FEC_WARNING_TRACE(str);
    FEC_ERROR_TRACE(str);
    FEC_DEBUG_TRACE(str);
    FEC_DETAIL_TRACE(str);

    SetWRTPExternalTraceSink(nullptr);
}

#endif