

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "CmBaseHTTPExProtoHandle.h"


TEST(create_basehttpex_proto_handle_test, ProtoTypeNULLTest)
{
    ICmBaseHTTPExProtoHandle* pHandle = create_basehttpex_proto_handle(PROTOCOL_TYPE_NULL);
    EXPECT_TRUE(NULL == pHandle);
}

TEST(create_basehttpex_proto_handle_test, ProtoTypeWebSocketTest)
{
    ICmBaseHTTPExProtoHandle* pHandle = create_basehttpex_proto_handle(PROTOCOL_TYPE_WEBSOCKET);
    EXPECT_TRUE(NULL != pHandle);
    
    pHandle->AddReference();
    pHandle->ReleaseReference();
    pHandle = NULL;
}

TEST(create_basehttpex_proto_handle_test, ProtoTypeBoshTest)
{
    ICmBaseHTTPExProtoHandle* pHandle = create_basehttpex_proto_handle(PROTOCOL_TYPE_BOSH);
    EXPECT_TRUE(NULL != pHandle);
    
    pHandle->AddReference();
    pHandle->ReleaseReference();
    pHandle = NULL;
}

TEST(create_basehttpex_proto_handle_test, ProtoTypeRestTest)
{
    ICmBaseHTTPExProtoHandle* pHandle = create_basehttpex_proto_handle(PROTOCOL_TYPE_REST);
    EXPECT_TRUE(NULL != pHandle);
    
    pHandle->AddReference();
    pHandle->ReleaseReference();
    pHandle = NULL;
}