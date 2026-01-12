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

#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <app/clusters/scenes-server/SceneHandlerImpl.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/OnOff/Attributes.h>
#include <clusters/OnOff/Commands.h>
#include <clusters/OnOff/Ids.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app::Clusters::OnOff {

/// Implements an On/Off server cluster.
///
/// *DOES NOT* support the Lighting feature (to keep this implementation small)
class OnOffCluster : public DefaultServerCluster, public scenes::DefaultSceneHandlerImpl
{
public:
    OnOffCluster(EndpointId endpointId, TimerDelegate & timerDelegate, BitMask<Feature> featureMap = {});
    ~OnOffCluster() override;

    void AddDelegate(OnOffDelegate * delegate) { mDelegates.PushBack(delegate); }
    void RemoveDelegate(OnOffDelegate * delegate) { mDelegates.Remove(delegate); }

    /// Sets the OnOff attribute.
    ///
    /// This also handles writing the new value into non-volatile storage and
    /// notifying the delegate.
    CHIP_ERROR SetOnOff(bool on);
    bool GetOnOff() const { return mOnOff; }

    // ServerClusterInterface methods
    CHIP_ERROR Startup(ServerClusterContext & context) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    // SceneHandler implementation
    bool SupportsCluster(EndpointId endpoint, ClusterId cluster) override;
    CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes) override;
    CHIP_ERROR ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                          scenes::TransitionTimeMs timeMs) override;

protected:
    /// Allows derived classes to specify the subset of OnOff features they implement.
    /// Validates that requested 'featureMap' does not exceed implementation capabilities.
    ///
    /// This will VerifyOrDie that featureMap is a subset of supportedFeatures.
    OnOffCluster(EndpointId endpointId, TimerDelegate & timerDelegate, BitMask<Feature> featureMap,
                 BitMask<Feature> supportedFeatures);

    IntrusiveList<OnOffDelegate, IntrusiveMode::AutoUnlink> mDelegates;
    BitMask<Feature> mFeatureMap;

    // Attribute local storage
    bool mOnOff = false;

    // Timer support
    TimerDelegate & mTimerDelegate;

private:
    class SceneTransitionTimer : public TimerContext
    {
    public:
        SceneTransitionTimer(OnOffCluster & cluster) : mCluster(cluster) {}

        void Start(uint32_t timeMs, bool targetValue);
        void Cancel();
        void TimerFired() override;

    private:
        OnOffCluster & mCluster;
        bool mTargetValue = false;
    };

    SceneTransitionTimer mSceneTimer;

    friend class OnOffClusterTestAccess;
};

} // namespace chip::app::Clusters::OnOff
