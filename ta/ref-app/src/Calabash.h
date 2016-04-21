//
//  Calabash.h
//  MediaSessionTest
//
//  Created by Dan Lee Jo on 8/15/14.
//  Copyright (c) 2014 Dan Lee Jo. All rights reserved.
//

/**
*
*   This function is designed for Calabash backdoor functions to avoid populating all automated test call results through ClickCall.cpp
*
*/
#ifndef MediaSessionTest_Calabash_h
#define MediaSessionTest_Calabash_h

#include "MediaConnection.h"
#include <vector>
#include "json/json.h"

using namespace wme;

class Calabash {
public:
    Calabash();
    ~Calabash();
    
    struct OnMediaReadyArgs {
        unsigned long mid;
        WmeDirection direction;
        WmeSessionType mediaType;
        IWmeMediaTrackBase *pTrack;
    };
    
    void onMediaReadyStore (unsigned long mid, WmeDirection direction, WmeSessionType mediaType, IWmeMediaTrackBase *pTrack);
    void onCSIChanged(const char* mediaType, unsigned int vid, const unsigned int* newCSIArray, unsigned int newCSICount);
    std::vector<OnMediaReadyArgs> getOnMediaReadyInfoList();
    std::string GetCSIChangeHistory();
    
private:
    std::vector<OnMediaReadyArgs> onMediaReadyInfoList;
    Json::Value m_CSIChangedHistory;
};

#endif
