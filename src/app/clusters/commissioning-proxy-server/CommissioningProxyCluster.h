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

#pragma once

#include <app/clusters/commissioning-proxy-server/CommissioningProxyScanAggregator.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyScanCache.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxySessionManager.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyTransport.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/CommissioningProxy/AttributeIds.h>
#include <clusters/CommissioningProxy/ClusterId.h>
#include <clusters/CommissioningProxy/Enums.h>
#include <clusters/CommissioningProxy/Events.h>
#include <clusters/CommissioningProxy/Structs.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

class CommissioningProxyCluster : public DefaultServerCluster, public ScanCacheObserver
{
private:
    using OptionalAttributesSet =
        OptionalAttributeSet<CommissioningProxy::Attributes::MaxCachedResults::Id,
                             CommissioningProxy::Attributes::NumCachedResults::Id, CommissioningProxy::Attributes::CacheTimeout::Id,
                             CommissioningProxy::Attributes::CachedResults::Id, CommissioningProxy::Attributes::WiFiBand::Id>;

public:
    enum State
    {
        kState_CPDisconnected = 0,
        kState_CPConnected
    };

    // Static device capabilities. The app supplies these up front; transports are
    // registered separately (RegisterTransport) since they may be constructed after
    // the cluster. There is no application delegate: all transport actions go through
    // the registered CommissioningProxyTransport drivers, and all reportable/derived
    // attribute state is owned by the cluster and its subsystems.
    struct Config
    {
        BitMask<CommissioningProxy::Feature> featureFlags;
        uint8_t maxSessions;
        uint8_t maxCachedResults;
        BitMask<CommissioningProxy::WiFiBandBitmap> supportedWiFiBands;

        Config(BitMask<CommissioningProxy::Feature> aFeatures, uint8_t aMaxSessions = 1, uint8_t aMaxCachedResults = 10,
               BitMask<CommissioningProxy::WiFiBandBitmap> aSupportedWiFiBands = {}) :
            featureFlags(aFeatures),
            maxSessions(aMaxSessions), maxCachedResults(aMaxCachedResults), supportedWiFiBands(aSupportedWiFiBands)
        {}
    };

    CommissioningProxyCluster() = delete;

    // The endpoint id is supplied separately from Config: a device's cluster config is
    // typically fixed up front, but the endpoint is only known when the device is
    // registered and the clusters are created.
    CommissioningProxyCluster(EndpointId endpointId, const Config & config) :
        DefaultServerCluster({ endpointId, CommissioningProxy::Id }), mFeatureFlags(config.featureFlags),
        mMaxSessions(config.maxSessions), mMaxCachedResults(config.maxCachedResults),
        mSupportedWiFiBands(config.supportedWiFiBands), mEnabledOptionalAttributes([&]() {
            OptionalAttributesSet attrs;
            attrs.Set<CommissioningProxy::Attributes::MaxCachedResults::Id>(
                config.featureFlags.Has(CommissioningProxy::Feature::kBackgroundScan));
            attrs.Set<CommissioningProxy::Attributes::NumCachedResults::Id>(
                config.featureFlags.Has(CommissioningProxy::Feature::kBackgroundScan));
            attrs.Set<CommissioningProxy::Attributes::CacheTimeout::Id>(
                config.featureFlags.Has(CommissioningProxy::Feature::kBackgroundScan));
            attrs.Set<CommissioningProxy::Attributes::CachedResults::Id>(
                config.featureFlags.Has(CommissioningProxy::Feature::kBackgroundScan));
            attrs.Set<CommissioningProxy::Attributes::WiFiBand::Id>(
                config.featureFlags.Has(CommissioningProxy::Feature::kWiFiNetworkInterface));
            return attrs;
        }()),
        mScanCache(*this)
    {
        mMainCommissioningProxyState = kState_CPDisconnected;
    }

    ~CommissioningProxyCluster() override
    {
        // Safety net: idempotent if the application already called Shutdown().
        Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Cancel subsystem timers, detach transports, and clear IM context.  Must be
    // called (or destructor invoked) before any stack-allocated CommandHandler
    // that was passed to InvokeCommand() goes out of scope.
    void Shutdown(ClusterShutdownType type) override
    {
        mScanCache.Shutdown();
        mScanAggregator.Shutdown();
        mSessions.Shutdown();
        for (size_t i = 0; i < mTransportCount; i++)
        {
            mTransports[i]->Shutdown();
            mTransports[i]->SetHost(nullptr);
        }
        mTransportCount = 0;
        DefaultServerCluster::Shutdown(type);
    }

    /**
     * @brief Register a platform transport driver. The driver's host back-pointer
     * is set to this cluster. Call before Startup; a driver for a given transport
     * bit may be registered only once.
     */
    void RegisterTransport(CommissioningProxyTransport & transport)
    {
        VerifyOrDie(mTransportCount < kMaxTransports);
        VerifyOrDie(FindTransport(transport.GetTransportType()) == nullptr);
        mTransports[mTransportCount++] = &transport;
        transport.SetHost(this);
    }

    const OptionalAttributesSet & OptionalAttributes() const { return mEnabledOptionalAttributes; }
    const BitFlags<CommissioningProxy::Feature> & Features() const { return mFeatureFlags; }

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;
    CHIP_ERROR SetCPState(State state);
    CommissioningProxyCluster::State GetCPState();

    /**
     * @brief Current ScanMaxTime / CacheTimeout attribute values.
     *
     * These writable attributes are stored and change-reported by the cluster.
     * A transport performing a scan reads the live value through these getters
     * (via its host back-pointer) rather than caching its own copy.
     */
    uint8_t GetScanMaxTime() const { return mScanMaxTime; }
    uint16_t GetCacheTimeout() const override { return mCacheTimeout; }

    // Static device capabilities (from Config).
    uint8_t GetMaxSessions() const { return mMaxSessions; }
    uint8_t GetMaxCachedResults() const override { return mMaxCachedResults; }
    BitMask<CommissioningProxy::WiFiBandBitmap> GetSupportedWiFiBands() const { return mSupportedWiFiBands; }

    /// Update the supported Wi-Fi bands (e.g. once the radio's capabilities are
    /// known at init). Also the Config default.
    void SetSupportedWiFiBands(BitMask<CommissioningProxy::WiFiBandBitmap> bands) { mSupportedWiFiBands = bands; }

    /// Number of proxy sessions counted against MaxSessions: established sessions
    /// plus any in-flight connect on any registered transport.
    uint8_t GetActiveSessionCount() const
    {
        uint8_t count = mSessions.ActiveCount();
        for (size_t i = 0; i < mTransportCount; i++)
        {
            if (mTransports[i]->IsConnectPending())
                count++;
        }
        return count;
    }

    /// The set of transports supported by this instance: those with a registered
    /// driver. Mirrors the Transport attribute and drives command validation.
    BitMask<CapabilitiesBitmap> GetSupportedTransports() const;

    /// Find the registered driver for a single transport bit, or nullptr.
    CommissioningProxyTransport * FindTransport(CapabilitiesBitmap bit) const;

    /**
     * @brief Accessors for the cluster's transport-agnostic subsystems. Platform
     * transports call these (via their host back-pointer) to register/close
     * sessions, route ProxyMessage replies, cache background-scan results, and
     * contribute foreground-scan results.
     */
    CommissioningProxySessionManager & Sessions() { return mSessions; }
    CommissioningProxyScanCache & ScanCache() { return mScanCache; }
    CommissioningProxyScanAggregator & ScanAggregator() { return mScanAggregator; }

    /**
     * @brief Notify subscribers that CachedResults and NumCachedResults have changed.
     *
     * Called by the scan cache whenever the background-scan result set changes.
     */
    void MarkCachedResultsDirty() override
    {
        NotifyAttributeChanged(CommissioningProxy::Attributes::CachedResults::Id);
        NotifyAttributeChanged(CommissioningProxy::Attributes::NumCachedResults::Id);
    }

private:
    std::optional<DataModel::ActionReturnStatus>
    HandleProxyConnectRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments, CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus> HandleProxyDisconnectRequest(const DataModel::InvokeRequest & request,
                                                                              TLV::TLVReader & input_arguments,
                                                                              CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus> HandleProxyScanRequest(const DataModel::InvokeRequest & request,
                                                                        TLV::TLVReader & input_arguments, CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus> HandleProxyBackGroundScanStartRequest(const DataModel::InvokeRequest & request,
                                                                                       TLV::TLVReader & input_arguments,
                                                                                       CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus> HandleProxyBackGroundScanStopRequest(const DataModel::InvokeRequest & request,
                                                                                      TLV::TLVReader & input_arguments,
                                                                                      CommandHandler * handler);
    std::optional<DataModel::ActionReturnStatus>
    HandleProxyMessageRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments, CommandHandler * handler);

    /// Cancel any in-flight connect(s) owned by @p fabricIndex across all transports
    /// (null-SessionID ProxyDisconnectRequest).
    Protocols::InteractionModel::Status CancelPendingConnect(FabricIndex fabricIndex);

    const BitFlags<CommissioningProxy::Feature> mFeatureFlags;
    const uint8_t mMaxSessions;
    const uint8_t mMaxCachedResults;
    BitMask<CommissioningProxy::WiFiBandBitmap> mSupportedWiFiBands;
    const OptionalAttributesSet mEnabledOptionalAttributes;
    State mMainCommissioningProxyState;

    // Writable attributes owned (stored and change-reported) by the cluster.
    uint8_t mScanMaxTime   = 10;
    uint16_t mCacheTimeout = 120;

    // Transport-agnostic subsystems owned by the cluster.
    CommissioningProxySessionManager mSessions;
    CommissioningProxyScanCache mScanCache;
    CommissioningProxyScanAggregator mScanAggregator;

    // Registered platform transport drivers (BLE, Wi-Fi PAF, ...), at most one per
    // transport bit. Owned by the application, not the cluster.
    static constexpr size_t kMaxTransports                    = 3;
    CommissioningProxyTransport * mTransports[kMaxTransports] = {};
    size_t mTransportCount                                    = 0;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
