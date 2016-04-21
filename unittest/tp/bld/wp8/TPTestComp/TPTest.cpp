#include <windows.h>
#include <iostream>
#include <string>
#include "TPTest.h"

using namespace TPTestComp;
using namespace Platform;
using namespace Windows;
using namespace Windows::Storage;

TPTest::TPTest()
{
}

typedef int(*pfTestAllCases)(const char* path);

int TPTest::TestAllCases()
{
    int   argc = 2;
    int   iRet = 0;
    char *argv[6];

    HMODULE         phTestCasesDllHandler = NULL;
    pfTestAllCases  pUTHandler = NULL;
    LPCWSTR         cTestCasesDllDLLName = L"TpTestDll.dll";

    // output xml file location
    char OutputPath[128] = { 0 };
    Windows::Storage::StorageFolder^ OutputLocation;
    Platform::String^ OutputLocationPath;

    OutputLocation = ApplicationData::Current->LocalFolder;
    OutputLocationPath = Platform::String::Concat(OutputLocation->Path, "\\Shared\\");
    const wchar_t *pWcharOutputFile = OutputLocationPath->Data();

    int size = wcslen(pWcharOutputFile);
    OutputPath[size] = 0;
    for (int y = 0; y<size; y++)
    {
        OutputPath[y] = (char)pWcharOutputFile[y];
    }

    // load dynamic library
    phTestCasesDllHandler = LoadPackagedLibrary(cTestCasesDllDLLName, 0);
    DWORD dw = GetLastError();
    if (NULL == phTestCasesDllHandler)
    {
        std::cout << "failed to load dll,error code is : " << dw << std::endl;
        return 1;
    }

    pUTHandler = (pfTestAllCases)GetProcAddress(phTestCasesDllHandler, "tp_unittest_main");

    if (NULL == pUTHandler)
    {
        std::cout << "failed to load function" << std::endl;
        return 2;
    }

    // test all cases
    iRet = pUTHandler(OutputPath);

    return iRet;
}
