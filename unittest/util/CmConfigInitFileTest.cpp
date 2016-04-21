//
//  CCmConfigInitFileTest.cpp
//  utilTest_mac
//
//  Created by C3 Team on 8/5/14.
//  Copyright (c) 2014 cisco. All rights reserved.
//
#include "gtest/gtest.h"
#include "CmConfigInitFile.h"

class CCmConfigInitFileTest : public testing::Test
{
public:
	CCmConfigInitFileTest()
	{

	}
	
	virtual ~CCmConfigInitFileTest()
	{

	}
    
    
	virtual void SetUp()
	{

	}
    
	virtual void TearDown()
	{

	}
    
    
protected:
	
	CCmConfigInitFile m_cif;

    static unsigned char get_string_param_from_configfile_mock_int(
                                                   const char* config_file_name,
                                                   const char* group,
                                                   const char* item_key,
                                                   char* item_value,
                                                   unsigned long len)
    {
        strncpy(item_value,"8008",16);
        return 1;
    }
    
    static unsigned char get_string_param_from_configfile_mock_float(
                                                                   const char* config_file_name,
                                                                   const char* group,
                                                                   const char* item_key,
                                                                   char* item_value,
                                                                   unsigned long len)
    {
        strncpy(item_value,"88.880",16);
        return 1;
    }
    
    static unsigned char get_string_param_from_configfile_mock_Dword(
                                                                   const char* config_file_name,
                                                                   const char* group,
                                                                   const char* item_key,
                                                                   char* item_value,
                                                                   unsigned long len)
    {
        strncpy(item_value,"8008",16);
        return 1;
    }
    
    static unsigned char get_string_param_from_configfile_mock_word(
                                                                     const char* config_file_name,
                                                                     const char* group,
                                                                     const char* item_key,
                                                                     char* item_value,
                                                                     unsigned long len)
    {
        strncpy(item_value,"8008",16);
        return 1;
    }
    
    static unsigned char get_string_param_from_configfile_mock_String(
                                                                     const char* config_file_name,
                                                                     const char* group,
                                                                     const char* item_key,
                                                                     char* item_value,
                                                                     unsigned long len)
    {
        strncpy(item_value,"HELLOTEST",16);
        return 1;
    }
    
    static unsigned char get_string_param_from_configfile_mock_BOOL(
                                                                     const char* config_file_name,
                                                                     const char* group,
                                                                     const char* item_key,
                                                                     char* item_value,
                                                                     unsigned long len)
    {
        strncpy(item_value,"TRUE",16);
        return 1;
    }
    
};


TEST_F(CCmConfigInitFileTest, TestGetIntParam)
{
    m_cif.RegisterFunc((GET_PARAM_FUNC)get_string_param_from_configfile_mock_int);
    EXPECT_TRUE(m_cif.GetIntParam("TEST", "TEST") == 8008);
}

TEST_F(CCmConfigInitFileTest, TestGetDwordParam)
{
    m_cif.RegisterFunc((GET_PARAM_FUNC)get_string_param_from_configfile_mock_Dword);
    EXPECT_TRUE(m_cif.GetDwordParam("TEST", "TEST") == 8008);
}

TEST_F(CCmConfigInitFileTest, TestGetWordParam)
{
    m_cif.RegisterFunc((GET_PARAM_FUNC)get_string_param_from_configfile_mock_word);
    EXPECT_TRUE(m_cif.GetWordParam("TEST", "TEST") == 8008);}

TEST_F(CCmConfigInitFileTest, TestGetStringParam)
{
    m_cif.RegisterFunc((GET_PARAM_FUNC)get_string_param_from_configfile_mock_String);
    std::string ss = m_cif.GetStringParam("TEST", "TEST","TEST");
    EXPECT_EQ(ss, "HELLOTEST");
}

TEST_F(CCmConfigInitFileTest, TestGetBoolParam)
{
    m_cif.RegisterFunc((GET_PARAM_FUNC)get_string_param_from_configfile_mock_BOOL);
    EXPECT_TRUE(m_cif.GetBoolParam("TEST", "TEST") == TRUE);
}
TEST_F(CCmConfigInitFileTest, TestGetFloatParam)
{
    m_cif.RegisterFunc((GET_PARAM_FUNC)get_string_param_from_configfile_mock_float);
    EXPECT_FLOAT_EQ(m_cif.GetFloatParam("TEST", "TEST"), 88.88);
}


