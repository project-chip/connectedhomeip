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

#include <app/InteractionModelEngine.h>
#include <app/SubscriptionResumptionHelper.h>

namespace chip {
namespace app {
SubscriptionResumptionHelper::SubscriptionResumptionHelper() :
    mOnConnectedCallback(HandleDeviceConnected, this), mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
{}

CHIP_ERROR SubscriptionResumptionHelper::ResumeSubscription(CASESessionManager & caseSessionManager,
                                                            SubscriptionResumptionStorage::SubscriptionInfo & subscriptionInfo)
{
    mNodeId         = subscriptionInfo.mNodeId;
    mFabricIndex    = subscriptionInfo.mFabricIndex;
    mSubscriptionId = subscriptionInfo.mSubscriptionId;
    mMinInterval    = subscriptionInfo.mMinInterval;
    mMaxInterval    = subscriptionInfo.mMaxInterval;
    mFabricFiltered = subscriptionInfo.mFabricFiltered;
    mAttributePaths = std::move(subscriptionInfo.mAttributePaths);
    mEventPaths     = std::move(subscriptionInfo.mEventPaths);

    ScopedNodeId peerNode = ScopedNodeId(mNodeId, mFabricIndex);
    caseSessionManager.FindOrEstablishSession(peerNode, &mOnConnectedCallback, &mOnConnectionFailureCallback);
    return CHIP_NO_ERROR;
}

void SubscriptionResumptionHelper::HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                                         const SessionHandle & sessionHandle)
{
    Platform::UniquePtr<SubscriptionResumptionHelper> _this(static_cast<SubscriptionResumptionHelper *>(context));
    InteractionModelEngine * imEngine = InteractionModelEngine::GetInstance();
    if (!imEngine->EnsureResourceForSubscription(_this->mFabricIndex, _this->mAttributePaths.AllocatedSize(),
                                                 _this->mEventPaths.AllocatedSize()))
    {
        ChipLogProgress(InteractionModel, "no resource for subscription resumption");
        return;
    }
    ReadHandler * readHandler = imEngine->mReadHandlers.CreateObject(*imEngine, imEngine->GetReportScheduler());
    if (readHandler == nullptr)
    {
        ChipLogProgress(InteractionModel, "no resource for ReadHandler creation");
        return;
    }
    readHandler->OnSubscriptionResumed(sessionHandle, *_this);
}

void SubscriptionResumptionHelper::HandleDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    Platform::UniquePtr<SubscriptionResumptionHelper> _this(static_cast<SubscriptionResumptionHelper *>(context));
    ChipLogError(DataManagement, "Failed to establish CASE for subscription-resumption with error '%" CHIP_ERROR_FORMAT "'",
                 error.Format());
    // If the device fails to establish the session, the subscriber might be offline and its subscription read client will
    // be deleted after the device reconnect to the subscriber. This subscription will be never used again. So clean up
    // the persistent subscription information storage.
    auto * subscriptionResumptionStorage = InteractionModelEngine::GetInstance()->GetSubscriptionResumptionStorage();
    if (subscriptionResumptionStorage)
    {
        subscriptionResumptionStorage->Delete(_this->mNodeId, _this->mFabricIndex, _this->mSubscriptionId);
    }
}

} // namespace app
} // namespace chip
