#include "gtest/gtest.h"
#include "json.h"
#include <string>

using namespace testing;

TEST(SuperEasyJson, TestParser)
{
	const char szTest[] = "{\"key\" : \"value\" }  ";
	std::string sParam(szTest, sizeof(szTest));
	json::Value root = json::Deserialize(sParam);

	ASSERT_TRUE(root.GetType() == json::ObjectVal);

	sParam = std::string(szTest, strlen(szTest));
	root = json::Deserialize(sParam);

	ASSERT_TRUE(root.GetType() == json::ObjectVal);
}

TEST(SuperEasyJson, TestParserNonNullEnd)
{
	const char szTest[] = "{\"key\" : \"value\" }  \r\t\n";
	std::string sParam(szTest, sizeof(szTest));
	json::Value root = json::Deserialize(sParam);

	ASSERT_TRUE(root.GetType() == json::ObjectVal);
}
