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
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/clusters/relative-humidity-measurement-server/RelativeHumidityMeasurementCluster.h>
#include <app/clusters/temperature-measurement-server/TemperatureMeasurementCluster.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace chip {
namespace app {

namespace Detail {

/**
 * @brief Compile-time mapping from ClusterId to the concrete ServerCluster type.
 *
 * Specializations define `using Type = ...` mapping Matter cluster IDs to their
 * corresponding code-driven cluster implementation classes.
 */
template <ClusterId CID>
struct ClusterTypeTraits;

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

/**
 * @brief Alias helper to obtain the concrete cluster type for a given ClusterId.
 */
template <ClusterId CID>
using ClusterType = typename ClusterTypeTraits<CID>::Type;

/**
 * @brief Compile-time traits mapping ClusterId to its configuration type and default initializer.
 *
 * Each supported cluster specializes this struct with:
 *   - `Type`: The concrete configuration struct type passed to the cluster's `.Create()` method.
 *   - `Default()`: Static method returning a spec-compliant default configuration.
 */
template <ClusterId CID>
struct ClusterConfigTraits;

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
 * @brief Provides standard Matter spec concentration units, min, and max defaults
 *        for standard gas measurement clusters. Implemented in AirQualitySensor.cpp.
 */
Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster::Config DefaultConcentrationConfig(ClusterId clusterId);

/**
 * @brief Base traits helper for concentration measurement clusters.
 */
template <ClusterId CID>
struct ConcentrationConfigTraits
{
    using Type = Clusters::ConcentrationMeasurement::ConcentrationMeasurementCluster::Config;
    static Type Default() { return DefaultConcentrationConfig(CID); }

    template <typename ClusterWrapper>
    static void CreateCluster(ClusterWrapper & wrapper, EndpointId endpoint, const Type & config)
    {
        wrapper.Create(endpoint, config);
    }
};

template <>
struct ClusterConfigTraits<Clusters::CarbonDioxideConcentrationMeasurement::Id>
    : ConcentrationConfigTraits<Clusters::CarbonDioxideConcentrationMeasurement::Id>
{
};
template <>
struct ClusterConfigTraits<Clusters::Pm25ConcentrationMeasurement::Id>
    : ConcentrationConfigTraits<Clusters::Pm25ConcentrationMeasurement::Id>
{
};
template <>
struct ClusterConfigTraits<Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id>
    : ConcentrationConfigTraits<Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id>
{
};
template <>
struct ClusterConfigTraits<Clusters::CarbonMonoxideConcentrationMeasurement::Id>
    : ConcentrationConfigTraits<Clusters::CarbonMonoxideConcentrationMeasurement::Id>
{
};
template <>
struct ClusterConfigTraits<Clusters::NitrogenDioxideConcentrationMeasurement::Id>
    : ConcentrationConfigTraits<Clusters::NitrogenDioxideConcentrationMeasurement::Id>
{
};
template <>
struct ClusterConfigTraits<Clusters::OzoneConcentrationMeasurement::Id>
    : ConcentrationConfigTraits<Clusters::OzoneConcentrationMeasurement::Id>
{
};
template <>
struct ClusterConfigTraits<Clusters::FormaldehydeConcentrationMeasurement::Id>
    : ConcentrationConfigTraits<Clusters::FormaldehydeConcentrationMeasurement::Id>
{
};
template <>
struct ClusterConfigTraits<Clusters::Pm1ConcentrationMeasurement::Id>
    : ConcentrationConfigTraits<Clusters::Pm1ConcentrationMeasurement::Id>
{
};
template <>
struct ClusterConfigTraits<Clusters::Pm10ConcentrationMeasurement::Id>
    : ConcentrationConfigTraits<Clusters::Pm10ConcentrationMeasurement::Id>
{
};
template <>
struct ClusterConfigTraits<Clusters::RadonConcentrationMeasurement::Id>
    : ConcentrationConfigTraits<Clusters::RadonConcentrationMeasurement::Id>
{
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

} // namespace Detail

} // namespace app
} // namespace chip
