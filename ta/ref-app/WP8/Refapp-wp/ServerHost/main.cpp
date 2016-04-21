
#include "pch.h"
#include <strsafe.h>

using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace ABI::Windows::ApplicationModel::Core;
using namespace ABI::Windows::UI::Core;
typedef HRESULT(WINAPI *GetActivationFactoryProc)(__in HSTRING activatableClassId, __deref_out IInspectable** factory);

class GetCustomClass : public Microsoft::WRL::RuntimeClass<
	RuntimeClassFlags<RuntimeClassType::WinRtClassicComMix>,
	ABI::Windows::Foundation::IGetActivationFactory,
	CloakedIid<IAgileObject >>
{
public:

	GetCustomClass()
	{
		//
		// Load the native library housing the winrt types for the application of this server host
		// in memory to get access to the activation factory method for the winrt library
		//
		 m_hMod = LoadPackagedLibrary(L"WMEAudioComponent.dll", 0x0);
        //m_hMod = NULL;
	}

	~GetCustomClass()
	{
		if (m_hMod)
		{
			FreeLibrary(m_hMod);
		}
	}

	IFACEMETHODIMP GetActivationFactory(_In_ HSTRING activatableClassId, _COM_Outptr_ IInspectable **factory)
	{
		HRESULT hr = S_OK;

		//
		// Retrieve the DllGetActivationFactory method from the native dll containing the winRT 
		// object used by our app and return it as the out parameter for activation factory
		//

		if (m_hMod)
		{
			GetActivationFactoryProc pActivationFactoryProc = reinterpret_cast<GetActivationFactoryProc>(
				GetProcAddress(m_hMod, "DllGetActivationFactory"));

			if (pActivationFactoryProc)
			{
				hr = pActivationFactoryProc(activatableClassId, factory);
			}
			else
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		return hr;
	}


private:

	HMODULE m_hMod;
};

static int flag = 0;

void __cdecl main()
{
	HRESULT hr = Windows::Foundation::Initialize(RO_INIT_MULTITHREADED);
	if (SUCCEEDED(hr))
	{
		// Scoping for smart pointers
		{
			ComPtr<ICoreApplication> spApplicationFactory;
			hr = GetActivationFactory(Platform::StringReference(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication).GetHSTRING(), &spApplicationFactory);
			if (SUCCEEDED(hr))
			{
				ComPtr<ABI::Windows::Foundation::IGetActivationFactory> spGetActivationFactory = Make<GetCustomClass>();
				spApplicationFactory->RunWithActivationFactories(spGetActivationFactory.Get());
			}
		}

		Windows::Foundation::Uninitialize();
		// Wait for any outstanding MTA references to be released
	}
}
