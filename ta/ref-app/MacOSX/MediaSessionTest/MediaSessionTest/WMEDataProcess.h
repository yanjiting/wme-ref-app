
#include "WmeEngine.h"
#include "WmeInterface.h"
#include "WmeScreenSource.h"
#include <string>

@interface WMEDataProcess : NSObject
{
    //std::vector<std::string> _appSource;
    NSArray *_arrayShareSourcesId;
    NSArray *_arrayShareSourcesName;
    NSInteger _shareSourceIndex;
    //wme::IWmeScreenSourceEnumerator * m_pIWmeScreenSourceEnumeratorDesktop;
    //wme::IWmeScreenSourceEnumerator * m_pIWmeScreenSourceEnumeratorApp;

}
@property (retain) NSArray *arrayShareSourcesId;
@property (retain) NSArray *arrayShareSourcesName;

- (void)getShareSources;
- (void)setShareSourceIndex:(NSInteger)idx;
- (NSInteger)getShareSourceIndex;

@end
