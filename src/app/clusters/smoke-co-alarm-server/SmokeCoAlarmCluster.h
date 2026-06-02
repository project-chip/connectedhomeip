/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

/**
 * @file
 *   APIs and defines for the Smoke CO Alarm Server plugin.
 *
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SmokeCoAlarm {

class SmokeCoAlarmDelegate;

class SmokeCoAlarmCluster : public DefaultServerCluster
{
public:
    /**
     * Tracks which optional attributes are present on this endpoint.
     * Populated at construction time from the ZAP/ember attribute store via CodegenIntegration.
     */
    using OptionalAttributeSet =
        chip::app::OptionalAttributeSet<Attributes::DeviceMuted::Id, Attributes::InterconnectSmokeAlarm::Id,
                                        Attributes::InterconnectCOAlarm::Id, Attributes::ContaminationState::Id,
                                        Attributes::SmokeSensitivityLevel::Id, Attributes::ExpiryDate::Id,
                                        Attributes::Unmounted::Id>;
    /**
     * Construction-time configuration.
     * Passed in by CodegenIntegration from the ZAP static config.
     */
    struct Config
    {
        chip::BitFlags<Feature> featureMap;
        OptionalAttributeSet optionalAttribs;
        bool inoperativeWhenUnmounted = false;
    };

    explicit SmokeCoAlarmCluster(EndpointId endpointId);
    explicit SmokeCoAlarmCluster(EndpointId endpointId, const Config & config);

    /* Expected byte size of the PriorityOrder */
    static constexpr size_t kPriorityOrderLength = 9;

    /**
     * @brief Set the highest level of Expressed State according to priorityOrder
     * @param priorityOrder Priority order of expressed state from highest to lowest
     */
    void SetExpressedStateByPriority(const std::array<ExpressedStateEnum, kPriorityOrderLength> & priorityOrder);

    /**
     * @brief Request self-test when receiving local commands
     * @return true on success, false on failure
     */
    bool RequestSelfTest();

    /**
     * For all the functions below, the return value is true on success, false on failure
     */

    bool SetSmokeState(AlarmStateEnum newSmokeState);
    bool SetCOState(AlarmStateEnum newCOState);
    bool SetBatteryAlert(AlarmStateEnum newBatteryAlert);
    bool SetDeviceMuted(MuteStateEnum newDeviceMuted);
    bool SetTestInProgress(bool newTestInProgress);
    bool SetHardwareFaultAlert(bool newHardwareFaultAlert);
    bool SetEndOfServiceAlert(EndOfServiceEnum newEndOfServiceAlert);
    bool SetInterconnectSmokeAlarm(AlarmStateEnum newInterconnectSmokeAlarm);
    bool SetInterconnectCOAlarm(AlarmStateEnum newInterconnectCOAlarm);
    bool SetContaminationState(ContaminationStateEnum newContaminationState);
    bool SetSmokeSensitivityLevel(SensitivityEnum newSmokeSensitivityLevel);
    bool SetExpiryDate(uint32_t newExpiryDate);
    /**
     * @brief Sets Unmounted attribute and updates ExpressedState accordingly.
     * @param isUnmounted new unmounted state
     * @return true on success, false on failure
     */
    bool SetUnmountedState(bool isUnmounted);

    void SetInoperativeWhenUnmounted(bool v) { mInoperativeWhenUnmounted = v; }
    void SetDelegate(SmokeCoAlarmDelegate * delegate) { mDelegate = delegate; }

    bool GetExpressedState(ExpressedStateEnum & expressedState) const;
    bool GetSmokeState(AlarmStateEnum & smokeState) const;
    bool GetCOState(AlarmStateEnum & coState) const;
    bool GetBatteryAlert(AlarmStateEnum & batteryAlert) const;
    bool GetDeviceMuted(MuteStateEnum & deviceMuted) const;
    bool GetTestInProgress(bool & testInProgress) const;
    bool GetHardwareFaultAlert(bool & hardwareFaultAlert) const;
    bool GetEndOfServiceAlert(EndOfServiceEnum & endOfServiceAlert) const;
    bool GetInterconnectSmokeAlarm(AlarmStateEnum & interconnectSmokeAlarm) const;
    bool GetInterconnectCOAlarm(AlarmStateEnum & interconnectCOAlarm) const;
    bool GetContaminationState(ContaminationStateEnum & contaminationState) const;
    bool GetSmokeSensitivityLevel(SensitivityEnum & smokeSensitivityLevel) const;
    bool GetExpiryDate(uint32_t & expiryDate) const;
    bool GetUnmountedState(bool & unmountedState) const;

    chip::BitFlags<Feature> GetFeatures() const { return mConfig.featureMap; }
    bool SupportsSmokeAlarm() const { return mConfig.featureMap.Has(Feature::kSmokeAlarm); }
    bool SupportsCOAlarm() const { return mConfig.featureMap.Has(Feature::kCoAlarm); }

    // DefaultServerCluster overrides
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
        override; /**
                   * @brief Common handler for the SelfTestRequest command
                   */
    void HandleRemoteSelfTestRequest(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath);

private:
    /**
     * @brief Updates the expressed state with new value
     *
     * @note If the value of ExpressedState is not Normal, the attribute corresponding to the
     *       value should not be Normal.
     */
    void SetExpressedState(ExpressedStateEnum newExpressedState);

    /**
     * @brief Send generic event
     *
     * @tparam T  Any event type supported by Matter
     */
    template <typename T>
    void SendEvent(T & event);

    const Config mConfig;
    bool mInoperativeWhenUnmounted   = false;
    SmokeCoAlarmDelegate * mDelegate = nullptr;

    // Runtime attribute storage
    ExpressedStateEnum mExpressedState         = ExpressedStateEnum::kNormal;
    AlarmStateEnum mSmokeState                 = AlarmStateEnum::kNormal;
    AlarmStateEnum mCOState                    = AlarmStateEnum::kNormal;
    AlarmStateEnum mBatteryAlert               = AlarmStateEnum::kNormal;
    MuteStateEnum mDeviceMuted                 = MuteStateEnum::kNotMuted;
    bool mTestInProgress                       = false;
    bool mHardwareFaultAlert                   = false;
    EndOfServiceEnum mEndOfServiceAlert        = EndOfServiceEnum::kNormal;
    AlarmStateEnum mInterconnectSmokeAlarm     = AlarmStateEnum::kNormal;
    AlarmStateEnum mInterconnectCOAlarm        = AlarmStateEnum::kNormal;
    ContaminationStateEnum mContaminationState = ContaminationStateEnum::kNormal;
    SensitivityEnum mSmokeSensitivityLevel     = SensitivityEnum::kHigh;
    uint32_t mExpiryDate                       = 0;
    bool mUnmounted                            = false;
};

/**
 * @brief Application delegate for SmokeCoAlarmCluster.
 *
 * Implement this to provide device-specific self-test behaviour. Pass the
 * concrete instance to SmokeCoAlarmServer::Init() or
 * SmokeCoAlarmCluster::SetDelegate().
 */
class SmokeCoAlarmDelegate
{
public:
    virtual ~SmokeCoAlarmDelegate() = default;

    /**
     * @brief Called when a self-test is initiated (locally or via the
     *        SelfTestRequest command).  The cluster has already set
     *        TestInProgress=true and ExpressedState=Testing before this
     *        is invoked.
     */
    virtual void OnSelfTestRequested() = 0;

protected:
    friend class SmokeCoAlarmCluster;
    SmokeCoAlarmCluster * mCluster = nullptr;
};

} // namespace SmokeCoAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
