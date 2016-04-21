#include "gtest/gtest.h"
#include "CmDebug.h"
#include <string.h>
#include <stdio.h>
#include "CmTraceFromT120.h"

class CCmTraceFormTest : public CCmT120Trace
{
public:
    CCmTraceFormTest(char* lpszModule, 
                     char* lpszPrivateInfo = NULL, unsigned char bShared = FALSE) : CCmT120Trace(lpszModule,
                                 lpszPrivateInfo, bShared) {
        load();
    }
    ~CCmTraceFormTest() {}
};

class CCmTraceUnitTest : public testing::Test
{
public:
	CCmTraceUnitTest() {
	}

	virtual ~CCmTraceUnitTest() {
	}

	virtual void SetUp() {

		m_pTraceTest = new CCmTraceFormTest((char *)"Log", NULL, FALSE);
	}

	virtual void TearDown() {
		if (NULL != m_pTraceTest) {
			delete m_pTraceTest;
			m_pTraceTest = NULL;
		}
	}
protected:
	CCmTraceFormTest * m_pTraceTest;
};
