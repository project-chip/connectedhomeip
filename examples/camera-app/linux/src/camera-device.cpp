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

#include "camera-device.h"
#include <AppMain.h>
#include <iostream>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Chime;
using namespace chip::app::Clusters::CameraAvStreamManagement;
using namespace Camera;

CameraDevice::CameraDevice()
{
    // set up the different modules/components
    mCameraAVStreamManager.Init();
}
#if 0
void CameraDevice::InitializeAvailableVideoStreams()
{
    // Example initialization with different codecs
    // id, isAllocated, codec, fps
    videoStreams.push_back({ 0, false, VideoCodecEnum::kH264, 24 });
    videoStreams.push_back({ 1, false, VideoCodecEnum::kH264, 30 });
    videoStreams.push_back({ 2, false, VideoCodecEnum::kHevc, 30 });
    videoStreams.push_back({ 3, false, VideoCodecEnum::kHevc, 60 });
}

void CameraDevice::InitializeAvailableAudioStreams()
{
    // Example initialization with different codecs
    // id, isAllocated, codec, channelCount
    audioStreams.push_back({ 0, false, AudioCodecEnum::kOpus, 2 });
    audioStreams.push_back({ 1, false, AudioCodecEnum::kOpus, 2 });
    audioStreams.push_back({ 2, false, AudioCodecEnum::kAacLc, 1 });
    audioStreams.push_back({ 3, false, AudioCodecEnum::kAacLc, 2 });
}

void CameraDevice::InitializeAvailableSnapshotStreams()
{
    // Example initialization with different codecs
    // id, isAllocated, codec, quality
    snapshotStreams.push_back({ 0, false, ImageCodecEnum::kJpeg, 80 });
    snapshotStreams.push_back({ 1, false, ImageCodecEnum::kJpeg, 90 });
    snapshotStreams.push_back({ 2, false, ImageCodecEnum::kJpeg, 90 });
    snapshotStreams.push_back({ 3, false, ImageCodecEnum::kJpeg, 80 });
}

void CameraDevice::InitializeStreams()
{
    InitializeAvailableVideoStreams();

    InitializeAvailableAudioStreams();

    InitializeAvailableSnapshotStreams();
}
#endif
ChimeDelegate & CameraDevice::GetChimeDelegate()
{
    return mChimeManager;
}

WebRTCTransportProvider::Delegate & CameraDevice::GetWebRTCProviderDelegate()
{
    return mWebRTCProviderManager;
}

CameraAVStreamMgmtDelegate & CameraDevice::GetCameraAVStreamMgmtDelegate()
{
    return mCameraAVStreamManager;
}
