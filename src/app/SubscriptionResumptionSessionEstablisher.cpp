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
#include <app/SubscriptionResumptionSessionEstablisher.h>

namespace chip {
namespace app {

class AutoDeleteEstablisher
{
public:
    AutoDeleteEstablisher(SubscriptionResumptionSessionEstablisher * sessionEstablisher) : mSessionEstablisher(sessionEstablisher)
    {}
    ~AutoDeleteEstablisher() { chip::Platform::Delete(mSessionEstablisher); }

    SubscriptionResumptionSessionEstablisher * operator->() const { return mSessionEstablisher; }

    SubscriptionResumptionSessionEstablisher & operator*() const { return *mSessionEstablisher; }

private:
    SubscriptionResumptionSessionEstablisher * mSessionEstablisher;
};

SubscriptionResumptionSessionEstablisher::SubscriptionResumptionSessionEstablisher() :
    mOnConnectedCallback(HandleDeviceConnected, this), mOnConnectionFailureCallback(HandleDeviceConnectionFailure, this)
{}

CHIP_ERROR
SubscriptionResumptionSessionEstablisher::ResumeSubscription(
    CASESessionManager & caseSessionManager, const SubscriptionResumptionStorage::SubscriptionInfo & subscriptionInfo)
{
    mSubscriptionInfo.mNodeId         = subscriptionInfo.mNodeId;
    mSubscriptionInfo.mFabricIndex    = subscriptionInfo.mFabricIndex;
    mSubscriptionInfo.mSubscriptionId = subscriptionInfo.mSubscriptionId;
    mSubscriptionInfo.mMinInterval    = subscriptionInfo.mMinInterval;
    mSubscriptionInfo.mMaxInterval    = subscriptionInfo.mMaxInterval;
    mSubscriptionInfo.mFabricFiltered = subscriptionInfo.mFabricFiltered;
    // Copy the Attribute Paths and Event Paths
    if (subscriptionInfo.mAttributePaths.AllocatedSize() > 0)
    {
        mSubscriptionInfo.mAttributePaths.Alloc(subscriptionInfo.mAttributePaths.AllocatedSize());
        if (!mSubscriptionInfo.mAttributePaths.Get())
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        for (size_t i = 0; i < mSubscriptionInfo.mAttributePaths.AllocatedSize(); ++i)
        {
            mSubscriptionInfo.mAttributePaths[i] = subscriptionInfo.mAttributePaths[i];
        }
    }
    if (subscriptionInfo.mEventPaths.AllocatedSize() > 0)
    {
        mSubscriptionInfo.mEventPaths.Alloc(subscriptionInfo.mEventPaths.AllocatedSize());
        if (!mSubscriptionInfo.mEventPaths.Get())
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        for (size_t i = 0; i < mSubscriptionInfo.mEventPaths.AllocatedSize(); ++i)
        {
            mSubscriptionInfo.mEventPaths[i] = subscriptionInfo.mEventPaths[i];
        }
    }

    ScopedNodeId peerNode = ScopedNodeId(mSubscriptionInfo.mNodeId, mSubscriptionInfo.mFabricIndex);
    caseSessionManager.FindOrEstablishSession(peerNode, &mOnConnectedCallback, &mOnConnectionFailureCallback);
    return CHIP_NO_ERROR;
}

void SubscriptionResumptionSessionEstablisher::HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                                                     const SessionHandle & sessionHandle)
{
    AutoDeleteEstablisher establisher(static_cast<SubscriptionResumptionSessionEstablisher *>(context));
    SubscriptionResumptionStorage::SubscriptionInfo & subscriptionInfo = establisher->mSubscriptionInfo;
    InteractionModelEngine * imEngine                                  = InteractionModelEngine::GetInstance();
    if (!imEngine->EnsureResourceForSubscription(subscriptionInfo.mFabricIndex, subscriptionInfo.mAttributePaths.AllocatedSize(),
                                                 subscriptionInfo.mEventPaths.AllocatedSize()))
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
    readHandler->OnSubscriptionResumed(sessionHandle, *establisher);
}

void SubscriptionResumptionSessionEstablisher::HandleDeviceConnectionFailure(void * context, const ScopedNodeId & peerId,
                                                                             CHIP_ERROR error)
{
    AutoDeleteEstablisher establisher(static_cast<SubscriptionResumptionSessionEstablisher *>(context));
    SubscriptionResumptionStorage::SubscriptionInfo & subscriptionInfo = establisher->mSubscriptionInfo;
    ChipLogError(DataManagement, "Failed to establish CASE for subscription-resumption with error '%" CHIP_ERROR_FORMAT "'",
                 error.Format());
    // If the device fails to establish the session, the subscriber might be offline and its subscription read client will
    // be deleted when the device reconnect to the subscriber. This subscription will be never used again. So clean up
    // the persistent subscription information storage.
    auto * subscriptionResumptionStorage = InteractionModelEngine::GetInstance()->GetSubscriptionResumptionStorage();
    if (subscriptionResumptionStorage)
    {
        subscriptionResumptionStorage->Delete(subscriptionInfo.mNodeId, subscriptionInfo.mFabricIndex,
                                              subscriptionInfo.mSubscriptionId);
    }
}

} // namespace app
} // namespace chip
