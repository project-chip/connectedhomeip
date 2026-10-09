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
        virtual ~WaterValveListener() = default;
        virtual void OnValveOpened()  = 0;
        virtual void OnValveClosed()  = 0;
    };

    WaterValve(TimerDelegate & timerDelegate);
    WaterValve(TimerDelegate & timerDelegate,
               const std::optional<Clusters::ValveConfigurationAndControlCluster::ValveContext> & context,
               WaterValveListener * listener = nullptr);
    ~WaterValve() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    CHIP_ERROR CloseValve();
    CHIP_ERROR OpenValve(DataModel::Nullable<Percent> level, DataModel::Nullable<uint32_t> duration);
    // Public getters for programmatic control
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }

    Clusters::ValveConfigurationAndControlCluster & ValveConfigurationAndControlCluster();

    Clusters::ValveConfigurationAndControlCluster::ValveContext SetUpValveContext()
    {
        if (!mValveContext.has_value())
        {
            return Clusters::ValveConfigurationAndControlCluster::ValveContext{
                .features = BitFlags<Clusters::ValveConfigurationAndControl::Feature>(
                    Clusters::ValveConfigurationAndControl::Feature::kLevel),
                .optionalAttributeSet = {},
                .config               = { DataModel::NullNullable, Clusters::ValveConfigurationAndControlCluster::kDefaultOpenLevel,
                                          Clusters::ValveConfigurationAndControlCluster::kDefaultLevelStep },
                .tsTracker            = nullptr,
                .delegate             = this,
            };
        }
        auto context     = *mValveContext;
        context.delegate = this;
        return context;
    }
    bool IsOpen() const { return mOpenLevel.has_value(); }
    std::optional<Percent> OpenLevel() const { return mOpenLevel; }
    std::optional<uint32_t> RemainingDuration() const { return mRemainingDuration; }

protected:
    // Clusters::ValveConfigurationAndControl::Delegate implementation
    DataModel::Nullable<Percent> HandleOpenValve(DataModel::Nullable<Percent> level) override;
    CHIP_ERROR HandleCloseValve() override;
    void HandleRemainingDurationTick(uint32_t duration) override;

    std::optional<Percent> mOpenLevel;
    std::optional<uint32_t> mRemainingDuration;
    TimerDelegate & mTimerDelegate;
    std::optional<Clusters::ValveConfigurationAndControlCluster::ValveContext> mValveContext;
    WaterValveListener * mListener = nullptr;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::ValveConfigurationAndControlCluster> mValveCluster;
};

} // namespace chip::app
