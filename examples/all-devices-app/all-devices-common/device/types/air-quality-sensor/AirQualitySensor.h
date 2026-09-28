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

/**
 * @brief Compile-time mapping from ClusterId to the concrete ServerCluster type.
 *
 * Specializations define `using Type = ...` mapping Matter cluster IDs to their
 * corresponding code-driven cluster implementation classes.
 */
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

/**
 * @brief Matter Air Quality Sensor device type (Matter Device Library specification section 2.6).
 *
 * Implements a code-driven Air Quality Sensor single-endpoint device.
 *
 * Required clusters:
 *   - Identify (0x0003)
 *   - Air Quality (0x005B)
 *
 * Optional clusters can be statically declared via the template parameter pack:
 *   `AirQualitySensor<OptionalClusters...>`
 *
 * Supported optional clusters:
 *   - TemperatureMeasurement (0x0402)
 *   - RelativeHumidityMeasurement (0x0405)
 *   - Concentration measurement clusters:
 *       - CarbonDioxideConcentrationMeasurement (0x040D)
 *       - Pm25ConcentrationMeasurement (0x042A)
 *       - TotalVolatileOrganicCompoundsConcentrationMeasurement (0x042E)
 *       - CarbonMonoxideConcentrationMeasurement (0x040C)
 *       - NitrogenDioxideConcentrationMeasurement (0x0413)
 *       - OzoneConcentrationMeasurement (0x0415)
 *       - FormaldehydeConcentrationMeasurement (0x042B)
 *       - Pm1ConcentrationMeasurement (0x042C)
 *       - Pm10ConcentrationMeasurement (0x042D)
 *       - RadonConcentrationMeasurement (0x042F)
 *
 * Storage for optional clusters is exact and allocates zero unused memory.
 *
 * Example:
 * @code
 *   using MySensor = AirQualitySensor<
 *       Clusters::TemperatureMeasurement::Id,
 *       Clusters::RelativeHumidityMeasurement::Id,
 *       Clusters::CarbonDioxideConcentrationMeasurement::Id
 *   >;
 *
 *   MySensor::Config config;
 *   config.Get<Clusters::TemperatureMeasurement::Id>().minMeasuredValue = DataModel::MakeNullable(static_cast<int16_t>(-1000));
 *   config.Get<Clusters::CarbonDioxideConcentrationMeasurement::Id>().minMeasured = DataModel::MakeNullable(400.0f);
 *
 *   MySensor sensor(timerDelegate, config);
 *   sensor.Register(endpointId, provider);
 * @endcode
 *
 * @tparam OptionalClusters Cluster IDs of the optional clusters to instantiate on this sensor.
 */
template <ClusterId... OptionalClusters>
class AirQualitySensor : public SingleEndpoint
{
    static_assert(((Detail::CountOf<OptionalClusters, OptionalClusters...>() == 1) && ...),
                  "Optional cluster IDs must not be duplicated");

public:
    /**
     * @brief Compile-time query to check if a specific cluster is supported by this sensor instance.
     */
    template <ClusterId CID>
    static constexpr bool HasCluster =
        ((OptionalClusters == CID) || ... || false) || (CID == Clusters::Identify::Id) || (CID == Clusters::AirQuality::Id);

    /**
     * @brief Configuration for the AirQualitySensor and its optional clusters.
     *
     * Holds the feature map for the mandatory Air Quality cluster, as well as
     * concrete configurations for each enabled optional cluster initialized with
     * spec-compliant defaults.
     */
    struct Config
    {
        /// Feature flags for the Air Quality cluster (defaults: Fair, Moderate, VeryPoor, ExtremelyPoor).
        BitFlags<Clusters::AirQuality::Feature> airQualityFeatures{ Clusters::AirQuality::Feature::kFair,
                                                                    Clusters::AirQuality::Feature::kModerate,
                                                                    Clusters::AirQuality::Feature::kVeryPoor,
                                                                    Clusters::AirQuality::Feature::kExtremelyPoor };

        /// Exact storage for each configured optional cluster's configuration struct.
        std::tuple<Detail::ClusterConfigType<OptionalClusters>...> clusterConfigs{
            Detail::DefaultClusterConfig<OptionalClusters>()...
        };

        Config() = default;

        /**
         * @brief Access the mutable configuration for a specific optional cluster.
         *
         * @tparam CID The ClusterId to access. Must be one of `OptionalClusters...`.
         * @return Mutable reference to the cluster's configuration struct.
         */
        template <ClusterId CID>
        Detail::ClusterConfigType<CID> & Get()
        {
            static_assert(((OptionalClusters == CID) || ...), "Cluster not configured on this sensor");
            constexpr size_t kIdx = Detail::IndexOf<CID, OptionalClusters...>();
            return std::get<kIdx>(clusterConfigs);
        }

        /**
         * @brief Access the read-only configuration for a specific optional cluster.
         *
         * @tparam CID The ClusterId to access. Must be one of `OptionalClusters...`.
         * @return Const reference to the cluster's configuration struct.
         */
        template <ClusterId CID>
        const Detail::ClusterConfigType<CID> & Get() const
        {
            static_assert(((OptionalClusters == CID) || ...), "Cluster not configured on this sensor");
            constexpr size_t kIdx = Detail::IndexOf<CID, OptionalClusters...>();
            return std::get<kIdx>(clusterConfigs);
        }
    };

    /**
     * @brief Constructs an AirQualitySensor device.
     *
     * @param timerDelegate Reference to platform TimerDelegate (used by IdentifyCluster and simulation).
     * @param config Device and optional cluster configuration.
     */
    explicit AirQualitySensor(TimerDelegate & timerDelegate, const Config & config = {}) :
        SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kAirQualitySensor, 1)), mTimerDelegate(timerDelegate),
        mConfig(config)
    {}

    ~AirQualitySensor() override = default;

    /**
     * @brief Registers the Air Quality Sensor endpoint and all configured clusters with the data model provider.
     *
     * Registers:
     *   1. Endpoint descriptor
     *   2. Identify cluster (mandatory)
     *   3. Air Quality cluster (mandatory)
     *   4. Statically declared optional clusters in `OptionalClusters...`
     *   5. Subclass additional clusters via `RegisterAdditionalClusters()`
     *
     * @param endpoint Endpoint ID to bind to.
     * @param provider The CodeDrivenDataModelProvider to register clusters and endpoint with.
     * @param composition Composition hierarchy metadata.
     * @return CHIP_NO_ERROR on success, or an error code on failure.
     */
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

        CHIP_ERROR err       = CHIP_NO_ERROR;
        auto registerCluster = [&](auto & clusterWrapper, auto clusterIdTag) {
            using TagType                 = decltype(clusterIdTag);
            constexpr ClusterId clusterId = TagType::value;
            if (err != CHIP_NO_ERROR)
            {
                return;
            }
            constexpr size_t kIdx = Detail::IndexOf<clusterId, OptionalClusters...>();
            auto & clusterConfig  = std::get<kIdx>(mConfig.clusterConfigs);

            if constexpr (clusterId == Clusters::TemperatureMeasurement::Id)
            {
                clusterWrapper.Create(endpoint, Clusters::TemperatureMeasurementCluster::OptionalAttributeSet(), clusterConfig);
            }
            else
            {
                clusterWrapper.Create(endpoint, clusterConfig);
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

    /**
     * @brief Unregisters the endpoint and cleanly tears down all constructed clusters.
     *
     * Removes and destroys only the clusters that were constructed, ensuring safe cleanup.
     *
     * @param provider The CodeDrivenDataModelProvider to remove clusters and endpoint from.
     */
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

    /**
     * @brief Unified generic cluster accessor.
     *
     * Returns a pointer to the concrete cluster instance if the cluster is supported
     * and configured on this sensor, or `nullptr` otherwise.
     *
     * Example:
     * @code
     *   if (auto * co2 = sensor.GetCluster<Clusters::CarbonDioxideConcentrationMeasurement::Id>())
     *   {
     *       co2->SetMeasuredValue(DataModel::MakeNullable(450.0f));
     *   }
     * @endcode
     *
     * @tparam CID The ClusterId to retrieve.
     * @return Pointer to concrete cluster instance, or nullptr if not configured.
     */
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

    /// Convenience accessor for the mandatory Air Quality cluster.
    Clusters::AirQualityCluster & AirQualityCluster() { return *GetCluster<Clusters::AirQuality::Id>(); }

    /// Convenience accessor for the mandatory Identify cluster.
    Clusters::IdentifyCluster & IdentifyCluster() { return *GetCluster<Clusters::Identify::Id>(); }

protected:
    /**
     * @brief Extension hook for subclasses to register additional clusters.
     */
    virtual CHIP_ERROR RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Extension hook for subclasses to unregister additional clusters.
     */
    virtual void UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider) {}

    TimerDelegate & mTimerDelegate;
    Config mConfig;

    /// Mandatory clusters
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::AirQualityCluster> mAirQualityCluster;

    /// Statically sized tuple holding only declared optional clusters (zero overhead for unconfigured clusters)
    std::tuple<LazyRegisteredServerCluster<Detail::ClusterType<OptionalClusters>>...> mOptionalClusters;
};

} // namespace app
} // namespace chip
