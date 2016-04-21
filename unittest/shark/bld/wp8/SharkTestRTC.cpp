#include <windows.h>
#include <iostream>
#include "SharkTestRTC.h"

using namespace Platform;
using namespace Windows;
using namespace Windows::Storage;
using namespace SharkTestRTComponent;

typedef int(*pfTestAllCases)(const char* path);

SharkTestRTC::SharkTestRTC()
{
}

int SharkTestRTC::TestAllCases()
{
	int argc = 2;
	char* argv[6];
	int iRet = 0;

	HMODULE phTestCasesDllHandler = NULL;
	pfTestAllCases pUTHandler = NULL;
	LPCWSTR cTestCasesDllDLLName = L"SharkTest.dll";

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

	pUTHandler = (pfTestAllCases)GetProcAddress(phTestCasesDllHandler, "shark_unittest_main");

	if (NULL == pUTHandler)
	{
		std::cout << "failed to load function" << std::endl;
		return 2;
	}


	// test all cases
	iRet = pUTHandler(OutputPath);

	return iRet;

}
