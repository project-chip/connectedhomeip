/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/valve-configuration-and-control-server/ValveConfigurationAndControlCluster.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-delegate.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>
#include <optional>

namespace chip::app {

class WaterValve : public SingleEndpoint, public Clusters::ValveConfigurationAndControl::Delegate
{
public:
    class WaterValveListener
    {
    public:
        virtual ~WaterValveListener()      = default;
        virtual void OnValveStateChanged() = 0;
    };

    WaterValve(TimerDelegate & timerDelegate);
    WaterValve(TimerDelegate & timerDelegate,
               const DataModel::Nullable<Clusters::ValveConfigurationAndControlCluster::StartupConfiguration> & config,
               const DataModel::Nullable<Clusters::ValveConfigurationAndControlCluster::ValveContext> & context,
               WaterValveListener * listener = nullptr);
    ~WaterValve() override = default;

    /// Closes the valve through the cluster, so the valve attributes are updated as well.
    CHIP_ERROR CloseValve();
    /// Remembers the current open level and remaining duration, then closes the valve.
    CHIP_ERROR Pause();
    /// Reopens a paused valve with the remembered level and remaining duration.
    CHIP_ERROR Resume();
    /// Forgets the remembered pause data without reopening the valve.
    void ClearPause()
    {
        mPausedLevel.reset();
        mPausedDuration.reset();
    }
    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public getters for programmatic control
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }

    Clusters::ValveConfigurationAndControlCluster & ValveConfigurationAndControlCluster();

    // Clusters::ValveConfigurationAndControl::Delegate implementation
    DataModel::Nullable<Percent> HandleOpenValve(DataModel::Nullable<Percent> level) override;
    CHIP_ERROR HandleCloseValve() override;
    void HandleRemainingDurationTick(uint32_t duration) override;

    DataModel::Nullable<Clusters::ValveConfigurationAndControlCluster::ValveContext> SetUpValveContext()
    {
        if (mValveContext.IsNull())
        {
            return Clusters::ValveConfigurationAndControlCluster::ValveContext{
                .features = BitFlags<Clusters::ValveConfigurationAndControl::Feature>(
                    Clusters::ValveConfigurationAndControl::Feature::kLevel),
                .optionalAttributeSet = {},
                .config               = SetUpStartUpConfiguration().Value(),
                .tsTracker            = nullptr,
                .delegate             = this,
            };
        }
        return Clusters::ValveConfigurationAndControlCluster::ValveContext{

            .features             = mValveContext.Value().features,
            .optionalAttributeSet = mValveContext.Value().optionalAttributeSet,
            .config               = mValveContext.Value().config,
            .tsTracker            = mValveContext.Value().tsTracker,
            .delegate             = this,
        };
    }
    DataModel::Nullable<Clusters::ValveConfigurationAndControlCluster::StartupConfiguration> SetUpStartUpConfiguration()
    {
        if (mStartupConfiguration.IsNull())
        {
            return Clusters::ValveConfigurationAndControlCluster::StartupConfiguration{
                DataModel::NullNullable, Clusters::ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                Clusters::ValveConfigurationAndControlCluster::kDefaultLevelStep
            };
        }

        return Clusters::ValveConfigurationAndControlCluster::StartupConfiguration{

            .defaultOpenDuration = mStartupConfiguration.Value().defaultOpenDuration,
            .defaultOpenLevel    = mStartupConfiguration.Value().defaultOpenLevel,
            .levelStep           = mStartupConfiguration.Value().levelStep,
        };
    }
    bool IsOpen() const { return mOpenLevel.has_value(); }
    bool IsPaused() const { return mPausedLevel.has_value(); }

protected:
    std::optional<Percent> mOpenLevel;
    std::optional<uint32_t> mRemainingDuration;
    std::optional<Percent> mPausedLevel;
    std::optional<uint32_t> mPausedDuration;
    TimerDelegate & mTimerDelegate;
    DataModel::Nullable<Clusters::ValveConfigurationAndControlCluster::StartupConfiguration> mStartupConfiguration;
    DataModel::Nullable<Clusters::ValveConfigurationAndControlCluster::ValveContext> mValveContext;
    WaterValveListener * mListener = nullptr;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::ValveConfigurationAndControlCluster> mValveCluster;
};

} // namespace chip::app
