
#ifndef WME_RENDER_INTERFACE_H
#define WME_RENDER_INTERFACE_H

#include "WmeDefine.h"
#include "WmeUnknown.h"
#include "WmeError.h"
#include "WmeExternalRenderer.h"



namespace wme
{
    
///============================Media Render Define============================

/// Enum of render type
typedef enum
{
    WmeD3DRender    = 0,
    WmeGDIRender	= 1,
    WmeD2DRender	= 2
} WmeVideoRenderType;

/// A struct of render picture info
typedef struct _tagRenderPicInfo
{
    void* pBitMap;
    long  lMask;
    float fLeft;
    float fTop;
    float fRight;
    float fBottom;
}WmeVideoRenderPicInfo;

///============================Video Render Define============================
    /// IWmeVideoRender interface
    static const WMEIID WMEIID_IWmeVideoRender =
    { 0xa4646096, 0x7aa4, 0x4539, { 0xad, 0x48, 0xcb, 0x46, 0x37, 0xc0, 0x08, 0xc5 } };
    
    class WME_NOVTABLE IWmeVideoRender :public IWmeExternalRenderer,
                                        public IWmeUnknown
    {
    public:
        /// Add render picture
        /*!
         \param uiID : [in] Render pic ID
         \param pPicData : [in] Render pic info
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT AddRenderPicture(uint32_t uiID, WmeVideoRenderPicInfo* pPicData) = 0;
        /// Remove render picture
        /*!
         \param uiID : [in] Render pic ID
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT RemoveRenderPicture(uint32_t uiID) = 0;
        /// Get render picture info
        /*!
         \param uiID : [in] Render pic ID
         \param pPicData : [out] Render pic Info
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT GetRenderPictureDesc(uint32_t uiID, WmeVideoRenderPicInfo* pPicData) = 0;
        /// Set the background color
        /*!
         \param color : [in] background color
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT SetBKColor(void* color)=0;
        /// Clear the render with background color
        /*!
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT ClearStream()=0;
        /// Lock the render
        /*!
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT LockDraw()=0;
        /// UnLock the render
        /*!
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT UnlockDraw()=0;
        /// Redraw with current render
        /*!
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT ReDraw()=0;
        /// Set render scaling mode of render
        /*!
         \param mode : [in] render scaling mode
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT SetRenderScalingMode(WmeTrackRenderScalingModeType mode)=0;
        /// Get render scaling mode of render
        /*!
         \param pmode : [out] render scaling mode
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT GetRenderScalingMode(WmeTrackRenderScalingModeType& pmode)=0;
    protected:
        /// The destructor function
        virtual ~IWmeVideoRender(){}
    };
    
    /// IWmeVideoRendererWinProfile interface
    static const WMEIID WMEIID_IWmeVideoRendererWinProfile =
    { 0xe0cf62c6, 0xeb83, 0x451f, { 0xb4, 0xd4, 0xe0, 0xfa, 0xa0, 0x5a, 0xa7, 0x3c } };
    class WME_NOVTABLE IWmeVideoRendererWinProfile : public IWmeUnknown
    {
    public:
        /// Inform render the screen resolution change
        /*!
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT OnDisplayChange()=0;
        /// Inform render the render window size change
        /*!
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT OnWindowPositionChange()=0;
        /// Set render type
        /*!
         \param eType : [in] render type
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT SetRendererType(WmeVideoRenderType eType)=0;
        /// Get render type
        /*!
         \param eType : [out] render type
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT GetRendererType(WmeVideoRenderType &eType)=0;
        /// Set render only via redraw flag
        /*!
         \param bYes : [in] yes or no
         \return The error value of the function result
         \note not thread safe
         */
        virtual WMERESULT UpdateOnlyViaReDraw(bool bYes)=0;
    protected:
        virtual ~IWmeVideoRendererWinProfile(){}
    };
	// {EF62F84A-78DF-47d1-8053-CC947A4F1BEB}
	static const WMEIID WMEIID_IWmeVideoRendererWp8Profile =
	{ 0xef62f84a, 0x78df, 0x47d1, { 0x80, 0x53, 0xcc, 0x94, 0x7a, 0x4f, 0x1b, 0xeb } };

	class WME_NOVTABLE IWmeVideoRendererWp8Profile : public IWmeUnknown
	{
	public:
		virtual WMERESULT Initialize(void* device) = 0;
		virtual WMERESULT UpdateDevice(void* device, void* context, void* renderTargetView) = 0;
		virtual WMERESULT UpdateForWindowSizeChange(float width, float height) = 0;
		virtual void Render() = 0;

		virtual WMERESULT Initialize() = 0;
		virtual WMERESULT UpdateForRenderResolutionChange(float width, float height) = 0;
		virtual WMERESULT GetTexture(void** ppTexture2D) = 0;
	};

}	//namespace

#endif // WME_RENDER_INTERFACE_H
