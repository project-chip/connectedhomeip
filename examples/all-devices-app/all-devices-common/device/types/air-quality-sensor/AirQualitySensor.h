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

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/air-quality-server/AirQualityCluster.h>
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/relative-humidity-measurement-server/RelativeHumidityMeasurementCluster.h>
#include <app/clusters/temperature-measurement-server/TemperatureMeasurementCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>
#include <devices/Types.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Compiler.h>
#include <lib/support/TimerDelegate.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>

namespace chip {
namespace app {

namespace Detail {

template <ClusterId CID>
struct ClusterTypeTraits;

template <>
struct ClusterTypeTraits<Clusters::Identify::Id>
{
    using Type = Clusters::IdentifyCluster;
};

template <>
struct ClusterTypeTraits<Clusters::AirQuality::Id>
{
    using Type = Clusters::AirQualityCluster;
};

template <>
struct ClusterTypeTraits<Clusters::TemperatureMeasurement::Id>
{
    using Type = Clusters::TemperatureMeasurementCluster;
};

template <>
struct ClusterTypeTraits<Clusters::RelativeHumidityMeasurement::Id>
{
    using Type = Clusters::RelativeHumidityMeasurementCluster;
};

template <>
struct ClusterTypeTraits<Clusters::CarbonDioxideConcentrationMeasurement::Id>
{
    using Type = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;
};

template <>
struct ClusterTypeTraits<Clusters::Pm25ConcentrationMeasurement::Id>
{
    using Type = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;
};

template <>
struct ClusterTypeTraits<Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id>
{
    using Type = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;
};

template <>
struct ClusterTypeTraits<Clusters::CarbonMonoxideConcentrationMeasurement::Id>
{
    using Type = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;
};

template <>
struct ClusterTypeTraits<Clusters::NitrogenDioxideConcentrationMeasurement::Id>
{
    using Type = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;
};

template <>
struct ClusterTypeTraits<Clusters::OzoneConcentrationMeasurement::Id>
{
    using Type = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;
};

template <>
struct ClusterTypeTraits<Clusters::FormaldehydeConcentrationMeasurement::Id>
{
    using Type = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;
};

template <>
struct ClusterTypeTraits<Clusters::Pm1ConcentrationMeasurement::Id>
{
    using Type = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;
};

template <>
struct ClusterTypeTraits<Clusters::Pm10ConcentrationMeasurement::Id>
{
    using Type = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;
};

template <>
struct ClusterTypeTraits<Clusters::RadonConcentrationMeasurement::Id>
{
    using Type = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;
};

template <ClusterId CID>
using ClusterType = typename ClusterTypeTraits<CID>::Type;

template <ClusterId Target, ClusterId... List>
constexpr size_t IndexOf()
{
    if constexpr (sizeof...(List) == 0)
    {
        return static_cast<size_t>(-1);
    }
    else
    {
        constexpr ClusterId arr[] = { List... };
        for (size_t i = 0; i < sizeof...(List); ++i)
        {
            if (arr[i] == Target)
            {
                return i;
            }
        }
        return static_cast<size_t>(-1);
    }
}

template <ClusterId Target, ClusterId... List>
constexpr size_t CountOf()
{
    if constexpr (sizeof...(List) == 0)
    {
        return 0;
    }
    else
    {
        constexpr ClusterId arr[] = { List... };
        size_t count              = 0;
        for (size_t i = 0; i < sizeof...(List); ++i)
        {
            if (arr[i] == Target)
            {
                count++;
            }
        }
        return count;
    }
}

Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster::Config DefaultConcentrationConfig(ClusterId clusterId);

} // namespace Detail

/// Matter Air Quality Sensor device type (spec section 2.6).
/// Owns mandatory clusters (Identify, Air Quality) and statically declared optional clusters.
/// Storage for optional clusters is exact and allocates zero unused memory.
template <ClusterId... OptionalClusters>
class AirQualitySensor : public SingleEndpoint
{
    static_assert(((Detail::CountOf<OptionalClusters, OptionalClusters...>() == 1) && ...),
                  "Optional cluster IDs must not be duplicated");

public:
    template <ClusterId CID>
    static constexpr bool HasCluster =
        ((OptionalClusters == CID) || ... || false) || (CID == Clusters::Identify::Id) || (CID == Clusters::AirQuality::Id);

    struct Config
    {
        BitFlags<Clusters::AirQuality::Feature> airQualityFeatures{ Clusters::AirQuality::Feature::kFair,
                                                                    Clusters::AirQuality::Feature::kModerate,
                                                                    Clusters::AirQuality::Feature::kVeryPoor,
                                                                    Clusters::AirQuality::Feature::kExtremelyPoor };
        Clusters::TemperatureMeasurementCluster::StartupConfiguration temperature;
        Clusters::RelativeHumidityMeasurementCluster::Config humidity;

        Config()
        {
            temperature.minMeasuredValue = DataModel::MakeNullable(static_cast<int16_t>(-4000));
            temperature.maxMeasuredValue = DataModel::MakeNullable(static_cast<int16_t>(8000));
            humidity.minMeasuredValue    = DataModel::MakeNullable(static_cast<uint16_t>(0));
            humidity.maxMeasuredValue    = DataModel::MakeNullable(static_cast<uint16_t>(10000));
        }
    };

    explicit AirQualitySensor(TimerDelegate & timerDelegate, const Config & config = {}) :
        SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kAirQualitySensor, 1)), mTimerDelegate(timerDelegate),
        mConfig(config)
    {}

    ~AirQualitySensor() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override
    {
        VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
        DeviceRegistrationTransaction transaction(*this, provider);

        ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

        mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mTimerDelegate));
        ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

        mAirQualityCluster.Create(endpoint, mConfig.airQualityFeatures);
        ReturnErrorOnFailure(provider.AddCluster(mAirQualityCluster.Registration()));

        CHIP_ERROR err = CHIP_NO_ERROR;
        auto registerCluster = [&](auto & clusterWrapper, auto clusterIdTag) {
            using TagType = decltype(clusterIdTag);
            constexpr ClusterId clusterId = TagType::value;
            if (err != CHIP_NO_ERROR)
            {
                return;
            }
            if constexpr (clusterId == Clusters::TemperatureMeasurement::Id)
            {
                clusterWrapper.Create(endpoint, Clusters::TemperatureMeasurementCluster::OptionalAttributeSet(),
                                      mConfig.temperature);
            }
            else if constexpr (clusterId == Clusters::RelativeHumidityMeasurement::Id)
            {
                clusterWrapper.Create(endpoint, mConfig.humidity);
            }
            else
            {
                clusterWrapper.Create(endpoint, Detail::DefaultConcentrationConfig(clusterId));
            }
            err = provider.AddCluster(clusterWrapper.Registration());
        };

        if constexpr (sizeof...(OptionalClusters) > 0)
        {
            (registerCluster(std::get<Detail::IndexOf<OptionalClusters, OptionalClusters...>()>(mOptionalClusters),
                             std::integral_constant<ClusterId, OptionalClusters>{}),
             ...);
            ReturnErrorOnFailure(err);
        }

        ReturnErrorOnFailure(RegisterAdditionalClusters(endpoint, provider));

        ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
        transaction.Commit();
        return CHIP_NO_ERROR;
    }

    void Unregister(CodeDrivenDataModelProvider & provider) override
    {
        UnregisterAdditionalClusters(provider);

        auto unregisterCluster = [&](auto & clusterWrapper) {
            if (clusterWrapper.IsConstructed())
            {
                LogErrorOnFailure(provider.RemoveCluster(&clusterWrapper.Cluster()));
                clusterWrapper.Destroy();
            }
        };

        std::apply([&](auto &... clusters) { (unregisterCluster(clusters), ...); }, mOptionalClusters);

        if (mAirQualityCluster.IsConstructed())
        {
            LogErrorOnFailure(provider.RemoveCluster(&mAirQualityCluster.Cluster()));
            mAirQualityCluster.Destroy();
        }
        if (mIdentifyCluster.IsConstructed())
        {
            LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
            mIdentifyCluster.Destroy();
        }

        UnregisterDescriptor(provider);
    }

    // Unified generic cluster accessor. Returns concrete pointer or nullptr if not configured.
    template <ClusterId CID>
    Detail::ClusterType<CID> * GetCluster()
    {
        if constexpr (CID == Clusters::Identify::Id)
        {
            return &mIdentifyCluster.Cluster();
        }
        else if constexpr (CID == Clusters::AirQuality::Id)
        {
            return &mAirQualityCluster.Cluster();
        }
        else if constexpr (((OptionalClusters == CID) || ... || false))
        {
            constexpr size_t kIdx = Detail::IndexOf<CID, OptionalClusters...>();
            return &std::get<kIdx>(mOptionalClusters).Cluster();
        }
        else
        {
            return nullptr;
        }
    }

    // Convenience accessors for mandatory clusters
    Clusters::AirQualityCluster & AirQualityCluster() { return *GetCluster<Clusters::AirQuality::Id>(); }
    Clusters::IdentifyCluster & IdentifyCluster() { return *GetCluster<Clusters::Identify::Id>(); }

protected:
    virtual CHIP_ERROR RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    virtual void UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider) {}

    TimerDelegate & mTimerDelegate;
    Config mConfig;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::AirQualityCluster> mAirQualityCluster;

    std::tuple<LazyRegisteredServerCluster<Detail::ClusterType<OptionalClusters>>...> mOptionalClusters;
};

} // namespace app
} // namespace chip
