

#import "WMEDataProcess.h"

@implementation WMEDataProcess
@synthesize arrayShareSourcesId = _arrayShareSourcesId;
@synthesize arrayShareSourcesName = _arrayShareSourcesName;

-(void)getShareSources
{
    wme::IWmeMediaEngine* pEngine = NULL;
    wme::IWmeScreenSourceEnumerator* pEnumDesktop = NULL;
    wme::IWmeScreenSourceEnumerator* pEnumApp = NULL;
    NSMutableArray *arySourcesId = [NSMutableArray array];
    NSMutableArray *arySourcesName = [NSMutableArray array];
    //[arySources addObject:@"Main Screen"];
    
    wme::WmeInit(0);
    wme::WmeCreateMediaEngine(&pEngine);
    if(pEngine!=NULL){
    	pEngine->CreateScreenSourceEnumerator(&pEnumDesktop, wme::WmeScreenSourceTypeDesktop);
        pEngine->CreateScreenSourceEnumerator(&pEnumApp, wme::WmeScreenSourceTypeApplication);
    }
    
    //list desktop
    if(pEnumDesktop){
        int nCount = 0;
        pEnumDesktop->GetNumber(nCount);
        for(int i=0;i<nCount;i++){
            wme::IWmeScreenSource * pWmeScreenSource = NULL;
            pEnumDesktop->GetSource(i,&pWmeScreenSource);
            if(pWmeScreenSource){
                int nBufLen = 64;
                char szUniqueName[64]={0};
                pWmeScreenSource->GetUniqueName(szUniqueName, nBufLen);
                [arySourcesId addObject:[NSString stringWithUTF8String: szUniqueName]];
                nBufLen = 64;
                char szFriendlyName[64]={0};
                pWmeScreenSource->GetFriendlyName(szFriendlyName, nBufLen);
                //[arySourcesName addObject:[NSString stringWithUTF8String: szFriendlyName]];
                [arySourcesName addObject:[NSString stringWithFormat:@"Monitor %d", i+1]];
            }
        }
    }
    
    //list application
    if(pEnumApp){
        int nCount = 0;
        pEnumApp->GetNumber(nCount);
        for(int i=0;i<nCount;i++){
            wme::IWmeScreenSource * pWmeScreenSource = NULL;
            pEnumApp->GetSource(i,&pWmeScreenSource);
            if(pWmeScreenSource){
                int nBufLen = 64;
                char szUniqueName[64]={0};
                pWmeScreenSource->GetUniqueName(szUniqueName, nBufLen);
                [arySourcesId addObject:[NSString stringWithUTF8String: szUniqueName]];
                nBufLen = 64;
                char szFriendlyName[64]={0};
                pWmeScreenSource->GetFriendlyName(szFriendlyName, nBufLen);
                [arySourcesName addObject:[NSString stringWithUTF8String: szFriendlyName]];
            }
        }
    }
    
    [_arrayShareSourcesId release];
    _arrayShareSourcesId = [[NSArray alloc] initWithArray:arySourcesId];
    [_arrayShareSourcesName release];
    _arrayShareSourcesName = [[NSArray alloc] initWithArray:arySourcesName];
    if(pEngine){
        pEngine->Release();
        pEngine = NULL;
    }
    if(pEnumApp){
        pEnumApp->Release();
        pEnumApp = NULL;
    }
    if(pEnumDesktop){
        pEnumDesktop->Release();
        pEnumDesktop = NULL;
    }
    wme::WmeUninit();
}

-(void)setShareSourceIndex:(NSInteger)idx
{
    _shareSourceIndex = idx;
}

-(NSInteger)getShareSourceIndex
{
    return _shareSourceIndex;
}

@end

