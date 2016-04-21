/*------------------------------------------------------*/
/* Copyright (C) WebEx Communications Inc.              */
/* All rights reserved                                  */
/*                                                      */
/* Author                                               */
/*    Jikky Ye (jikkyy@hz.webex.com)                    */
/*                                                      */
/* History                                              */
/*    05/29/2008  Create                                */
/*------------------------------------------------------*/

#ifndef __WBX_SIMPLECLIENTSESSIONSINK_H__
#define __WBX_SIMPLECLIENTSESSIONSINK_H__

#include "CmBase.h"
#include "SimpleClientSession.h"

class CSimpleClientSession;

class ISimpleClientSessionSink 
{
public:
	virtual void OnClientSessionJoin(CmResult aReason) = 0;
	virtual void OnClientSessionLeave(CmResult aReason) = 0;
	virtual void OnClientReceive(CCmString& aData) = 0;

protected:
	virtual ~ISimpleClientSessionSink() {}
};
#endif	// __WBX_SIMPLECLIENTSESSIONSINK_H__
