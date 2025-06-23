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

#pragma once
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <rtc/rtc.hpp>
#include <string>

namespace chip {
namespace webrtc {

class WebRTCClient
{
public:
    WebRTCClient();
    ~WebRTCClient();

    void createPeerConnection(const std::string & stunUrl);
    void createOffer();
    void createAnswer();
    void setRemoteDescription(const std::string & sdp, const std::string & type);
    void addIceCandidate(const std::string & candidate, const std::string & mid);

    void onLocalDescription(std::function<void(const std::string &, const std::string &)> callback);
    void onIceCandidate(std::function<void(const std::string &, const std::string &)> callback);

private:
    std::shared_ptr<rtc::PeerConnection> pc_;
    std::function<void(const std::string &, const std::string &)> localDescriptionCallback_;
    std::function<void(const std::string &, const std::string &)> iceCandidateCallback_;
};

} // namespace webrtc
} // namespace chip
