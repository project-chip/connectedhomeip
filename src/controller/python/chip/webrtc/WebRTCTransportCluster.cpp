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

#include <controller/webrtc/WebRTCTransportRequestorManager.h>
// These methods are expected to be called from Python.
extern "C" {
// WebRTC Requestor functions
void pychip_WebRTCTransportRequestor_Init()
{
    WebRTCTransportRequestorManager::Instance().Init();
}

void pychip_WebRTCTransportRequestor_InitCallbacks(OnOfferCallback onOnOfferCallback, OnAnswerCallback onAnswerCallback,
                                                   OnICECandidatesCallback onICECandidatesCallback, OnEndCallback onEndCallback)
{
    WebRTCTransportRequestorManager::Instance().InitCallbacks(onOnOfferCallback, onAnswerCallback, onICECandidatesCallback,
                                                              onEndCallback);
}

// WebRTC Provider client functions
}
