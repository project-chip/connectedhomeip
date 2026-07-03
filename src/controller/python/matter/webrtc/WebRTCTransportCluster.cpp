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
#include <lib/core/CHIPError.h>

extern "C" {
    typedef int (*PyOnOfferCallback)(uint16_t, const char *);
    typedef int (*PyOnAnswerCallback)(uint16_t, const char *);
    typedef int (*PyOnICECandidatesCallback)(uint16_t, const IceCandidate *, int);
    typedef int (*PyOnEndCallback)(uint16_t, uint8_t);
}

static PyOnOfferCallback gPyOnOfferCallback = nullptr;
static PyOnAnswerCallback gPyOnAnswerCallback = nullptr;
static PyOnICECandidatesCallback gPyOnICECandidatesCallback = nullptr;
static PyOnEndCallback gPyOnEndCallback = nullptr;

static CHIP_ERROR WrapperOnOffer(uint16_t sessionId, const char * offer)
{
    if (gPyOnOfferCallback == nullptr) return CHIP_ERROR_INCORRECT_STATE;
    int status = gPyOnOfferCallback(sessionId, offer);
    return (status == 0) ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

static CHIP_ERROR WrapperOnAnswer(uint16_t sessionId, const char * answer)
{
    if (gPyOnAnswerCallback == nullptr) return CHIP_ERROR_INCORRECT_STATE;
    int status = gPyOnAnswerCallback(sessionId, answer);
    return (status == 0) ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

static CHIP_ERROR WrapperOnICECandidates(uint16_t sessionId, const IceCandidate * candidates, int count)
{
    if (gPyOnICECandidatesCallback == nullptr) return CHIP_ERROR_INCORRECT_STATE;
    int status = gPyOnICECandidatesCallback(sessionId, candidates, count);
    return (status == 0) ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

static CHIP_ERROR WrapperOnEnd(uint16_t sessionId, uint8_t reason)
{
    if (gPyOnEndCallback == nullptr) return CHIP_ERROR_INCORRECT_STATE;
    int status = gPyOnEndCallback(sessionId, reason);
    return (status == 0) ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

// These methods are expected to be called from Python.
extern "C" {
// WebRTC Requestor functions
void pychip_WebRTCTransportRequestor_Init()
{
    WebRTCTransportRequestorManager::Instance().Init();
}

void pychip_WebRTCTransportRequestor_Shutdown()
{
    WebRTCTransportRequestorManager::Instance().Shutdown();
}

void pychip_WebRTCTransportRequestor_InitCallbacks(PyOnOfferCallback onOffer, PyOnAnswerCallback onAnswer,
                                                   PyOnICECandidatesCallback onICECandidates, PyOnEndCallback onEnd)
{
    gPyOnOfferCallback = onOffer;
    gPyOnAnswerCallback = onAnswer;
    gPyOnICECandidatesCallback = onICECandidates;
    gPyOnEndCallback = onEnd;

    WebRTCTransportRequestorManager::Instance().InitCallbacks(WrapperOnOffer, WrapperOnAnswer, WrapperOnICECandidates, WrapperOnEnd);
}

// WebRTC Provider client functions
}
