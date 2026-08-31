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

// Level Control has no hardware/AudioControl hook wired up yet in this phase, so it gets
// a no-op delegate (LevelControlDelegate's virtual methods all have empty default bodies).
LevelControlDelegate gNoOpLevelControlDelegate;

LazyRegisteredServerCluster<OnOffCluster> gOnOffCluster;
LazyRegisteredServerCluster<LevelControlCluster> gLevelControlCluster;

} // namespace

namespace chip::app::Clusters::Speaker {

void InitOnOffLevelControl()
{
    // Seeded from tv-app.zap's declared defaults via the generated Get() accessor, which reads
    // the Ember RAM buffer Server::Init() populated from that same defaultValue -- these
    // attributes are kept "ram" (not "callback") in tv-app.zap for exactly this reason. Nothing
    // else writes to them before this runs, so the read is equivalent to reading the .zap default
    // directly. Falls back to a neutral value if the read ever fails (defensive; matches the
    // established CodegenIntegration.cpp idiom used throughout the tree).
    bool onOffDefault = false;
    if (OnOff::Attributes::OnOff::Get(kSpeakerEndpointId, &onOffDefault) != Status::Success)
    {
        onOffDefault = false;
    }
    OnOffCluster::Context onOffContext{ gTimerDelegate };
    onOffContext.defaults.onOff = onOffDefault;
    gOnOffCluster.Create(kSpeakerEndpointId, onOffContext);

    uint8_t minLevel = 0;
    if (LevelControl::Attributes::MinLevel::Get(kSpeakerEndpointId, &minLevel) != Status::Success)
    {
        minLevel = 0;
    }
    uint8_t maxLevel = 0xFF;
    if (LevelControl::Attributes::MaxLevel::Get(kSpeakerEndpointId, &maxLevel) != Status::Success)
    {
        maxLevel = 0xFF;
    }
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
    // OnTransitionTime, OffTransitionTime and DefaultMoveRate are declared with an empty (null)
    // default in tv-app.zap. Unlike a real literal default, ZAP does not seed the RAM buffer with
    // the traits' null sentinel for an empty default -- it zero-fills it -- so Get() would read
    // back 0, not null. These three stay hardcoded to NullNullable to match the actual .zap intent
    // rather than what the (inapplicable here) Get() mechanism would report.
    DataModel::Nullable<uint16_t> onTransitionTime  = DataModel::NullNullable;
    DataModel::Nullable<uint16_t> offTransitionTime = DataModel::NullNullable;
    DataModel::Nullable<uint8_t> defaultMoveRate    = DataModel::NullNullable;

    // Lighting is intentionally not enabled: MA-speaker's FeatureMap only ever declared the OnOff
    // feature, and StartUpCurrentLevel/RemainingTime are spec-scoped to the Lighting feature --
    // the code-driven cluster now correctly hides them for a non-Lighting instance, where the
    // legacy Ember plugin exposed them regardless of feature support.
    LevelControlCluster::Config levelControlConfig(gTimerDelegate, gNoOpLevelControlDelegate);
    levelControlConfig.WithOnOff(gOnOffCluster.Cluster());
    levelControlConfig.WithMinLevel(minLevel);
    levelControlConfig.WithMaxLevel(maxLevel);
    levelControlConfig.WithOnOffTransitionTime(onOffTransitionTime);
    levelControlConfig.WithOnTransitionTime(onTransitionTime);
    levelControlConfig.WithOffTransitionTime(offTransitionTime);
    levelControlConfig.WithDefaultMoveRate(defaultMoveRate);
    // CurrentLevel is the one exception left hardcoded: it is still "callback" (External) in
    // tv-app.zap because the legacy Level Control Ember plugin's init callback unconditionally
    // resolves StartUpCurrentLevel and overwrites CurrentLevel's RAM buffer during Server::Init()
    // (see SpeakerOnOffLevelControl.h), so that buffer cannot be trusted to still hold the raw
    // .zap default by the time this code runs. 0x00 matches tv-app.zap's originally-declared
    // CurrentLevel default.
    levelControlConfig.WithInitialCurrentLevel(0x00);
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
