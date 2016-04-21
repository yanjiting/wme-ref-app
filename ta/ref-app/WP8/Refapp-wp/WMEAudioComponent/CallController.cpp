#include "CallController.h"
#include <windows.phone.networking.voip.h>

using namespace WMEAudioComponent;
using namespace Windows::Phone::Networking::Voip;

CallController::CallController() :
	UIMainPageUri(L"/VideoPage.xaml"),
	voipServiceName(L"Ref-app"),
    voipCall(nullptr)
{
    callCoordinator = VoipCallCoordinator::GetDefault();
}

CallController::~CallController()
{
}

bool CallController::StartOutgoingCall(String ^recepientName)
{
    try
    {
        callCoordinator->RequestNewOutgoingCall(UIMainPageUri, recepientName, voipServiceName, VoipCallMedia::Audio, &voipCall);
    }
    catch (...)
    {
        return false;
    }

    return true;
}

void CallController::OnOutgoingCallAnswered()
{
    ActiveCall();
}


void CallController::OnCallEnded()
{
    EndCall();
}


String ^CallController::GetCallMessage()
{
    return callMsg;
}

bool CallController::IsCallInProgress()
{
    return voipCall != nullptr;
}

void CallController::ActiveCall()
{
    if (voipCall == nullptr)
        return;

    voipCall->NotifyCallActive();
}

void CallController::EndCall()
{
    if (voipCall == nullptr)
        return;

    voipCall->NotifyCallEnded();
    voipCall = nullptr;
}


