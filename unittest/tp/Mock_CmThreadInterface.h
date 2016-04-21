
#ifndef __MOCK_CMTHREAD_INTERFACE_H__
#define __MOCK_CMTHREAD_INTERFACE_H__

#include "CmThreadInterface.h"


class MockICmEvent : public ICmEvent {
 public:
  MOCK_METHOD0(OnEventFire,
      CmResult());
  MOCK_METHOD0(OnDestorySelf,
      void());
};

class MockICmEventQueue : public ICmEventQueue {
 public:
  MOCK_METHOD2(PostEvent,
      CmResult(ICmEvent *aEvent, EPriority aPri));
  MOCK_METHOD1(SendEvent,
      CmResult(ICmEvent *aEvent));
  MOCK_METHOD0(GetPendingEventsCount,
      DWORD());
};

class MockICmTimerHandler : public ICmTimerHandler {
 public:
  MOCK_METHOD2(OnTimeout,
      void(const CCmTimeValue &aCurTime, LPVOID aArg));
};

class MockICmTimerQueue : public ICmTimerQueue {
 public:
  MOCK_METHOD4(ScheduleTimer,
      CmResult(ICmTimerHandler *aTh, LPVOID aArg, const CCmTimeValue &aInterval,  DWORD aCount));
  MOCK_METHOD1(CancelTimer,
      CmResult(ICmTimerHandler *aTh));
};

#endif