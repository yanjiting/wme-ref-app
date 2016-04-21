#include "wrtpwmeapi.h"
#include "CmReferenceControl.h"
#include "CmMessageBlock.h"
#include "wrtpwmeapi.h"
#include "wrtpclientapi.h"

using namespace wrtp;
using namespace wme;

class  CWMEMediaPackage
    : public IWmeMediaPackage
    , private CCmReferenceControlMutilThread
{
public:
    CWMEMediaPackage(WmeMediaFormatType eFormatType, uint32_t uFormatSize, uint32_t uDataSize)
        : m_formatType(eFormatType)
        , m_formatSize(uFormatSize)
        , m_maxDataSize(uDataSize)
        , m_currentDataSize(0)
        , m_dataOffset(0)
        , m_timestamp(0)
        , m_payloadType(0)
        , m_buffer(NULL)
        , m_formatPointer(NULL)
        , m_dataPointer(NULL)
    {
        uint32_t bufferSize = m_formatSize + m_maxDataSize;
        if (bufferSize > 0) {
            m_buffer        = new uint8_t[bufferSize];
            m_formatPointer = m_buffer;
            m_dataPointer   = m_buffer + m_formatSize;
        }
    }

    ~CWMEMediaPackage()
    {
        if (m_buffer) {
            delete [] m_buffer;
        }
    }

    /// Add reference count
    /*!
        \return Reference count after add one
    */
    virtual uint32_t WMECALLTYPE AddRef()
    {
        return CCmReferenceControlMutilThread::AddReference();
    }

    /// Subtract reference count
    /*!
        \return Reference count after subtract one
    */
    virtual uint32_t WMECALLTYPE Release()
    {
        return CCmReferenceControlMutilThread::ReleaseReference();
    }

    virtual uint32_t WMECALLTYPE GetReference()
    {
        return CCmReferenceControlMutilThread::GetReference();
    }

    /// Query pointer of interface by REFWMEIID
    /*!
        \param idd : [in] REFWMEIID valude
        \param ppvObject: [out] Pointer of interface
        \return The error value of the funtion result
    */
    virtual WMERESULT WMECALLTYPE QueryInterface(REFWMEIID iid, void **ppvObject)
    {
        if (ppvObject) {
            *ppvObject = NULL;
        }

        return WME_E_FAIL;
    }


    /// Get timestamp of the package
    /*!
        \param uTimestamp : [out] Reference of timestamp
        \return The error value of the function result
    */
    virtual WMERESULT GetTimestamp(uint32_t &uTimestamp)
    {
        uTimestamp = m_timestamp;
        return WME_S_OK;
    }

    /// Set timestamp of the package
    /*!
        \param uTimestamp : [in] Timestamp value
        \return The error value of the function result
    */
    virtual WMERESULT SetTimestamp(uint32_t uTimestamp)
    {
        m_timestamp = uTimestamp;
        return WME_S_OK;
    }

    /// Get media format type
    /*!
        \param eType : [out] Reference of media format type
        \return The error value of the function result
    */
    virtual WMERESULT GetFormatType(WmeMediaFormatType &eType)
    {
        eType = m_formatType;
        return WME_S_OK;
    }

    /// Get size of format data
    /*!
        \param uSize : [out] Reference of size value
        \return The error value of the function result
    */
    virtual WMERESULT GetFormatSize(uint32_t &uSize)
    {
        uSize = m_formatSize;
        return WME_S_OK;
    }

    /// Get pointer of format data
    /*!
        \param ppBuffer : [out] Reference of format data handle
        \return The error value of the function result
    */
    virtual WMERESULT GetFormatPointer(unsigned char **ppBuffer)
    {
        if (ppBuffer) {
            *ppBuffer = m_formatPointer;
        }
        return WME_S_OK;
    }

    /// Get size of data buffer
    /*!
        \param uSize : [out] Reference of size value
        \return The error value of the function result
    */
    virtual WMERESULT GetDataSize(uint32_t &uSize)
    {
        uSize = m_maxDataSize;
        return WME_S_OK;
    }

    /// Get data buffer
    /*!
        \param ppBuffer : [out] Pointer to the buffer
        \return The error value of the function result
    */
    virtual WMERESULT GetDataPointer(unsigned char **ppBuffer)
    {
        if (ppBuffer) {
            *ppBuffer = m_dataPointer;
        }
        return WME_S_OK;
    }

    /// Get offset of data buffer
    /*!
        \param uOffset : [out] Reference of offset value
        \return The error value of the function result
    */
    virtual WMERESULT GetDataOffset(uint32_t &uOffset)
    {
        uOffset = m_dataOffset;
        return WME_S_OK;
    }

    /// Get length of data buffer
    /*!
        \param uLength : [out] Reference of length value
        \return The error value of the function result
    */
    virtual WMERESULT GetDataLength(uint32_t &uLength)
    {
        uLength = m_currentDataSize;
        return WME_S_OK;
    }

    /// Set offset of data buffer
    /*!
        \param uOffset : [in] new offset value
        \return The error value of the function result
    */
    virtual WMERESULT SetDataOffset(uint32_t uOffset)
    {
        if (uOffset > m_currentDataSize) {
            return WME_E_FAIL;
        }

        m_currentDataSize = uOffset;
        return WME_S_OK;
    }

    /// Set length of data buffer
    /*!
        \param uLength : [in] new length value
        \return The error value of the function result
    */
    virtual WMERESULT SetDataLength(uint32_t uLength)
    {
        if (uLength > m_maxDataSize) {
            return WME_E_FAIL;
        }

        m_currentDataSize = uLength;
        return WME_S_OK;
    }

private:
    WmeMediaFormatType  m_formatType;
    uint32_t            m_formatSize;
    uint32_t            m_maxDataSize;
    uint32_t            m_currentDataSize;
    uint32_t            m_dataOffset;
    uint32_t            m_timestamp;
    uint32_t                m_payloadType;

    uint8_t *m_buffer;
    uint8_t *m_formatPointer;
    uint8_t *m_dataPointer;
};


class CWmeMediaPackageAllocatorMock : public IWmeMediaPackageAllocator//, private CCmReferenceControlMutilThread
{
public:
    CWmeMediaPackageAllocatorMock()  {}
    ~CWmeMediaPackageAllocatorMock() {}

    virtual uint32_t WMECALLTYPE AddRef()
    {
        //return CCmReferenceControlMutilThread::AddReference();
        return 1;
    }

    virtual uint32_t WMECALLTYPE Release()
    {
        //return CCmReferenceControlMutilThread::ReleaseReference();
        return 1;
    }

    virtual uint32_t WMECALLTYPE GetReference()
    {
        return 1;
    }


    virtual WMERESULT WMECALLTYPE QueryInterface(REFWMEIID iid, void **ppvObject)
    {
        if (*ppvObject) {
            *ppvObject = NULL;
        }

        return WME_E_FAIL;
    }

    /// Allocate package with specified format size and data size
    /*!
        \param uFormatSize : [in] Size of format data
        \param uDataSize : [in] Size of buffer
        \param ppPackage : [out] Pointer to package handle
        \return The error value of the function result
    */
    virtual WMERESULT GetPackage(WmeMediaFormatType eFormatType, uint32_t uFormatSize, uint32_t uDataSize, IWmeMediaPackage **ppPackage)
    {
        if (*ppPackage) {
            return WME_E_INVALIDARG;
        }

        *ppPackage = new CWMEMediaPackage(eFormatType, uFormatSize, uDataSize);
        (*ppPackage)->AddRef();
        return WME_S_OK;
    }

    static CWmeMediaPackageAllocatorMock *GetInstance();

private:
    static CWmeMediaPackageAllocatorMock s_mediaPackageAllocator;
};

// Note: the output mb should be released by calling CCmMessageBlock::DestroyChained()
void ConvertMediaPackageToMessageBlock(IRTPMediaPackage *rtpPackage, CCmMessageBlock *&mb);

// Note: the output *rtpPackage should be released by calling IRTPMediaPackage::Release()
void ConvertMessageBlockToMediaPackage(CCmMessageBlock &mb, IRTPMediaPackage **rtpPackage, bool isRTPPacket);

