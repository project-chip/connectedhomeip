/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/commissioning-proxy-server/CommissioningProxyDelegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

class CommissioningProxyMockDelegate : public Delegate
{
public:
#if 0
    static constexpr int64_t kAbsMinPower = 0;
    static constexpr int64_t kAbsMaxPower = 5000;
#endif
    CommissioningProxyMockDelegate();
    ~CommissioningProxyMockDelegate() override; // = default;

    /**
     * @brief Handle PowerAdjustRequest command
     *
     * The cluster verifies that PowerAdjustmentCapability.cause is updated to match the command's cause.
     */

    Protocols::InteractionModel::Status ProxyConnectRequest(DataModel::Nullable<chip::ByteSpan> address,
                        chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport,
                        uint16_t discriminator,
                        chip::VendorId vendorid,
                        uint16_t productid,
                        uint16_t timeout,
                        chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBand,
                        app::CommandHandler * commandObj,
                        const DataModel::InvokeRequest & request) override;

    /**
     * @brief Handle PowerAdjustRequest command
     *
     * The cluster verifies that PowerAdjustmentCapability.cause is updated to match the command's cause.
     */
    Protocols::InteractionModel::Status ProxyScanRequest(
        CapabilitiesBitmap transport, WiFiBandBitmap wiFiBands, app::CommandHandler * commandObj,
        const DataModel::InvokeRequest & request) override;

    Protocols::InteractionModel::Status ProxyMessageRequest(
        uint16_t sessionId, chip::Optional<chip::ByteSpan> message, uint8_t responseTimeout,
        app::CommandHandler * commandObj, const DataModel::InvokeRequest & request) override;

    Protocols::InteractionModel::Status ProxyDisconnectRequest(uint16_t sessionId) override;

    Protocols::InteractionModel::Status ProxyBackgroundScanStartRequest(
        chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport,
        uint16_t timeout,
        chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBands,
        chip::FabricIndex fabricIndex,
        chip::NodeId nodeId,
        app::CommandHandler * commandObj,
        const DataModel::InvokeRequest & request) override;

    Protocols::InteractionModel::Status ProxyBackgroundScanStopRequest(
        chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport,
        chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBands,
        chip::FabricIndex fabricIndex,
        chip::NodeId nodeId) override;

    uint8_t GetMaxCachedResults() override { return 10; }
    uint8_t GetNumCachedResults() override { return 0; }
    uint16_t GetCacheTimeout() override { return 120; }
    void SetCacheTimeout(uint16_t) override {}
    CHIP_ERROR EncodeCachedResults(app::AttributeValueEncoder &) override { return CHIP_NO_ERROR; }

    void SetServer(CommissioningProxyCluster * server) override { mServer = server; }
    CommissioningProxyCluster * GetServer() const override { return mServer; }

    uint8_t GetScanMaxTime() override;
    void SetScanMaxTime(uint8_t seconds) override;
    chip::BitMask<WiFiBandBitmap> GetSupportedWiFiBands() override { return chip::BitMask<WiFiBandBitmap>(); }
#if 0
    /**
     * @brief Handle PowerAdjustRequest command
     *
     * The cluster verifies that PowerAdjustmentCapability.cause is updated to match the command's cause.
     */
    Protocols::InteractionModel::Status PowerAdjustRequest(const int64_t power, const uint32_t duration,
                                                           AdjustmentCauseEnum cause) override;

    /**
     * @brief Handle CancelPowerAdjustRequest command
     *
     * The cluster verifies that PowerAdjustmentCapability.cause is updated to kNoAdjustment.
     */
    Protocols::InteractionModel::Status CancelPowerAdjustRequest() override;

    /**
     * @brief Handle StartTimeAdjustRequest command
     *
     * The cluster verifies:
     * - Forecast.startTime is updated to requestedStartTime
     * - Forecast.forecastID is incremented (new ForecastID)
     * - Forecast.endTime is updated to requestedStartTime + duration
     */
    Protocols::InteractionModel::Status StartTimeAdjustRequest(const uint32_t requestedStartTime,
                                                               AdjustmentCauseEnum cause) override;

    Protocols::InteractionModel::Status PauseRequest(const uint32_t duration, AdjustmentCauseEnum cause) override;

    /**
     * @brief Handle ResumeRequest command
     *
     * The cluster verifies:
     * - ESAState is changed from kPaused (GetESAState() != kPaused)
     * - Forecast.forecastUpdateReason is set to kInternalOptimization
     */
    Protocols::InteractionModel::Status ResumeRequest() override;

    Protocols::InteractionModel::Status
    ModifyForecastRequest(const uint32_t forecastID,
                          const DataModel::DecodableList<Structs::SlotAdjustmentStruct::Type> & slotAdjustments,
                          AdjustmentCauseEnum cause) override;

    Protocols::InteractionModel::Status
    RequestConstraintBasedForecast(const DataModel::DecodableList<Structs::ConstraintsStruct::Type> & constraints,
                                   AdjustmentCauseEnum cause) override;

    /**
     * @brief Handle CancelRequest command
     *
     * The cluster verifies that Forecast.forecastUpdateReason is set to kInternalOptimization.
     */
    Protocols::InteractionModel::Status CancelRequest() override;

    ESATypeEnum GetESAType() override;
    bool GetESACanGenerate() override;
    ESAStateEnum GetESAState() override;
    int64_t GetAbsMinPower() override;
    int64_t GetAbsMaxPower() override;
    OptOutStateEnum GetOptOutState() override;

    const DataModel::Nullable<Structs::PowerAdjustCapabilityStruct::Type> & GetPowerAdjustmentCapability() override;

    const DataModel::Nullable<Structs::ForecastStruct::Type> & GetForecast() override;

    CHIP_ERROR SetESAState(ESAStateEnum) override;

    // Test helpers to set internal state
    void SetForecast(const DataModel::Nullable<Structs::ForecastStruct::Type> & forecast) { mForecast = forecast; }
    DataModel::Nullable<Structs::ForecastStruct::Type> & GetForecastMutable() { return mForecast; }
    void SetOptOutState(OptOutStateEnum state) { mOptOutState = state; }
#endif

private:
    CommissioningProxyCluster * mServer = nullptr;
    uint8_t     mScanMaxTime        = 120;
    // uint8_t     mMaxSessions        = 1;
    // uint8_t     mMaxCachedResult    = 1;
    // uint16_t    mCacheTimeout       = 120;
#if 0
 ESAStateEnum mESAState       = ESAStateEnum::kOnline;
    ESATypeEnum mESAType         = ESATypeEnum::kEvse;
    bool mESACanGenerate         = false;
    int64_t mAbsMinPower         = kAbsMinPower;
    int64_t mAbsMaxPower         = kAbsMaxPower;
    OptOutStateEnum mOptOutState = OptOutStateEnum::kNoOptOut;
    DataModel::Nullable<Structs::PowerAdjustCapabilityStruct::Type> mPowerAdjustmentCapability;
    DataModel::Nullable<Structs::ForecastStruct::Type> mForecast;
#endif
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
