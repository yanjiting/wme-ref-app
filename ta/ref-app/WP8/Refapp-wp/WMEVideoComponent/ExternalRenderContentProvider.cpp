//#include "pch.h"
#include "ExternalRenderContentProvider.h"
namespace WMEVideoComponent
{
	RenderDemoWindowMode::RenderDemoWindowMode() :
		//m_timer(ref new BasicTimer()),
		m_nPlayState(0)
	{
		m_pRenderer = NULL;
		//m_pSence = NULL;
		m_nLastRenderTime.HighPart = 0;
		m_nLastRenderTime.LowPart = 0;
		m_nLastRenderTime2 = 0;
		InitializeCriticalSectionEx(&m_CriticalSection, 0, 0);
	}

	RenderDemoWindowMode::~RenderDemoWindowMode()
	{
		if (NULL != m_pRenderer) {
			m_pRenderer->Release();
			m_pRenderer = nullptr;
		}
		DeleteCriticalSection(&m_CriticalSection);
	}

	void RenderDemoWindowMode::setRenderNULL()
	{
		m_pRenderer = NULL;
	}

	void RenderDemoWindowMode::getRenderInstance(wme::IWmeVideoRender **ppRenderer)
	{
		if (NULL != ppRenderer)
		{
			*ppRenderer = m_pRenderer;
		}
	}

	Windows::Phone::Graphics::Interop::IDrawingSurfaceContentProvider^ RenderDemoWindowMode::CreateContentProvider()
	{
		Microsoft::WRL::ComPtr<Direct3DContentProviderWindowMode> provider = Microsoft::WRL::Make<Direct3DContentProviderWindowMode>(this);
		return reinterpret_cast<Windows::Phone::Graphics::Interop::IDrawingSurfaceContentProvider^>(provider.Get());
	}

	// IDrawingSurfaceManipulationHandler
	void RenderDemoWindowMode::SetManipulationHost(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ manipulationHost)
	{
		manipulationHost->PointerPressed +=
			ref new Windows::Foundation::TypedEventHandler < Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^,
			Windows::UI::Core::PointerEventArgs^ > (this, &RenderDemoWindowMode::OnPointerPressed);

		manipulationHost->PointerMoved +=
			ref new Windows::Foundation::TypedEventHandler < Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^,
			Windows::UI::Core::PointerEventArgs^ > (this, &RenderDemoWindowMode::OnPointerMoved);

		manipulationHost->PointerReleased +=
			ref new Windows::Foundation::TypedEventHandler < Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^,
			Windows::UI::Core::PointerEventArgs^ > (this, &RenderDemoWindowMode::OnPointerReleased);
	}

	// Event Handlers
	void RenderDemoWindowMode::OnPointerPressed(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender,
		Windows::UI::Core::PointerEventArgs^ args)
	{
		// Insert your code here.
	}

	void RenderDemoWindowMode::OnPointerMoved(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender,
		Windows::UI::Core::PointerEventArgs^ args)
	{
		// Insert your code here.
	}

	void RenderDemoWindowMode::OnPointerReleased(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender,
		Windows::UI::Core::PointerEventArgs^ args)
	{
		// Insert your code here.
	}

	void RenderDemoWindowMode::RenderResolution::set(Windows::Foundation::Size renderResolution)
	{

		OutputDebugStringA("enter  RenderDemoWindowMode::RenderResolution::set\n");
		EnterCriticalSection(&m_CriticalSection);
		if (renderResolution.Width != m_renderResolution.Width ||
			renderResolution.Height != m_renderResolution.Height)
		{
			m_renderResolution = renderResolution;

			if (m_pRenderer)
			{
				wme::IWmeVideoRendererWp8Profile* pWp8Render = NULL;
				m_pRenderer->QueryInterface(wme::WMEIID_IWmeVideoRendererWp8Profile, (void**)(&pWp8Render));
				if (pWp8Render)
				{
					pWp8Render->UpdateForRenderResolutionChange(m_renderResolution.Width, m_renderResolution.Height);
					pWp8Render->Release();
				}
				//m_pRenderer->UpdateForRenderResolutionChange(m_renderResolution.Width, m_renderResolution.Height);

			}

			RecreateSynchronizedTexture();

		}

		LeaveCriticalSection(&m_CriticalSection);
	}

	int calculate16_9(int width, int height)
	{
		return width * 9 / 16;
	}

	// Interface With Direct3DContentProvider
	HRESULT RenderDemoWindowMode::Connect(_In_ IDrawingSurfaceRuntimeHostNative* host)
	{
        if (NULL == m_pRenderer)
        {
            int nRet = wme::WmeCreateVideoRender(NULL, &m_pRenderer);
			if (NULL != m_pRenderer)
			{
				wme::IWmeVideoRendererWp8Profile* pWp8Render = NULL;
				m_pRenderer->QueryInterface(wme::WMEIID_IWmeVideoRendererWp8Profile, (void**)(&pWp8Render));
				if (pWp8Render)
				{
					pWp8Render->UpdateForWindowSizeChange(WindowBounds.Width, WindowBounds.Height);
					pWp8Render->Initialize();
					pWp8Render->UpdateForRenderResolutionChange(m_renderResolution.Width, m_renderResolution.Height);
					pWp8Render->Release();
				}

				RecreateSynchronizedTexture();
				return S_OK;
			}
        }
	}

	void RenderDemoWindowMode::Disconnect()
	{
		OutputDebugStringA("enter RenderDemoWindowMode::Disconnect\n");

    }

    HRESULT RenderDemoWindowMode::PrepareResources(_In_ const LARGE_INTEGER* presentTargetTime, _Out_ BOOL* contentDirty)
    {
        //OutputDebugStringA("enter RenderDemoWindowMode::PrepareResources\n");

        //if (NULL != m_pRenderer)

        EnterCriticalSection(&m_CriticalSection);

        //LARGE_INTEGER diff = (*presentTargetTime) - m_nLastRenderTime;

        LONGLONG nDiff = 0;
        nDiff = presentTargetTime->QuadPart - m_nLastRenderTime.QuadPart;

        if (nDiff > 7 * 10000)
        {
            *contentDirty = true;
            m_nLastRenderTime.QuadPart = presentTargetTime->QuadPart;
            //      char szBuffer[256];
            //      sprintf(szBuffer, "RenderDemoWindowMode::PrepareResources, dwDiff1 = %I64d, dwDiff2 = %I64d\n", nDiff1, nDiff2 );
            //      OutputDebugStringA(szBuffer);
        }
        else
        {
            *contentDirty = false;
            //RequestAdditionalFrame();
        }

    {
        //      *contentDirty = true;

        LeaveCriticalSection(&m_CriticalSection);
        return S_OK;
    }
    //else
    {
        //  *contentDirty = false;
        //  return S_FALSE;
    }

    }


    HRESULT RenderDemoWindowMode::GetTexture(_In_ const DrawingSurfaceSizeF* size, _Out_ IDrawingSurfaceSynchronizedTextureNative** synchronizedTexture, _Out_ DrawingSurfaceRectF* textureSubRectangle)
    {
        //OutputDebugStringA("enter RenderDemoWindowMode::GetTexture -1\n");

        //if(m_nPlayState == 1)
        //  return S_OK;

        if (size->width - m_renderResolution.Width + 1 < 0 || size->height - m_renderResolution.Height + 1 < 0)
        {
            OutputDebugStringA("enter RenderDemoWindowMode::GetTexture 1, size->width != m_renderResolution.Width || size->height != m_renderResolution.Height\n");
            return S_OK;
        }

        if (NULL != m_pRenderer)
        {
            EnterCriticalSection(&m_CriticalSection);

            wme::IWmeVideoRendererWp8Profile* pWp8Render = NULL;
            m_pRenderer->QueryInterface(wme::WMEIID_IWmeVideoRendererWp8Profile, (void**)(&pWp8Render));
            if (pWp8Render)
            {
                pWp8Render->Render();
                pWp8Render->Release();
            }


            LeaveCriticalSection(&m_CriticalSection);

            RequestAdditionalFrame();
            //      OutputDebugStringA("enter RenderDemoWindowMode::GetTexture, RequestAdditionalFrame\n");
            return S_OK;
        }
        else
        {
            return S_OK;
        }


    }

    ID3D11Texture2D* RenderDemoWindowMode::GetTexture()
    {
        ID3D11Texture2D *pTexure = NULL;

        //OutputDebugStringA("enter RenderDemoWindowMode::GetTexture 2 \n");

        EnterCriticalSection(&m_CriticalSection);

        if (NULL != m_pRenderer)
        {
            wme::IWmeVideoRendererWp8Profile* pWp8Render = NULL;
            m_pRenderer->QueryInterface(wme::WMEIID_IWmeVideoRendererWp8Profile, (void**)(&pWp8Render));
            if (pWp8Render)
            {
                pWp8Render->GetTexture((void **)&pTexure);
                pWp8Render->Release();
            }

        }

        LeaveCriticalSection(&m_CriticalSection);

        return pTexure;
    }

    void RenderDemoWindowMode::UnInitilizeScene()
    {
    }



    int calculate_height_16v9(int width)
    {
        return width * 9 / 16;
    }


    int calculate_width_16v9(int height)
    {
        return height * 16 / 9;
    }


    long RenderDemoWindowMode::InitilizeScene(int width, int height)
    {
        //long ret = WSE_S_OK;
        return 0;
    }

    void RenderDemoWindowMode::UpdateSceneForRotate(int iFlag, int iWidth, int iHeight)
    {

    }


    //long RenderDemoWindowMode::ApplyToRender(void *vpAddress,
    //  IWseVideoDeliverer** ppDeliverer,
    //  int idObject, int idUnit, int idElement)
    //{
    //  long ret = WSE_S_FALSE;

    //  if (m_pRenderer)
    //  {
    //      *ppDeliverer = (IWseVideoDeliverer *)m_pRenderer;
    //  }

    //  return WSE_S_OK;
    //}


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Direct3DContentProviderWindowMode::Direct3DContentProviderWindowMode(RenderDemoWindowMode^ controller) :
        m_controller(controller)
    {
        m_controller->RequestAdditionalFrame += ref new RequestAdditionalFrameHandler([=]()
        {
            if (m_host)
            {
                m_host->RequestAdditionalFrame();
            }
        });

        m_controller->RecreateSynchronizedTexture += ref new RecreateSynchronizedTextureHandler([=]()
        {
            if (m_host)
            {
                m_host->CreateSynchronizedTexture(m_controller->GetTexture(), &m_synchronizedTexture);
            }
        });
    }

    // IDrawingSurfaceContentProviderNative interface
    HRESULT Direct3DContentProviderWindowMode::Connect(_In_ IDrawingSurfaceRuntimeHostNative* host)
    {
        m_host = host;

        return m_controller->Connect(host);
    }

    void Direct3DContentProviderWindowMode::Disconnect()
    {
        m_controller->Disconnect();
        m_host = nullptr;
        m_synchronizedTexture = nullptr;
    }

    HRESULT Direct3DContentProviderWindowMode::PrepareResources(_In_ const LARGE_INTEGER* presentTargetTime, _Out_ BOOL* contentDirty)
    {
        return m_controller->PrepareResources(presentTargetTime, contentDirty);
    }


    HRESULT Direct3DContentProviderWindowMode::GetTexture(_In_ const DrawingSurfaceSizeF* size, _Out_ IDrawingSurfaceSynchronizedTextureNative** synchronizedTexture, _Out_ DrawingSurfaceRectF* textureSubRectangle)
    {
        HRESULT hr = S_OK;

        if (!m_synchronizedTexture)
        {
            hr = m_host->CreateSynchronizedTexture(m_controller->GetTexture(), &m_synchronizedTexture);
        }

        // Set output parameters.
        textureSubRectangle->left = 0.0f;
        textureSubRectangle->top = 0.0f;
        textureSubRectangle->right = static_cast<FLOAT>(size->width);
        textureSubRectangle->bottom = static_cast<FLOAT>(size->height);

        m_synchronizedTexture.CopyTo(synchronizedTexture);

        // Draw to the texture.
        if (SUCCEEDED(hr))
        {
            hr = m_synchronizedTexture->BeginDraw();

            if (SUCCEEDED(hr))
            {
                hr = m_controller->GetTexture(size, synchronizedTexture, textureSubRectangle);
            }

            m_synchronizedTexture->EndDraw();
        }

        return S_OK;
    }
}