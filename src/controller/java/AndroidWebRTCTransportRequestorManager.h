/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <jni.h>
#include <lib/core/CHIPError.h>
#include <controller/webrtc/WebRTCTransportRequestorManager.h>

namespace chip {
namespace Controller {

class AndroidWebRTCTransportRequestorManager
{
public:
    static AndroidWebRTCTransportRequestorManager & Instance()
    {
        static AndroidWebRTCTransportRequestorManager instance;
        return instance;
    }

    /**
     * Registers the callback object received from the Java layer and initializes the JNI environment.
     * Internally, it calls InitCallbacks of the existing ::AndroidWebRTCTransportRequestorManager.
     */
    void Init(JNIEnv * env, jobject javaCallbackObj);

    /**
     * Releases the global reference of the registered Java callback object and cleans up resources.
     */
    void Shutdown(JNIEnv * env);

private:
    AndroidWebRTCTransportRequestorManager() = default;
    ~AndroidWebRTCTransportRequestorManager() = default;

    // Static callback functions to be passed to the existing C++ Manager (InitCallbacks).
    static CHIP_ERROR OnOffer(uint16_t sessionId, const char * offer);
    static CHIP_ERROR OnAnswer(uint16_t sessionId, const char * answer);
    static CHIP_ERROR OnICECandidates(uint16_t sessionId, const IceCandidate * candidates, int candidateCount);
    static CHIP_ERROR OnEnd(uint16_t sessionId, uint8_t reason);

    // Stores the global reference of the Java callback object.
    jobject mJavaCallbackObj = nullptr;

    // Caches the Java callback method IDs to improve JNI call performance.
    jmethodID mOnOfferMethod = nullptr;
    jmethodID mOnAnswerMethod = nullptr;
    jmethodID mOnICECandidatesMethod = nullptr;
    jmethodID mOnEndMethod = nullptr;

    jclass mIceCandidateClass = nullptr;
};

} // namespace Controller
} // namespace chip