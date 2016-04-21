
#include "gtest/gtest.h"
#include "CmDef.h"
#include "CmStdCpp.h"
#include "CmUtilTemplates.h"

TEST (CCmString,CCmString) {
	const char *szBuff = "";
	CCmString *pCMStr = new CCmString();
	EXPECT_STREQ (szBuff, pCMStr->c_str());
	delete(pCMStr);
	pCMStr = NULL;

}

TEST (CCmString,CCmString_pchar) {
    const char *szBuff = "hello,world";
	CCmString *pCMStr = new CCmString(szBuff);
	EXPECT_STREQ (szBuff, pCMStr->c_str());
	delete(pCMStr);
	pCMStr = NULL;
    
}

TEST (CCmString,CCmString_pchar_size) {
	const char *szBuff = "hello,world";
	char szBuff2[32];
	memset(szBuff2,0,32);
	CCmString *pCMStr = new CCmString(szBuff,1024);
	ASSERT_EQ (1024,pCMStr->length());
	EXPECT_STREQ (szBuff, pCMStr->c_str());
	delete(pCMStr);
        pCMStr = NULL;

	pCMStr = new CCmString(szBuff,5);
	ASSERT_EQ (5,pCMStr->length() );
	strncpy(szBuff2,szBuff, 5);
	EXPECT_STREQ (szBuff2, pCMStr->c_str());
	delete(pCMStr);

	pCMStr = NULL;

}



TEST (CCmString,CCmString_Template) {
#if defined(CM_WIN32) || defined(CM_MACOS) || defined(CM_SOLARIS)
	std::string source;
	source.resize(128, '1');
	std::string stdStr(source.begin(), source.end());
	CCmString cCmstr(source.begin(), source.end());
	EXPECT_STREQ (stdStr.c_str(), cCmstr.c_str());

#else
	char *szBuffer = "hello,World";
	CCmString stdStr(szBuffer, szBuffer + strlen(szBuffer));
	std::string cCmstr(szBuffer, szBuffer + strlen(szBuffer));
	EXPECT_STREQ (stdStr.c_str(), cCmstr.c_str());

#endif

}


TEST (CCmString,CCmString_string) {
	const char *szBuff = "hello,world";
	std::string cstr = szBuff;

	CCmString *pCMStr2 = new CCmString(cstr);
	EXPECT_STREQ (szBuff, pCMStr2->c_str());

	delete(pCMStr2);
	pCMStr2 = NULL;

}

TEST (CCmString,CCmString_CCmString) {
	const char *szBuff = "hello,world";
	CCmString cMStr;
	cMStr = szBuff;

	CCmString *pCMStr2 = new CCmString(cMStr);
	EXPECT_STREQ (szBuff, pCMStr2->c_str());

	delete(pCMStr2);
	pCMStr2 = NULL;

}


TEST (CCmString,CCmString_n_c) {
	std::string *pStr = new std::string(12,'h');
	CCmString *pCMStr2 = new CCmString(12,'h');
	
	EXPECT_STREQ (pStr->c_str(), pCMStr2->c_str());

	delete(pStr);
	pStr = NULL;
	delete(pCMStr2);
	pCMStr2 = NULL;

}

TEST (CCmString,operatorAssign_pchar) {
	const char *szBuff = "hello,world";
	CCmString cMStr;
	cMStr = szBuff;
	EXPECT_STREQ (szBuff, cMStr.c_str());

}

TEST (CCmString,operatorAssign_CCmString) {
	const char *szBuff = "hello,world";
	CCmString cMStr = szBuff;
	CCmString cMStr2;
	cMStr2 = cMStr;
	EXPECT_STREQ (szBuff, cMStr2.c_str());
}

TEST (CCmString,operatorAssign_c) {
		char c = 'h';
		const char *szBuff = "h";
		CCmString cMStr;
		cMStr = c;
		EXPECT_STREQ (szBuff, cMStr.c_str());
}

TEST (CCmString,operatorAssign_i) {
	int i = 108;
	char szBuff[16] = {108};
	CCmString cMStr;
	cMStr = i;
	EXPECT_STREQ (szBuff, cMStr.c_str());
}

TEST (CCmString,toUpperCase) {
	const char *szBuff1 = "hello,world";
	const char *szBuff2 = "HELLO,WORLD";
	CCmString cMStr = szBuff1;
	EXPECT_STREQ (szBuff2, cMStr.toUpperCase().c_str());
}

TEST (CCmString,toLowerCase) {
	const char *szBuff1 = "hello,world";
	const char *szBuff2 = "HELLO,WORLD";
	CCmString cMStr = szBuff2;
	EXPECT_STREQ (szBuff1, cMStr.toLowerCase().c_str());
}

TEST (global,TrimString_CCmIsSpace) {
	const char *szBuff1 = "   hello,world   ";
	const char *szBuff2 = "hello,world";
	CCmString cMStr = szBuff1;
	TrimString(cMStr, CCmIsSpace());
	EXPECT_STREQ (szBuff2, cMStr.c_str());
}

	class CCmIsStar
	{
	public:
		int operator() (const char c) {
			return c == '*';
		}
	};

TEST (global,TrimString_template) {
	const char *szBuff1 = "****hello,world***";
	const char *szBuff2 = "hello,world";
	CCmString cMStr = szBuff1;
	TrimString(cMStr, CCmIsStar());
	EXPECT_STREQ (szBuff2, cMStr.c_str());
}

TEST(global, wmetof) {
    char str[256] = {0};
    EXPECT_STREQ(wme_ftoa(0.25, str), "0.25");
    EXPECT_STREQ(wme_ftoa(0.256, str), "0.25");
    EXPECT_STREQ(wme_ftoa(-0.256, str), "-0.25");
    EXPECT_STREQ(wme_ftoa(-10.256, str), "-10.25");
    EXPECT_STREQ(wme_ftoa(sqrt(-1.0), str), "NaN");
    EXPECT_STREQ(wme_ftoa(sqrtf(-1.0), str), "NaN");
}

