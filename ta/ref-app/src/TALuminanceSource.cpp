//
//  TALuminanceSource.cpp
//  MediaSessionTest

#include "TALuminanceSource.h"
#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/common/GreyscaleRotatedLuminanceSource.h>
#include <zxing/common/HybridBinarizer.h>

int DecodeQRCodeInFrame(std::vector<std::string> &result, void *pRawData, int w, int h, int dataType, int rows, int cols, int bytesPerPix)
{
    zxing::qrcode::QRCodeReader reader;
    zxing::DecodeHints hints;
    zxing::TALuminanceSource taSource(pRawData, w, h, dataType, rows, cols, bytesPerPix);
    zxing::ArrayRef<char> data = taSource.getMatrix();
    int qrW = w / cols, qrH = h / rows;
    int ret(0);
    for( int i = 0; i< rows; i ++ )
    {
        int top = i * h / rows;
        for (int j=0; j<cols; j++ )
        {
            int left = j * w / cols;
            zxing::GreyscaleRotatedLuminanceSource source(data, w, h, left, top, qrW, qrH);
            zxing::Ref<zxing::LuminanceSource> rsource(&source);
            zxing::HybridBinarizer binarizer(rsource);
            zxing::Ref<zxing::Binarizer> rbinarizer(&binarizer);
            zxing::BinaryBitmap bitmap(rbinarizer);
            zxing::Ref<zxing::BinaryBitmap> rbitmap(&bitmap);
            zxing::Ref<zxing::Result> r = reader.decode(rbitmap, hints);
            if( r->getBarcodeFormat() == zxing::BarcodeFormat::QR_CODE )
            {
                result.push_back(r->getText()->getText());
                ret ++;
            }
            else {
                result.push_back("error");
            }
        }
    }
    return ret;
}

char GrayScalePixel(const char *pixData, int type);
namespace zxing {
    TALuminanceSource::TALuminanceSource(void *pRawData, int w, int h, int dataType, int rows, int cols, int bytesPerPix)
    : LuminanceSource(w, h)
    , m_pRawData((char *)pRawData)
    , m_nRows(rows)
    , m_nCols(cols)
    , m_nBytesPerPix(bytesPerPix)
    , m_nDataType(dataType)
    {
        
    }
    
    TALuminanceSource::~TALuminanceSource()
    {
        
    }
    Ref<LuminanceSource> TALuminanceSource::crop(int left, int top, int width, int height) const
    {
        return Ref<LuminanceSource>(new GreyscaleRotatedLuminanceSource(getMatrix(), getWidth(), getHeight(), left, top, width, height));
    }
    
    ArrayRef<char> TALuminanceSource::getRow(int y, ArrayRef<char> row) const
    {
        if( m_nDataType == 1 && m_nBytesPerPix == 1 )
        {
            const char* pixelRow = m_pRawData + y * getWidth() * m_nBytesPerPix;
            if (!row) {
                row = zxing::ArrayRef<char>(getWidth());
            }
            memcpy(&row[0], pixelRow, row.count());
        }
        else {
            const char* pixelRow = m_pRawData + y * getWidth() * m_nBytesPerPix;
            if (!row) {
                row = zxing::ArrayRef<char>(getWidth());
            }
            for (int x = 0; x < getWidth(); x++) {
                row[x] = GrayScalePixel(pixelRow + (x * m_nBytesPerPix), m_nDataType);
            }
        }
        return row;
        
    }
    ArrayRef<char> TALuminanceSource::getMatrix() const
    {
        const char* p = m_pRawData;
        zxing::ArrayRef<char> matrix(getWidth() * getHeight());
        if( m_nDataType == 1 && m_nBytesPerPix == 1 )
        {
            memcpy(&matrix[0], p, matrix.count());
        }
        else
        {
            char* m = &matrix[0];
            for (int y = 0; y < getHeight(); y++) {
                for (int x = 0; x < getWidth(); x++) {
                    *m = GrayScalePixel(p, m_nDataType);
                    m++;
                    p += m_nBytesPerPix;
                }
            }
        }
        return matrix;
    }
}