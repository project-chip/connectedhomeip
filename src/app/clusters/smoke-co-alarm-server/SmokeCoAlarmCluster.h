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

#pragma once

#include <app/CommandHandler.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/SmokeCoAlarm/Attributes.h>
#include <clusters/SmokeCoAlarm/Events.h>

namespace chip {
namespace app {
namespace Clusters {

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
};

class SmokeCoAlarmCluster : public DefaultServerCluster
{
public:
    using OptionalAttributeSet = chip::app::OptionalAttributeSet<
        SmokeCoAlarm::Attributes::DeviceMuted::Id, SmokeCoAlarm::Attributes::InterconnectSmokeAlarm::Id,
        SmokeCoAlarm::Attributes::InterconnectCOAlarm::Id, SmokeCoAlarm::Attributes::ContaminationState::Id,
        SmokeCoAlarm::Attributes::SmokeSensitivityLevel::Id, SmokeCoAlarm::Attributes::ExpiryDate::Id,
        SmokeCoAlarm::Attributes::Unmounted::Id>;
    struct Config
    {
        chip::BitFlags<SmokeCoAlarm::Feature> featureMap;
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
    void SetExpressedStateByPriority(const std::array<SmokeCoAlarm::ExpressedStateEnum, kPriorityOrderLength> & priorityOrder);

    /**
     * @brief Request self-test when receiving local commands
     * @return true on success, false on failure
     */
    bool RequestSelfTest();

    /**
     * For the bool-returning setters below, false means the operation is not supported
     * (feature disabled or optional attribute absent). void-returning setters always succeed.
     */

    bool SetSmokeState(SmokeCoAlarm::AlarmStateEnum newSmokeState);
    bool SetCOState(SmokeCoAlarm::AlarmStateEnum newCOState);
    void SetBatteryAlert(SmokeCoAlarm::AlarmStateEnum newBatteryAlert);
    bool SetDeviceMuted(SmokeCoAlarm::MuteStateEnum newDeviceMuted);
    void SetTestInProgress(bool newTestInProgress);
    void SetHardwareFaultAlert(bool newHardwareFaultAlert);
    void SetEndOfServiceAlert(SmokeCoAlarm::EndOfServiceEnum newEndOfServiceAlert);
    bool SetInterconnectSmokeAlarm(SmokeCoAlarm::AlarmStateEnum newInterconnectSmokeAlarm);
    bool SetInterconnectCOAlarm(SmokeCoAlarm::AlarmStateEnum newInterconnectCOAlarm);
    void SetContaminationState(SmokeCoAlarm::ContaminationStateEnum newContaminationState);
    void SetSmokeSensitivityLevel(SmokeCoAlarm::SensitivityEnum newSmokeSensitivityLevel);
    void SetExpiryDate(uint32_t newExpiryDate);
    /**
     * @brief Sets Unmounted attribute and updates ExpressedState accordingly.
     * @param isUnmounted new unmounted state
     * @return true on success, false on failure
     */
    bool SetUnmountedState(bool isUnmounted);

    void SetInoperativeWhenUnmounted(bool v) { mInoperativeWhenUnmounted = v; }
    void SetDelegate(SmokeCoAlarmDelegate * delegate) { mDelegate = delegate; }

    SmokeCoAlarm::ExpressedStateEnum GetExpressedState() const { return mExpressedState; }
    SmokeCoAlarm::AlarmStateEnum GetSmokeState() const { return mSmokeState; }
    SmokeCoAlarm::AlarmStateEnum GetCOState() const { return mCOState; }
    SmokeCoAlarm::AlarmStateEnum GetBatteryAlert() const { return mBatteryAlert; }
    SmokeCoAlarm::MuteStateEnum GetDeviceMuted() const { return mDeviceMuted; }
    bool GetTestInProgress() const { return mTestInProgress; }
    bool GetHardwareFaultAlert() const { return mHardwareFaultAlert; }
    SmokeCoAlarm::EndOfServiceEnum GetEndOfServiceAlert() const { return mEndOfServiceAlert; }
    SmokeCoAlarm::AlarmStateEnum GetInterconnectSmokeAlarm() const { return mInterconnectSmokeAlarm; }
    SmokeCoAlarm::AlarmStateEnum GetInterconnectCOAlarm() const { return mInterconnectCOAlarm; }
    SmokeCoAlarm::ContaminationStateEnum GetContaminationState() const { return mContaminationState; }
    SmokeCoAlarm::SensitivityEnum GetSmokeSensitivityLevel() const { return mSmokeSensitivityLevel; }
    uint32_t GetExpiryDate() const { return mExpiryDate; }
    bool GetUnmountedState() const { return mUnmounted; }

    chip::BitFlags<SmokeCoAlarm::Feature> GetFeatures() const { return mConfig.featureMap; }
    bool SupportsSmokeAlarm() const { return mConfig.featureMap.Has(SmokeCoAlarm::Feature::kSmokeAlarm); }
    bool SupportsCOAlarm() const { return mConfig.featureMap.Has(SmokeCoAlarm::Feature::kCoAlarm); }

    // DefaultServerCluster overrides
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

private:
    Protocols::InteractionModel::Status HandleRemoteSelfTestRequest();
    /**
     * @brief Updates the expressed state with new value
     *
     * @note If the value of ExpressedState is not Normal, the attribute corresponding to the
     *       value should not be Normal.
     */
    void SetExpressedState(SmokeCoAlarm::ExpressedStateEnum newExpressedState);

    const Config mConfig;
    bool mInoperativeWhenUnmounted   = false;
    SmokeCoAlarmDelegate * mDelegate = nullptr;

    // Runtime attribute storage
    SmokeCoAlarm::ExpressedStateEnum mExpressedState         = SmokeCoAlarm::ExpressedStateEnum::kNormal;
    SmokeCoAlarm::AlarmStateEnum mSmokeState                 = SmokeCoAlarm::AlarmStateEnum::kNormal;
    SmokeCoAlarm::AlarmStateEnum mCOState                    = SmokeCoAlarm::AlarmStateEnum::kNormal;
    SmokeCoAlarm::AlarmStateEnum mBatteryAlert               = SmokeCoAlarm::AlarmStateEnum::kNormal;
    SmokeCoAlarm::MuteStateEnum mDeviceMuted                 = SmokeCoAlarm::MuteStateEnum::kNotMuted;
    bool mTestInProgress                                     = false;
    bool mHardwareFaultAlert                                 = false;
    SmokeCoAlarm::EndOfServiceEnum mEndOfServiceAlert        = SmokeCoAlarm::EndOfServiceEnum::kNormal;
    SmokeCoAlarm::AlarmStateEnum mInterconnectSmokeAlarm     = SmokeCoAlarm::AlarmStateEnum::kNormal;
    SmokeCoAlarm::AlarmStateEnum mInterconnectCOAlarm        = SmokeCoAlarm::AlarmStateEnum::kNormal;
    SmokeCoAlarm::ContaminationStateEnum mContaminationState = SmokeCoAlarm::ContaminationStateEnum::kNormal;
    SmokeCoAlarm::SensitivityEnum mSmokeSensitivityLevel     = SmokeCoAlarm::SensitivityEnum::kStandard;
    uint32_t mExpiryDate                                     = 0;
    bool mUnmounted                                          = false;
};

} // namespace Clusters
} // namespace app
} // namespace chip
