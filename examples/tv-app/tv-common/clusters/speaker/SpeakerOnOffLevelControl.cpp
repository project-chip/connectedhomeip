/*
 *
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

#include "speaker/SpeakerOnOffLevelControl.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/level-control/LevelControlCluster.h>
#include <app/clusters/level-control/LevelControlDelegate.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DefaultTimerDelegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace {

// MA-speaker (tv-app.zap) is endpoint 2.
constexpr EndpointId kSpeakerEndpointId = 2;

DefaultTimerDelegate gTimerDelegate;

// Level Control has no hardware/AudioControl hook wired up yet on this endpoint, so it
// gets a no-op delegate (LevelControlDelegate's virtual methods all have empty default
// bodies).
LevelControlDelegate gNoOpLevelControlDelegate;

LazyRegisteredServerCluster<OnOffCluster> gOnOffCluster;
LazyRegisteredServerCluster<LevelControlCluster> gLevelControlCluster;

} // namespace

namespace chip::app::Clusters::Speaker {

void InitOnOffLevelControl()
{
    // Both clusters below persist their own state (CurrentLevel, StartUpCurrentLevel, OnOff)
    // via the same AttributePersistenceProvider keys -- (endpoint, cluster, attribute) -- that
    // the legacy Ember "persist"/NVM storage option already used, so on the second and later
    // boots these Get() reads are academic: Startup() below reloads the real persisted value
    // itself and only falls back to what is seeded here when nothing has been persisted yet.
    // These reads exist purely so that FIRST-boot behavior matches tv-app.zap's declared
    // defaults instead of a hardcoded literal, mirroring the established CodegenIntegration.cpp
    // idiom used throughout the tree.
    //
    // Once registered below, CodegenDataModelProvider's registry lookup (CodegenDataModelProvider.cpp)
    // is checked before the Ember codegen path for every read/write/command on this (endpoint,
    // cluster) pair, so tv-app.zap's "ram"/"persist" storage options for these attributes become
    // dead metadata for endpoint 2 from that point on -- left as-is (rather than marked "External")
    // because nothing needs to change there for correctness, and it keeps the single .zap file an
    // accurate record of each attribute's declared default for every endpoint that still uses it.
    bool onOffDefault = false;
    if (OnOff::Attributes::OnOff::Get(kSpeakerEndpointId, &onOffDefault) != Status::Success)
    {
        onOffDefault = false;
    }
    OnOffCluster::Context onOffContext{ gTimerDelegate };
    onOffContext.defaults.onOff = onOffDefault;
    gOnOffCluster.Create(kSpeakerEndpointId, onOffContext);

    uint16_t onOffTransitionTime = 0;
    if (LevelControl::Attributes::OnOffTransitionTime::Get(kSpeakerEndpointId, &onOffTransitionTime) != Status::Success)
    {
        onOffTransitionTime = 0;
    }
    DataModel::Nullable<uint8_t> onLevel;
    if (LevelControl::Attributes::OnLevel::Get(kSpeakerEndpointId, onLevel) != Status::Success)
    {
        onLevel.SetNull();
    }
    DataModel::Nullable<uint16_t> onTransitionTime;
    if (LevelControl::Attributes::OnTransitionTime::Get(kSpeakerEndpointId, onTransitionTime) != Status::Success)
    {
        onTransitionTime.SetNull();
    }
    DataModel::Nullable<uint16_t> offTransitionTime;
    if (LevelControl::Attributes::OffTransitionTime::Get(kSpeakerEndpointId, offTransitionTime) != Status::Success)
    {
        offTransitionTime.SetNull();
    }
    DataModel::Nullable<uint8_t> defaultMoveRate;
    if (LevelControl::Attributes::DefaultMoveRate::Get(kSpeakerEndpointId, defaultMoveRate) != Status::Success)
    {
        defaultMoveRate.SetNull();
    }
    DataModel::Nullable<uint8_t> startUpCurrentLevel;
    if (LevelControl::Attributes::StartUpCurrentLevel::Get(kSpeakerEndpointId, startUpCurrentLevel) != Status::Success)
    {
        startUpCurrentLevel.SetNull();
    }
    DataModel::Nullable<uint8_t> currentLevel;
    if (LevelControl::Attributes::CurrentLevel::Get(kSpeakerEndpointId, currentLevel) != Status::Success || currentLevel.IsNull())
    {
        // Only reachable on a Get() failure or if the legacy plugin's own init resolved a null
        // CurrentLevel, neither of which tv-app.zap's declared defaults trigger today. Fall back
        // to the class's own published Lighting minimum rather than an arbitrary literal.
        currentLevel.SetNonNull(LevelControlCluster::kLightingMinLevel);
    }

    // MA-speaker's FeatureMap declares both OnOff and Lighting (tv-app.zap). WithLighting()
    // sets MinLevel/MaxLevel to the spec-mandated 1/254 itself -- LevelControlCluster::Config
    // asserts on any other value for a Lighting-enabled instance, so those two are not read
    // from ZAP/passed here separately.
    LevelControlCluster::Config levelControlConfig(gTimerDelegate, gNoOpLevelControlDelegate);
    levelControlConfig.WithOnOff(gOnOffCluster.Cluster());
    levelControlConfig.WithLighting(startUpCurrentLevel);
    levelControlConfig.WithOnOffTransitionTime(onOffTransitionTime);
    levelControlConfig.WithOnTransitionTime(onTransitionTime);
    levelControlConfig.WithOffTransitionTime(offTransitionTime);
    levelControlConfig.WithDefaultMoveRate(defaultMoveRate);
    levelControlConfig.WithInitialCurrentLevel(currentLevel.Value());
    gLevelControlCluster.Create(kSpeakerEndpointId, levelControlConfig);
    gLevelControlCluster.Cluster().SetOnLevel(onLevel);

    // Level Control reacts to On/Off changes the same way the legacy Ember plugins coupled
    // these two clusters (turning the speaker off moves/restores CurrentLevel).
    gOnOffCluster.Cluster().AddDelegate(&gLevelControlCluster.Cluster());

    SingleEndpointServerClusterRegistry & registry = CodegenDataModelProvider::Instance().Registry();

    // Registry::Register() starts the cluster immediately (Server::Init() has already set up
    // the registry's context by the time this runs), so both clusters must be fully
    // constructed and wired above before either is registered.
    CHIP_ERROR err = registry.Register(gOnOffCluster.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "TV Linux App: Speaker OnOff cluster registration failed: %" CHIP_ERROR_FORMAT, err.Format());
    }

    err = registry.Register(gLevelControlCluster.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "TV Linux App: Speaker LevelControl cluster registration failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ShutdownOnOffLevelControl()
{
    SingleEndpointServerClusterRegistry & registry = CodegenDataModelProvider::Instance().Registry();

    if (gLevelControlCluster.IsConstructed())
    {
        LogErrorOnFailure(registry.Unregister(&gLevelControlCluster.Cluster()));
        if (gOnOffCluster.IsConstructed())
        {
            gOnOffCluster.Cluster().RemoveDelegate(&gLevelControlCluster.Cluster());
        }
        gLevelControlCluster.Destroy();
    }
    if (gOnOffCluster.IsConstructed())
    {
        LogErrorOnFailure(registry.Unregister(&gOnOffCluster.Cluster()));
        gOnOffCluster.Destroy();
    }
}

} // namespace chip::app::Clusters::Speaker
