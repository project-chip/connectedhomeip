/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/AttributePathParams.h>
#include <app/CASESessionManager.h>
#include <app/SubscriptionResumptionStorage.h>

namespace chip {
namespace app {

/**
 *  Session Establisher to resume persistent subscription. A CASE session will be established upon invoking
 *  ResumeSubscription(), followed by the creation and intialization of a ReadHandler. This class helps prevent
 *  a scenario where all ReadHandlers in the pool grab the invalid session handle. In such scenario, if the device
 *  receives a new subscription request, it will crash as there is no evictable ReadHandler.
 */

class SubscriptionResumptionSessionEstablisher
{
public:
    SubscriptionResumptionSessionEstablisher();

    CHIP_ERROR ResumeSubscription(CASESessionManager & caseSessionManager,
                                  const SubscriptionResumptionStorage::SubscriptionInfo & subscriptionInfo);

    SubscriptionResumptionStorage::SubscriptionInfo mSubscriptionInfo;

private:
    // Callback funstions for continuing the subscription resumption
    static void HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                      const SessionHandle & sessionHandle);
    static void HandleDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    // Callbacks to handle server-initiated session success/failure
    chip::Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback;
};
} // namespace app
} // namespace chip
