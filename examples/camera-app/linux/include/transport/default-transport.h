#pragma once

class DefaultTransport : public Transport
{
public:
    void SendVideo(const char * data, size_t size, uint16_t videoStreamID) {}
    void SendAudio(const char * data, size_t size, uint16_t audioStreamID) {}
    void SendAudioVideo(const char * data, size_t size, uint16_t videoSTreamID, uint16_t audioStreamID) {}
    bool CanSendVideo() { return true; }
    bool CanSendAudio() { return true; }
    virtual ~Transport() = default;
};