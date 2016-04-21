///
///  WmeUuid.h
///
///
///  Created by Sand Pei on 2013-1-14.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_UUID_DEFINE_H
#define WME_UUID_DEFINE_H

#include "WmeCommonTypes.h"

namespace wme
{

/// WMEUUID
typedef struct _tagUUID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[ 8 ];
} WMEUUID;

#define REFWMEUUID		const WMEUUID &
#define IsEqualWMEUUID(ruuid1, ruuid2) (!memcmp(&ruuid1, &ruuid2, sizeof(WMEUUID)))

}

#endif // WME_UUID_DEFINE_H
