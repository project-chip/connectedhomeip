
#pragma once
// Base class for media transports(WebRTC, PushAV)
class Transport
{
public:
    virtual void SendVideo(const char * data, size_t size, uint16_t videoStreamID) = 0;
    virtual void SendAudio(const char * data, size_t size, uint16_t audioStreamID) = 0;
    virtual void SendAudioVideo(const char * data, size_t size, uint16_t videoSTreamID, uint16_t audioStreamID) = 0;
    virtual bool CanSendVideo() = 0;
    virtual bool CanSendAudio() = 0;
    virtual ~Transport() = default;
};