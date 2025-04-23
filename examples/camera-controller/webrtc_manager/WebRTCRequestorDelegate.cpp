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

#include "WebRTCRequestorDelegate.h"
#include "WebRTCManager.h"

#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WebRTCTransportRequestor;

CHIP_ERROR WebRTCRequestorDelegate::HandleOffer(uint16_t sessionId, const OfferArgs & args, WebRTCSessionTypeStruct & outSession)
{
    ChipLogProgress(Camera, "WebRTCRequestorDelegate::HandleOffer");
    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCRequestorDelegate::HandleAnswer(uint16_t sessionId, const std::string & sdpAnswer)
{
    ChipLogProgress(Camera, "WebRTCRequestorDelegate::HandleAnswer");
    return WebRTCManager::Instance().SetRemoteDescription(sessionId, sdpAnswer);
}

CHIP_ERROR WebRTCRequestorDelegate::HandleICECandidates(uint16_t sessionId, const std::vector<std::string> & candidates)
{
    ChipLogProgress(Camera, "WebRTCRequestorDelegate::HandleICECandidates");
    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCRequestorDelegate::HandleEnd(uint16_t sessionId, WebRTCEndReasonEnum reasonCode)
{
    ChipLogProgress(Camera, "WebRTCRequestorDelegate::HandleEnd");
    return CHIP_NO_ERROR;
}
