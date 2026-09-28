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
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>
#include <device/types/air-quality-sensor/AirQualityOptionalClusters.h>
#include <devices/Types.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Compiler.h>
#include <lib/support/TimerDelegate.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <utility>

namespace chip {
namespace app {

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
 * Supported optional clusters (defined in AirQualityOptionalClusters.h):
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

        // 3. Register all statically declared optional clusters (Temperature, Relative Humidity, Concentrations)
        CHIP_ERROR err = CHIP_NO_ERROR;

        // Helper lambda to instantiate and register an individual optional cluster from mOptionalClusters.
        // Takes a compile-time tag std::integral_constant<ClusterId, CID> to extract constexpr ClusterId.
        auto registerCluster = [&](auto & clusterWrapper, auto clusterIdTag) {
            using TagType                 = decltype(clusterIdTag);
            constexpr ClusterId clusterId = TagType::value;
            if (err != CHIP_NO_ERROR)
            {
                return;
            }

            // Retrieve the cluster's configuration from mConfig.clusterConfigs tuple
            constexpr size_t kIdx = Detail::IndexOf<clusterId, OptionalClusters...>();
            auto & clusterConfig  = std::get<kIdx>(mConfig.clusterConfigs);

            // Instantiate cluster in-place with its specific config signature (handled by ClusterConfigTraits)
            Detail::ClusterConfigTraits<clusterId>::CreateCluster(clusterWrapper, endpoint, clusterConfig);

            // Register cluster with data model provider
            err = provider.AddCluster(clusterWrapper.Registration());
        };

        if constexpr (sizeof...(OptionalClusters) > 0)
        {
            // Expand fold expression across all OptionalClusters to instantiate each declared cluster
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
    auto * GetCluster()
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
            return static_cast<Detail::ClusterType<CID> *>(nullptr);
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
