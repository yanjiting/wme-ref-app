//
//  CWseResourceForUT.h
//  sharkUnitTestApp
//
//  Created by rory on 15/4/15.
//  Copyright (c) 2015年 WME. All rights reserved.
//

#ifndef sharkUnitTestApp_CWseResourceForUT_h
#define sharkUnitTestApp_CWseResourceForUT_h
const static char * bitstreamForDecoderControlCase = "720p.264";
typedef struct{
    const void* pBitStreamForDecoderControl;
    unsigned int   uiBitSreamForDecoderControlLen;
}WseResourceForUTInfo;

typedef  struct {
    unsigned char pDeviceInfo[100];
    unsigned int  iInfoLen;
}WseDeviceInfo;

extern WseResourceForUTInfo sResourceInfo;
extern WseDeviceInfo        sDeviceInfo;


#endif
