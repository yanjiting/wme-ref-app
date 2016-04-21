#pragma once

using namespace Platform;

namespace WMEAudioComponent
{

    // A method that is called back when the incoming call dialog has been dismissed.
    // This callback is used to complete the incoming call agent.
    //public delegate void IncomingCallDialogDismissedCallback(bool callAccepted);

    // A class that provides methods and properties related to VoIP calls.
    // It wraps Windows.Phone.Networking.Voip.VoipCallCoordinator, and provides app-specific call functionality.
    public ref class CallController sealed
    {
    public:
        CallController();
        virtual ~CallController();

        bool    StartOutgoingCall(String^ recepientName);
        void    OnOutgoingCallAnswered();
        void    OnCallEnded();
        String^ GetCallMessage();
        bool    IsCallInProgress();
    private:
        void    ActiveCall();
        void    EndCall();

        String^               voipServiceName;
        String^               UIMainPageUri;
        String^               callMsg;


        Windows::Phone::Networking::Voip::VoipPhoneCall^        voipCall;
        Windows::Phone::Networking::Voip::VoipCallCoordinator^  callCoordinator;


    };
}
