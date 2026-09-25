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

#include <app/clusters/air-quality-server/AirQualityCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/BitFlags.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

/// Base Matter Air Quality Sensor device type (spec section 2.6).
/// Owns only mandatory clusters (Identify, Air Quality).
/// Optional clusters (Temperature, Relative Humidity, Concentration measurements)
/// belong to subclasses via RegisterAdditionalClusters / UnregisterAdditionalClusters.
class AirQualitySensor : public SingleEndpoint
{
public:
    struct Config
    {
        BitFlags<Clusters::AirQuality::Feature> airQualityFeatures{ Clusters::AirQuality::Feature::kFair,
                                                                    Clusters::AirQuality::Feature::kModerate,
                                                                    Clusters::AirQuality::Feature::kVeryPoor,
                                                                    Clusters::AirQuality::Feature::kExtremelyPoor };

        constexpr Config() = default;

        Config & WithAirQuality(BitFlags<Clusters::AirQuality::Feature> features)
        {
            airQualityFeatures = features;
            return *this;
        }
    };

    AirQualitySensor(TimerDelegate & timerDelegate, const Config & config);
    explicit AirQualitySensor(TimerDelegate & timerDelegate);
    ~AirQualitySensor() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public cluster accessors for mandatory clusters
    Clusters::AirQualityCluster & AirQualityCluster();
    Clusters::IdentifyCluster & IdentifyCluster();

protected:
    /// Called before the endpoint is added, within the registration transaction.
    /// Subclasses in impl/ override this to attach optional clusters.
    virtual CHIP_ERROR RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    /// Called after the endpoint is removed, also on partial registration failure.
    virtual void UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider) {}

    TimerDelegate & mTimerDelegate;
    Config mConfig;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::AirQualityCluster> mAirQualityCluster;
};

} // namespace app
} // namespace chip
