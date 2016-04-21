#ifdef ENABLE_ZBAR
#include "../../../vendor/zbar/include/zbar.h"
#include "zbar.h"
#endif
#include <string>
#include "WMEMediaFileRenderSink.h"

#define STR(s) #s
using namespace std;
#ifdef ENABLE_ZBAR
using namespace zbar;
#endif

WMERESULT CWMEMediaFileRenderSink::OnOutputMediaData(uint32_t uTimestamp, WmeMediaFormatType eType, void* pFormat, unsigned char *pData, uint32_t uLength)
{
#ifdef ENABLE_ZBAR
    ImageScanner scanner;
    
    WmeVideoRawFormat* pVideoFormat = (WmeVideoRawFormat*)pFormat;
    
    int width = pVideoFormat->iWidth;
    int height = pVideoFormat->iHeight;
    CM_INFO_TRACE_THIS("CWMEMediaFileRenderSink::OnOutputMediaData,width = "<<width<<", height = "<< height);
    
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
    
    Image image(width, width, "GREY", (void*)pData,  width * width);
    
    int n = scanner.scan(image);
    
    Image::SymbolIterator symbol = image.symbol_begin();
    
    std::string str = symbol->get_data();
    
    image.set_data(NULL, 0);
    
    CM_INFO_TRACE_THIS("CWMEMediaFileRenderSink::OnOutputMediaData,str="<<str);
#endif
    
    return 0;
}