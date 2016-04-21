#pragma once
#include "MediaConnection.h"
#include <wrl/module.h>
#include <Windows.Phone.Graphics.Interop.h>
#include <DrawingSurfaceNative.h>

namespace WMEVideoComponent
{
	public delegate void RequestAdditionalFrameHandler();
	public delegate void RecreateSynchronizedTextureHandler();
	//[Windows::Foundation::Metadata::WebHostHidden]
	public ref class RenderDemoWindowMode sealed : public Windows::Phone::Input::Interop::IDrawingSurfaceManipulationHandler
	{
	public:
		RenderDemoWindowMode();
		virtual ~RenderDemoWindowMode();

		Windows::Phone::Graphics::Interop::IDrawingSurfaceContentProvider^ CreateContentProvider();

		// IDrawingSurfaceManipulationHandler
		virtual void SetManipulationHost(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ manipulationHost);

		event RequestAdditionalFrameHandler^ RequestAdditionalFrame;
		event RecreateSynchronizedTextureHandler^ RecreateSynchronizedTexture;

		property Windows::Foundation::Size WindowBounds;
		property Windows::Foundation::Size NativeResolution;
		property Windows::Foundation::Size RenderResolution	{
			Windows::Foundation::Size get(){ return m_renderResolution; }
			void set(Windows::Foundation::Size renderResolution);
		}


		void setRenderNULL();
		// Landscape, LandscapeLeft, LandscapeRight, iFlag is 0,
		// Portrait, PortraitUp, PortraitDown, iFlag is 1.
		void UpdateSceneForRotate(int iFlag, int iWidth, int iHeight);

	protected:
		// Event Handlers
		void OnPointerPressed(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerReleased(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerMoved(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender, Windows::UI::Core::PointerEventArgs^ args);

	internal:
		HRESULT STDMETHODCALLTYPE Connect(_In_ IDrawingSurfaceRuntimeHostNative* host);
		void STDMETHODCALLTYPE Disconnect();
		HRESULT STDMETHODCALLTYPE PrepareResources(_In_ const LARGE_INTEGER* presentTargetTime, _Out_ BOOL* contentDirty);
		HRESULT STDMETHODCALLTYPE GetTexture(_In_ const DrawingSurfaceSizeF* size, _Inout_ IDrawingSurfaceSynchronizedTextureNative** synchronizedTexture, _Inout_ DrawingSurfaceRectF* textureSubRectangle);
		ID3D11Texture2D* GetTexture();


		void getRenderInstance(wme::IWmeVideoRender **ppRenderer);

		long InitilizeScene(int width, int height);
		void UnInitilizeScene();

		//long ApplyToRender(void *vpAddress, wme::IWmeVideoRender** ppDeliverer, int idObject, int idUnit, int idElement);

	private:
		wme::IWmeVideoRender *m_pRenderer;

		//WseVideoRenderScene*	m_pSence;

		//BasicTimer^ m_timer;
		Windows::Foundation::Size m_renderResolution;
		bool m_nPlayState;
		CRITICAL_SECTION m_CriticalSection;
		bool m_bInDrawing;

		LARGE_INTEGER m_nLastRenderTime;
		LONGLONG m_nLastRenderTime2;

	};



	class Direct3DContentProviderWindowMode : public Microsoft::WRL::RuntimeClass <
		Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
		ABI::Windows::Phone::Graphics::Interop::IDrawingSurfaceContentProvider,
		IDrawingSurfaceContentProviderNative >
	{
	public:
		Direct3DContentProviderWindowMode(RenderDemoWindowMode^ controller);


		void ReleaseD3DResources();

		// IDrawingSurfaceContentProviderNative
		HRESULT STDMETHODCALLTYPE Connect(_In_ IDrawingSurfaceRuntimeHostNative* host);
		void STDMETHODCALLTYPE Disconnect();

		HRESULT STDMETHODCALLTYPE PrepareResources(_In_ const LARGE_INTEGER* presentTargetTime, _Out_ BOOL* contentDirty);
		HRESULT STDMETHODCALLTYPE GetTexture(_In_ const DrawingSurfaceSizeF* size, _Out_ IDrawingSurfaceSynchronizedTextureNative** synchronizedTexture, _Out_ DrawingSurfaceRectF* textureSubRectangle);

	private:
		HRESULT InitializeTexture(_In_ const DrawingSurfaceSizeF* size);

		RenderDemoWindowMode^ m_controller;
		Microsoft::WRL::ComPtr<IDrawingSurfaceRuntimeHostNative> m_host;
		Microsoft::WRL::ComPtr<IDrawingSurfaceSynchronizedTextureNative> m_synchronizedTexture;

	};

}