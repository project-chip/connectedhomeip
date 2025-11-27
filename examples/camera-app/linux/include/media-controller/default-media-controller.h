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

#pragma once

#include "pushav-prerollbuffer.h"
#include <media-controller.h>
#include <mutex>
#include <vector>

namespace Camera {
class CameraDevice; // forward declaration
}
// Default Media Controller
class DefaultMediaController : public MediaController
{
public:
    DefaultMediaController() {}
    virtual ~DefaultMediaController() {}
    // Transports register themselves with the media-controller for receiving
    // media from stream sources.
    void RegisterTransport(Transport * transport, uint16_t videoStreamID, uint16_t audioStreamID) override;
    // Transports must first unregister from the media-controller when they are
    // getting destroyed.
    void UnregisterTransport(Transport * transport) override;
    // DistributeVideo and DistributeAudio are called when data is ready to be sent out
    void DistributeVideo(const uint8_t * data, size_t size, uint16_t videoStreamID) override;
    void DistributeAudio(const uint8_t * data, size_t size, uint16_t audioStreamID) override;
    // Sets the desired preroll buffer length in milliseconds for the given transport
    void SetPreRollLength(Transport * transport, uint16_t preRollBufferLength) override;
    void SetCameraDevice(Camera::CameraDevice * device);
    // Get transport registered for a specific stream ID
    Transport * GetTransportForVideoStream(uint16_t videoStreamID) override;
    Transport * GetTransportForAudioStream(uint16_t audioStreamID) override;

private:
    PreRollBuffer mPreRollBuffer;
    std::vector<Connection> mConnections;
    std::mutex mConnectionsMutex;
    std::unordered_map<Transport *, BufferSink *> mSinkMap; // map of transport to sink
    Camera::CameraDevice * mCameraDevice = nullptr;         // pointer to parent camera device
};
