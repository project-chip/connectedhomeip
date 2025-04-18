/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <transport/pushav-transport.h>

PushAVTransport::PushAVTransport(uint16_t sessionID, uint64_t nodeID) {}

// Destructor definition
PushAVTransport::~PushAVTransport()
{
    // Destructor implementation
}
// Dummy implementation of SendVideo method
void PushAVTransport::SendVideo(const char * data, size_t size, uint16_t videoStreamID)
{
    // Placeholder for actual PushAV implementation to send video data
}

// Dummy implementation of SendAudio method
void PushAVTransport::SendAudio(const char * data, size_t size, uint16_t audioStreamID)
{
    // Placeholder for actual PushAV implementation to send audio data
}

// Dummy implementation of SendAudioVideo method
void PushAVTransport::SendAudioVideo(const char * data, size_t size, uint16_t videoStreamID, uint16_t audioStreamID)
{
    // Placeholder for actual PushAV implementation to send synchronized audio/video data
}

// Dummy implementation of CanSendVideo method
bool PushAVTransport::CanSendVideo()
{
    return mCanSendVideo;
}

// Dummy implementation of CanSendAudio method
bool PushAVTransport::CanSendAudio()
{
    return mCanSendAudio;
}
