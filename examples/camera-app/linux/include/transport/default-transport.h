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
#include <transport.h>

#pragma once

class DefaultTransport : public Transport
{
public:
    void SendVideo(const chip::ByteSpan & data, int64_t timestamp, uint16_t videoStreamID) {}
    void SendAudio(const chip::ByteSpan & data, int64_t timestamp, uint16_t audioStreamID) {}
    void SendAudioVideo(const chip::ByteSpan & data, uint16_t videoStreamID, uint16_t audioStreamID) {}
    bool CanSendVideo() { return true; }
    bool CanSendAudio() { return true; }
    virtual ~Transport() = default;
};
