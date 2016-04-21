//
//  Calabash.cpp
//  MediaSessionTest
//
//  Created by Dan Lee Jo on 8/15/14.
//  Copyright (c) 2014 Dan Lee Jo. All rights reserved.
//

#include "Calabash.h"

Calabash::Calabash() : m_CSIChangedHistory(Json::objectValue)
{
    m_CSIChangedHistory["audio"] = Json::Value(Json::objectValue);
    m_CSIChangedHistory["video"] = Json::Value(Json::objectValue);
    m_CSIChangedHistory["share"] = Json::Value(Json::objectValue);
}

Calabash::~Calabash() {

}

void Calabash::onMediaReadyStore (unsigned long mid, WmeDirection direction, WmeSessionType mediaType, IWmeMediaTrackBase *pTrack) {
    OnMediaReadyArgs myArgs;
    myArgs.mid = mid;
    myArgs.direction = direction;
    myArgs.mediaType = mediaType;
    myArgs.pTrack = pTrack;

    onMediaReadyInfoList.push_back(myArgs);

}

std::string Calabash::GetCSIChangeHistory()
{
    Json::FastWriter w;
    return w.write(m_CSIChangedHistory);
}

void Calabash::onCSIChanged(const char* mediaType, unsigned int vid, const unsigned int* newCSIArray, unsigned int newCSICount)
{
    std::string sVid = std::to_string(vid);
    Json::Value & csi_media = m_CSIChangedHistory[mediaType];
    if (!csi_media.isMember(sVid))
        csi_media[sVid] = Json::Value(Json::arrayValue);
    Json::Value & csi_array = csi_media[sVid];
    Json::Value newCSIs(Json::arrayValue);
    int i = 0;
    for (i = 0; i < newCSICount; i++) {
        newCSIs.append(Json::Value(newCSIArray[i]));
    }
    csi_array.append(newCSIs);
}

std::vector<Calabash::OnMediaReadyArgs> Calabash::getOnMediaReadyInfoList() {
    return onMediaReadyInfoList;
}
