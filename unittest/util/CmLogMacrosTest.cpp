//
//  CmLogMacrosTest.cpp
//  utilTest_mac
//
//  Created by Peng Yu on 3/3/16.
//  Copyright © 2016 cisco. All rights reserved.
//

#include <mutex>
#include <map>
#include <string>

#include "gtest/gtest.h"
#include "CmDebug.h"

int counter = 0;
unsigned long test_count;
unsigned long test_level;
std::string test_log;
void log_sink(unsigned long trace_level, char *trace_info, unsigned int len) {
    std::string logInput(trace_info, len);
    test_level = trace_level;
    test_log = logInput;
    ++test_count;
}

class CmLogMacrosTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        counter = 0;
        test_count = 0;
        test_level = -1;
        test_log = "";
        set_external_trace_mask(CM_TRACE_LEVEL_ALL);
        set_util_external_trace_sink(log_sink);
    }

    virtual void TearDown()
    {
        set_util_external_trace_sink(nullptr);
        set_external_trace_mask(CM_TRACE_LEVEL_INFO);
    }

};

unsigned logN = 2;
const char *exModule = "ititnzfvnwie";
void LoglogN(unsigned long logLevel, std::string &inputlog) {
    switch (logLevel) {
        case CM_TRACE_LEVEL_ERROR: {
            CM_EVERY_N_ERROR_TRACE(logN, inputlog); break;
        }
        case CM_TRACE_LEVEL_WARNING: {
            CM_EVERY_N_WARNING_TRACE(logN, inputlog); break;
        }
        case CM_TRACE_LEVEL_INFO: {
            CM_EVERY_N_INFO_TRACE(logN, inputlog); break;
        }
        case CM_TRACE_LEVEL_DEBUG: {
            CM_EVERY_N_DEBUG_TRACE(logN, inputlog); break;
        }
        case CM_TRACE_LEVEL_DETAIL: {
            CM_EVERY_N_DETAIL_TRACE(logN, inputlog); break;
        }
    }
}
void LoglogN_EX(unsigned long logLevel, std::string &inputlog) {
    switch (logLevel) {
        case CM_TRACE_LEVEL_ERROR: {
            CM_EVERY_N_ERROR_TRACE_EX(logN, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_WARNING: {
            CM_EVERY_N_WARNING_TRACE_EX(logN, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_INFO: {
            CM_EVERY_N_INFO_TRACE_EX(logN, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_DEBUG: {
            CM_EVERY_N_DEBUG_TRACE_EX(logN, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_DETAIL: {
            CM_EVERY_N_DETAIL_TRACE_EX(logN, exModule, inputlog); break;
        }
    }
}

void LogIflogN(bool condition, unsigned long logLevel, std::string &inputlog) {
    switch (logLevel) {
        case CM_TRACE_LEVEL_ERROR: {
            CM_IF_EVERY_N_ERROR_TRACE(condition == true, logN, inputlog); break;
        }
        case CM_TRACE_LEVEL_WARNING: {
            CM_IF_EVERY_N_WARNING_TRACE(condition == true, logN, inputlog); break;
        }
        case CM_TRACE_LEVEL_INFO: {
            CM_IF_EVERY_N_INFO_TRACE(condition == true, logN, inputlog); break;
        }
        case CM_TRACE_LEVEL_DEBUG: {
            CM_IF_EVERY_N_DEBUG_TRACE(condition == true, logN, inputlog); break;
        }
        case CM_TRACE_LEVEL_DETAIL: {
            CM_IF_EVERY_N_DETAIL_TRACE(condition == true, logN, inputlog); break;
        }
    }
}
void LogIflogN_EX(bool condition, unsigned long logLevel, std::string &inputlog) {
    switch (logLevel) {
        case CM_TRACE_LEVEL_ERROR: {
            CM_IF_EVERY_N_ERROR_TRACE_EX(condition == true, logN, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_WARNING: {
            CM_IF_EVERY_N_WARNING_TRACE_EX(condition == true, logN, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_INFO: {
            CM_IF_EVERY_N_INFO_TRACE_EX(condition == true, logN, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_DEBUG: {
            CM_IF_EVERY_N_DEBUG_TRACE_EX(condition == true, logN, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_DETAIL: {
            CM_IF_EVERY_N_DETAIL_TRACE_EX(condition == true, logN, exModule, inputlog); break;
        }
    }
}

void LogFirstN(unsigned long logLevel, std::string &inputlog) {
    switch (logLevel) {
        case CM_TRACE_LEVEL_ERROR: {
            CM_FIRST_N_ERROR_TRACE(&counter, logN, inputlog); break;
        }
        case CM_TRACE_LEVEL_WARNING: {
            CM_FIRST_N_WARNING_TRACE(&counter, logN, inputlog); break;
        }
        case CM_TRACE_LEVEL_INFO: {
            CM_FIRST_N_INFO_TRACE(&counter, logN, inputlog); break;
        }
        case CM_TRACE_LEVEL_DEBUG: {
            CM_FIRST_N_DEBUG_TRACE(&counter, logN, inputlog); break;
        }
        case CM_TRACE_LEVEL_DETAIL: {
            CM_FIRST_N_DETAIL_TRACE(&counter, logN, inputlog); break;
        }
    }
}
void LogFirstN_EX(unsigned long logLevel, std::string &inputlog) {
    switch (logLevel) {
        case CM_TRACE_LEVEL_ERROR: {
            CM_FIRST_N_ERROR_TRACE_EX(&counter, logN, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_WARNING: {
            CM_FIRST_N_WARNING_TRACE_EX(&counter, logN, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_INFO: {
            CM_FIRST_N_INFO_TRACE_EX(&counter, logN, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_DEBUG: {
            CM_FIRST_N_DEBUG_TRACE_EX(&counter, logN, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_DETAIL: {
            CM_FIRST_N_DETAIL_TRACE_EX(&counter, logN, exModule, inputlog); break;
        }
    }
}

void LogIf(bool condition, unsigned long logLevel, std::string &inputlog) {
    switch (logLevel) {
        case CM_TRACE_LEVEL_ERROR: {
            CM_IF_ERROR_TRACE(condition == true, inputlog); break;
        }
        case CM_TRACE_LEVEL_WARNING: {
            CM_IF_WARNING_TRACE(condition == true, inputlog); break;
        }
        case CM_TRACE_LEVEL_INFO: {
            CM_IF_INFO_TRACE(condition == true, inputlog); break;
        }
        case CM_TRACE_LEVEL_DEBUG: {
            CM_IF_DEBUG_TRACE(condition == true, inputlog); break;
        }
        case CM_TRACE_LEVEL_DETAIL: {
            CM_IF_DETAIL_TRACE(condition == true, inputlog); break;
        }
    }
}
void LogIf_EX(bool condition, unsigned long logLevel, std::string &inputlog) {
    switch (logLevel) {
        case CM_TRACE_LEVEL_ERROR: {
            CM_IF_ERROR_TRACE_EX(condition == true, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_WARNING: {
            CM_IF_WARNING_TRACE_EX(condition == true, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_INFO: {
            CM_IF_INFO_TRACE_EX(condition == true, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_DEBUG: {
            CM_IF_DEBUG_TRACE_EX(condition == true, exModule, inputlog); break;
        }
        case CM_TRACE_LEVEL_DETAIL: {
            CM_IF_DETAIL_TRACE_EX(condition == true, exModule, inputlog); break;
        }
    }
}

typedef enum {
    IF,
    EVERY_N,
    IF_EVERY_N,
    FIRST_N,
    IF_EX,
    EVERY_N_EX,
    IF_EVERY_N_EX,
    FIRST_N_EX
} LogMacroType;

void TestLogMacro(LogMacroType logType, unsigned long logLevel) {
    std::string inputlog = std::to_string(logLevel)+"logmacrotest"+std::to_string(logLevel);

    switch (logType) {
        case IF: {
            LogIf(false, logLevel, test_log);
            EXPECT_EQ(0, test_count);
            LogIf(true, logLevel, test_log);
            EXPECT_EQ(1, test_count);
            LogIf(false, logLevel, test_log);
            EXPECT_EQ(1, test_count);
            LogIf(true, logLevel, inputlog);
            EXPECT_EQ(2, test_count);
            EXPECT_NE(std::string::npos, test_log.find(inputlog));
            break;
        }
        case EVERY_N: {
            LoglogN(logLevel, inputlog);
            EXPECT_EQ(logLevel, test_level);
            EXPECT_NE(std::string::npos, test_log.find(inputlog));
            LoglogN(logLevel, inputlog);
            EXPECT_EQ(1, test_count);
            LoglogN(logLevel, inputlog);
            EXPECT_EQ(2, test_count);
            break;
        }
        case IF_EVERY_N: {
            LogIflogN(true, logLevel, inputlog);
            EXPECT_EQ(logLevel, test_level);
            EXPECT_NE(std::string::npos, test_log.find(inputlog));
            LogIflogN(true, logLevel, inputlog);
            EXPECT_EQ(1, test_count);
            LogIflogN(true, logLevel, inputlog);
            EXPECT_EQ(2, test_count);
            std::string notexistLog = inputlog+"notexist";
            LogIflogN(false, logLevel, notexistLog);
            EXPECT_EQ(std::string::npos, test_log.find("notexist"));
            EXPECT_EQ(2, test_count);
            break;
        }
        case FIRST_N: {
            LogFirstN(logLevel, inputlog);
            EXPECT_EQ(logLevel, test_level);
            EXPECT_NE(std::string::npos, test_log.find(inputlog));
            LogFirstN(logLevel, inputlog);
            EXPECT_EQ(2, test_count);
            LogFirstN(logLevel, inputlog);
            EXPECT_EQ(2, test_count);
            break;
        }
        case IF_EX: {
            LogIf_EX(false, logLevel, test_log);
            EXPECT_EQ(0, test_count);
            LogIf_EX(true, logLevel, test_log);
            EXPECT_EQ(1, test_count);
            EXPECT_NE(std::string::npos, test_log.find(exModule));
            LogIf_EX(false, logLevel, test_log);
            EXPECT_EQ(1, test_count);
            LogIf_EX(true, logLevel, inputlog);
            EXPECT_EQ(2, test_count);
            EXPECT_NE(std::string::npos, test_log.find(inputlog));
            break;
        }
        case EVERY_N_EX: {
            LoglogN_EX(logLevel, inputlog);
            EXPECT_EQ(logLevel, test_level);
            EXPECT_NE(std::string::npos, test_log.find(inputlog));
            LoglogN_EX(logLevel, inputlog);
            EXPECT_EQ(1, test_count);
            EXPECT_NE(std::string::npos, test_log.find(exModule));
            LoglogN_EX(logLevel, inputlog);
            EXPECT_EQ(2, test_count);
            break;
        }
        case IF_EVERY_N_EX: {
            LogIflogN_EX(true, logLevel, inputlog);
            EXPECT_EQ(logLevel, test_level);
            EXPECT_NE(std::string::npos, test_log.find(inputlog));
            LogIflogN_EX(true, logLevel, inputlog);
            EXPECT_EQ(1, test_count);
            EXPECT_NE(std::string::npos, test_log.find(exModule));
            LogIflogN_EX(true, logLevel, inputlog);
            EXPECT_EQ(2, test_count);
            std::string notexistLog = inputlog+"notexist";
            LogIflogN_EX(false, logLevel, notexistLog);
            EXPECT_EQ(std::string::npos, test_log.find("notexist"));
            EXPECT_EQ(2, test_count);
            break;
        }
        case FIRST_N_EX: {
            LogFirstN_EX(logLevel, inputlog);
            EXPECT_EQ(logLevel, test_level);
            EXPECT_NE(std::string::npos, test_log.find(inputlog));
            EXPECT_NE(std::string::npos, test_log.find(exModule));
            LogFirstN_EX(logLevel, inputlog);
            EXPECT_EQ(2, test_count);
            LogFirstN_EX(logLevel, inputlog);
            EXPECT_EQ(2, test_count);
            break;
        }
    }
}

TEST_F(CmLogMacrosTest, IF_LogErrorlogN) {
    TestLogMacro(IF, CM_TRACE_LEVEL_ERROR);
}
TEST_F(CmLogMacrosTest, IF_LogWarnlogN) {
    TestLogMacro(IF, CM_TRACE_LEVEL_WARNING);
}
TEST_F(CmLogMacrosTest, IF_LogInfologN) {
    TestLogMacro(IF, CM_TRACE_LEVEL_INFO);
}
TEST_F(CmLogMacrosTest, IF_LogDebuglogN) {
    TestLogMacro(IF, CM_TRACE_LEVEL_DEBUG);
}
TEST_F(CmLogMacrosTest, IF_LogDetaillogN) {
    TestLogMacro(IF, CM_TRACE_LEVEL_DETAIL);
}

TEST_F(CmLogMacrosTest, EVERY_LogErrorlogN) {
    TestLogMacro(EVERY_N, CM_TRACE_LEVEL_ERROR);
}
TEST_F(CmLogMacrosTest, EVERY_LogWarnlogN) {
    TestLogMacro(EVERY_N, CM_TRACE_LEVEL_WARNING);
}
TEST_F(CmLogMacrosTest, EVERY_LogInfologN) {
    TestLogMacro(EVERY_N, CM_TRACE_LEVEL_INFO);
}
TEST_F(CmLogMacrosTest, EVERY_LogDebuglogN) {
    TestLogMacro(EVERY_N, CM_TRACE_LEVEL_DEBUG);
}
TEST_F(CmLogMacrosTest, EVERY_LogDetaillogN) {
    TestLogMacro(EVERY_N, CM_TRACE_LEVEL_DETAIL);
}

TEST_F(CmLogMacrosTest, IF_EVERY_LogErrorlogN) {
    TestLogMacro(IF_EVERY_N, CM_TRACE_LEVEL_ERROR);
}
TEST_F(CmLogMacrosTest, IF_EVERY_LogWarnlogN) {
    TestLogMacro(IF_EVERY_N, CM_TRACE_LEVEL_WARNING);
}
TEST_F(CmLogMacrosTest, IF_EVERY_LogInfologN) {
    TestLogMacro(IF_EVERY_N, CM_TRACE_LEVEL_INFO);
}
TEST_F(CmLogMacrosTest, IF_EVERY_LogDebuglogN) {
    TestLogMacro(IF_EVERY_N, CM_TRACE_LEVEL_DEBUG);
}
TEST_F(CmLogMacrosTest, IF_EVERY_LogDetaillogN) {
    TestLogMacro(IF_EVERY_N, CM_TRACE_LEVEL_DETAIL);
}

TEST_F(CmLogMacrosTest, FIRST_LogErrorlogN) {
    TestLogMacro(FIRST_N, CM_TRACE_LEVEL_ERROR);
}
TEST_F(CmLogMacrosTest, FIRST_LogWarnlogN) {
    TestLogMacro(FIRST_N, CM_TRACE_LEVEL_WARNING);
}
TEST_F(CmLogMacrosTest, FIRST_LogInfologN) {
    TestLogMacro(FIRST_N, CM_TRACE_LEVEL_INFO);
}
TEST_F(CmLogMacrosTest, FIRST_LogDebuglogN) {
    TestLogMacro(FIRST_N, CM_TRACE_LEVEL_DEBUG);
}
TEST_F(CmLogMacrosTest, FIRST_LogDetaillogN) {
    TestLogMacro(FIRST_N, CM_TRACE_LEVEL_DETAIL);
}