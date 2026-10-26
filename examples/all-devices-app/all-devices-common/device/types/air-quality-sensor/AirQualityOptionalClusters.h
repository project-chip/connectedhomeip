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

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace chip {
namespace app {

namespace AirQualitySensorInternal {

/**
 * @brief Compile-time predicate identifying Matter concentration measurement clusters.
 */
template <ClusterId CID>
inline constexpr bool IsConcentrationCluster =
    (CID == Clusters::CarbonDioxideConcentrationMeasurement::Id || CID == Clusters::Pm25ConcentrationMeasurement::Id ||
     CID == Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id ||
     CID == Clusters::CarbonMonoxideConcentrationMeasurement::Id || CID == Clusters::NitrogenDioxideConcentrationMeasurement::Id ||
     CID == Clusters::OzoneConcentrationMeasurement::Id || CID == Clusters::FormaldehydeConcentrationMeasurement::Id ||
     CID == Clusters::Pm1ConcentrationMeasurement::Id || CID == Clusters::Pm10ConcentrationMeasurement::Id ||
     CID == Clusters::RadonConcentrationMeasurement::Id);

/**
 * @brief Compile-time mapping from ClusterId to the concrete ServerCluster type.
 */
template <ClusterId CID, typename = void>
struct ClusterTypeTraits
{
    using Type = void;
};

template <ClusterId CID>
struct ClusterTypeTraits<CID, std::enable_if_t<IsConcentrationCluster<CID>>>
{
    using Type = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster;
};

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

/**
 * @brief Alias helper to obtain the concrete cluster type for a given ClusterId.
 */
template <ClusterId CID>
using ClusterType = typename ClusterTypeTraits<CID>::Type;

/**
 * @brief Provides standard Matter spec concentration units, min, and max defaults
 *        for standard gas measurement clusters. Implemented in AirQualitySensor.cpp.
 */
Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster::Config DefaultConcentrationConfig(ClusterId clusterId);

/**
 * @brief Compile-time traits mapping ClusterId to its configuration type and default initializer.
 */
template <ClusterId CID, typename = void>
struct ClusterConfigTraits;

template <ClusterId CID>
struct ClusterConfigTraits<CID, std::enable_if_t<IsConcentrationCluster<CID>>>
{
    using Type = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster::Config;
    static Type Default() { return DefaultConcentrationConfig(CID); }

    template <typename ClusterWrapper>
    static void CreateCluster(ClusterWrapper & wrapper, EndpointId endpoint, const Type & config)
    {
        Type resolvedConfig      = config;
        resolvedConfig.clusterId = CID;
        wrapper.Create(endpoint, resolvedConfig);
    }
};

template <>
struct ClusterConfigTraits<Clusters::TemperatureMeasurement::Id>
{
    using Type = Clusters::TemperatureMeasurementCluster::StartupConfiguration;

    /**
     * @brief Spec default: -40.00°C (min) to +80.00°C (max) in 0.01°C steps.
     */
    static Type Default()
    {
        Type config;
        config.minMeasuredValue = DataModel::MakeNullable(static_cast<int16_t>(-4000));
        config.maxMeasuredValue = DataModel::MakeNullable(static_cast<int16_t>(8000));
        return config;
    }

    template <typename ClusterWrapper>
    static void CreateCluster(ClusterWrapper & wrapper, EndpointId endpoint, const Type & config)
    {
        wrapper.Create(endpoint, Clusters::TemperatureMeasurementCluster::OptionalAttributeSet(), config);
    }
};

template <>
struct ClusterConfigTraits<Clusters::RelativeHumidityMeasurement::Id>
{
    using Type = Clusters::RelativeHumidityMeasurementCluster::Config;

    /**
     * @brief Spec default: 0.00% (min) to 100.00% (max) in 0.01% steps.
     */
    static Type Default()
    {
        Type config;
        config.minMeasuredValue = DataModel::MakeNullable(static_cast<uint16_t>(0));
        config.maxMeasuredValue = DataModel::MakeNullable(static_cast<uint16_t>(10000));
        return config;
    }

    template <typename ClusterWrapper>
    static void CreateCluster(ClusterWrapper & wrapper, EndpointId endpoint, const Type & config)
    {
        wrapper.Create(endpoint, config);
    }
};

/**
 * @brief Helper alias to extract the config type for a given ClusterId.
 */
template <ClusterId CID>
using ClusterConfigType = typename ClusterConfigTraits<CID>::Type;

/**
 * @brief Helper function to retrieve the spec default configuration for a given ClusterId.
 */
template <ClusterId CID>
ClusterConfigType<CID> DefaultClusterConfig()
{
    return ClusterConfigTraits<CID>::Default();
}

/**
 * @brief Finds the 0-based index of `Target` in parameter pack `List...`.
 *        Returns `static_cast<size_t>(-1)` if not found.
 */
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

/**
 * @brief Counts the occurrences of `Target` in parameter pack `List...`.
 */
template <ClusterId Target, ClusterId... List>
constexpr size_t CountOf()
{
    return ((List == Target ? 1 : 0) + ... + 0);
}

} // namespace AirQualitySensorInternal

} // namespace app
} // namespace chip
