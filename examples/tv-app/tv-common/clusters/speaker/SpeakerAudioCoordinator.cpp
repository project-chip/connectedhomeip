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

#include "speaker/SpeakerAudioCoordinator.h"

#include <algorithm>
#include <cmath>

#include <clusters/LevelControl/Enums.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace {

// RAII: marks a coordinator-driven sync in progress so the reverse cluster notification is
// ignored while it runs.
class ReentrancyGuard
{
public:
    explicit ReentrancyGuard(bool & flag) : mFlag(flag) { mFlag = true; }
    ~ReentrancyGuard() { mFlag = false; }

private:
    bool & mFlag;
};

BitMask<LevelControl::OptionsBitmap> ExecuteIfOff()
{
    return BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff);
}

} // namespace

namespace chip::app::Clusters::Speaker {

// ---------------------------------------------------------------------------------------
// Construction / destruction -- the coordinator owns the three clusters.
// ---------------------------------------------------------------------------------------

SpeakerAudioCoordinator::SpeakerAudioCoordinator(const Config & config) : mHardware(config.hardwareDelegate)
{
    // On/Off first: Level Control's config references the On/Off cluster.
    OnOffCluster::Context onOffContext{ config.timerDelegate };
    onOffContext.defaults.onOff = config.onOffStartValue;
    mOnOff.Create(config.endpoint, onOffContext);
    mOnOff.Cluster().AddDelegate(this);

    // Level Control config binds this coordinator as delegate, so it is built here.
    // WithLighting() sets MinLevel/MaxLevel to the spec 1/254; WithOnOff() lets Level Control's
    // own *WithOnOff commands drive On/Off.
    LevelControlCluster::Config levelControlConfig(config.timerDelegate, *this);
    levelControlConfig.WithOnOff(mOnOff.Cluster());
    levelControlConfig.WithLighting(config.levelControlStartUpCurrentLevel);
    levelControlConfig.WithOnOffTransitionTime(config.levelControlOnOffTransitionTime);
    levelControlConfig.WithOnTransitionTime(config.levelControlOnTransitionTime);
    levelControlConfig.WithOffTransitionTime(config.levelControlOffTransitionTime);
    levelControlConfig.WithDefaultMoveRate(config.levelControlDefaultMoveRate);
    levelControlConfig.WithInitialCurrentLevel(config.levelControlInitialCurrentLevel);
    mLevelControl.Create(config.endpoint, levelControlConfig);

    // OnLevel NULL is deliberate (see the header): a raw Off then runs Level Control's internal
    // fade-to-off and restores CurrentLevel to the pre-off value (LevelControlCluster.cpp
    // kInternalOffTransition), so CurrentLevel -- and the mirrored Volume -- survives a mute.
    mLevelControl.Cluster().SetOnLevel(config.levelControlOnLevel);

    // Level Control is the endpoint's second On/Off delegate (see the header): raw On/Off/Toggle
    // then runs its CurrentLevel choreography, mirrored to AudioControl.Volume by OnLevelChanged().
    mOnOff.Cluster().AddDelegate(&mLevelControl.Cluster());

    // Audio Control: delegate is this coordinator (constructor argument), config is caller-built.
    mAudioControl.Create(config.endpoint, *this, config.audioControl);
}

SpeakerAudioCoordinator::~SpeakerAudioCoordinator()
{
    // Detach both On/Off delegates while all clusters are still alive; the members then destroy
    // in reverse (Audio Control, Level Control, On/Off). The owner must have unregistered all
    // three from the registry before this runs.
    mOnOff.Cluster().RemoveDelegate(&mLevelControl.Cluster());
    mOnOff.Cluster().RemoveDelegate(this);
}

// ---------------------------------------------------------------------------------------
// Proportional maps
// ---------------------------------------------------------------------------------------

uint16_t SpeakerAudioCoordinator::EffectiveMaxVolume() const
{
    return std::min<uint16_t>(mHardware.GetMaxDeviceVolume(), mAudioControl.Cluster().GetMaxUserVolume());
}

uint8_t SpeakerAudioCoordinator::VolumeToLevel(uint16_t volume) const
{
    const uint16_t vMin = mHardware.GetMinDeviceVolume();
    const uint16_t vMax = EffectiveMaxVolume();
    const uint8_t lMin  = mLevelControl.Cluster().GetMinLevel();
    const uint8_t lMax  = mLevelControl.Cluster().GetMaxLevel();

    if (vMax <= vMin || lMax <= lMin)
    {
        return lMin;
    }

    volume            = std::clamp<uint16_t>(volume, vMin, vMax);
    const long mapped = lMin + std::lround(static_cast<double>(volume - vMin) * (lMax - lMin) / (vMax - vMin));
    return static_cast<uint8_t>(std::clamp<long>(mapped, lMin, lMax));
}

uint16_t SpeakerAudioCoordinator::LevelToVolume(uint8_t level) const
{
    const uint16_t vMin = mHardware.GetMinDeviceVolume();
    const uint16_t vMax = EffectiveMaxVolume();
    const uint8_t lMin  = mLevelControl.Cluster().GetMinLevel();
    const uint8_t lMax  = mLevelControl.Cluster().GetMaxLevel();

    if (vMax <= vMin || lMax <= lMin)
    {
        return vMin;
    }

    level             = std::clamp<uint8_t>(level, lMin, lMax);
    const long mapped = vMin + std::lround(static_cast<double>(level - lMin) * (vMax - vMin) / (lMax - lMin));
    return static_cast<uint16_t>(std::clamp<long>(mapped, vMin, vMax));
}

// ---------------------------------------------------------------------------------------
// Coordination
// ---------------------------------------------------------------------------------------

void SpeakerAudioCoordinator::SyncFromAudioControl(uint16_t volume, bool softMuted)
{
    ReentrancyGuard guard(mSyncing);

    if (softMuted != !mOnOff.Cluster().GetOnOff())
    {
        LogErrorOnFailure(mOnOff.Cluster().SetOnOff(!softMuted));
    }

    const DataModel::Nullable<uint8_t> currentLevel = mLevelControl.Cluster().GetCurrentLevel();
    const uint8_t target                            = VolumeToLevel(volume);
    if (!currentLevel.IsNull() && target != currentLevel.Value())
    {
        // transitionTime 0 -> immediate; ExecuteIfOff so the level tracks even while muted.
        auto status =
            mLevelControl.Cluster().MoveToLevel(target, DataModel::MakeNullable<uint16_t>(0), ExecuteIfOff(), ExecuteIfOff());
        if (!status.IsSuccess())
        {
            ChipLogError(Zcl, "TV Speaker: MoveToLevel(%u) failed while mirroring Volume", target);
        }
    }
}

Status SpeakerAudioCoordinator::HandleVolumeAndMuteChange(uint16_t newVolume, bool newSoftMuted)
{
    // Called by an Audio Control command before the cluster commits newVolume/newSoftMuted.
    SyncFromAudioControl(newVolume, newSoftMuted);

    // One canonical notification to hardware; its status gates the Audio Control commit.
    return mHardware.HandleVolumeAndMuteChange(newVolume, newSoftMuted);
}

void SpeakerAudioCoordinator::OnOnOffChanged(bool on)
{
    VerifyOrReturn(!mSyncing); // ignore our own echo from SyncFromAudioControl()

    ReentrancyGuard guard(mSyncing);
    // SetSoftMuted reports but does not call the Audio Control delegate, so cannot re-enter.
    // This method leaves CurrentLevel alone; Level Control (the other On/Off delegate) runs its
    // own choreography and, OnLevel being NULL, settles back at the pre-off level.
    LogErrorOnFailure(mAudioControl.Cluster().SetSoftMuted(!on));
}

void SpeakerAudioCoordinator::OnLevelChanged(uint8_t level)
{
    VerifyOrReturn(!mSyncing); // ignore our own echo from SyncFromAudioControl()

    ReentrancyGuard guard(mSyncing);
    const uint16_t volume = LevelToVolume(level);
    LogErrorOnFailure(mAudioControl.Cluster().SetVolume(volume));
}

void SpeakerAudioCoordinator::OnOffStartup(bool on)
{
    // On/Off startup is reconciled from Audio Control in OnStartup(); nothing to do here.
}

void SpeakerAudioCoordinator::OnStartup(const AudioControlDelegate::StartupState & state)
{
    mHardware.OnStartup(state);

    // Audio Control is authoritative at boot: bring OnOff and CurrentLevel into agreement.
    SyncFromAudioControl(state.volume, state.softMuted);
}

} // namespace chip::app::Clusters::Speaker
