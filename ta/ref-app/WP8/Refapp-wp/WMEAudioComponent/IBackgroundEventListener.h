#pragma once

namespace WMEAudioComponent
{
    public interface class IBackgroundEventListener
    {
        void OnBackgroundDataAvailable(Platform::String^ key, Platform::String^ value);
    };
}
