/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <iostream>
#include <memory>

#include "media/base/video_broadcaster.h"
#include "pc/video_track_source.h"

class FakeVideoTrackSource : public webrtc::VideoTrackSource
{
public:
    static rtc::scoped_refptr<FakeVideoTrackSource> Create(bool is_screencast)
    {
        return rtc::make_ref_counted<FakeVideoTrackSource>(is_screencast);
    }

    static rtc::scoped_refptr<FakeVideoTrackSource> Create() { return Create(false); }

    bool is_screencast() const override { return is_screencast_; }

    void InjectFrame(const webrtc::VideoFrame & frame) { video_broadcaster_.OnFrame(frame); }

protected:
    explicit FakeVideoTrackSource(bool is_screencast) : webrtc::VideoTrackSource(false /* remote */), is_screencast_(is_screencast)
    {}
    ~FakeVideoTrackSource() override = default;

    rtc::VideoSourceInterface<webrtc::VideoFrame> * source() override { return &video_broadcaster_; }

private:
    const bool is_screencast_;
    rtc::VideoBroadcaster video_broadcaster_;
};
