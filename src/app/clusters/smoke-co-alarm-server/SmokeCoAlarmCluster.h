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

using namespace chip::app::Clusters::SmokeCoAlarm;

namespace chip {
namespace app {
namespace Clusters {

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
     * For the bool-returning setters below, false means the operation is not supported
     * (feature disabled or optional attribute absent). void-returning setters always succeed.
     */

    bool SetSmokeState(AlarmStateEnum newSmokeState);
    bool SetCOState(AlarmStateEnum newCOState);
    void SetBatteryAlert(AlarmStateEnum newBatteryAlert);
    bool SetDeviceMuted(MuteStateEnum newDeviceMuted);
    void SetTestInProgress(bool newTestInProgress);
    void SetHardwareFaultAlert(bool newHardwareFaultAlert);
    void SetEndOfServiceAlert(EndOfServiceEnum newEndOfServiceAlert);
    bool SetInterconnectSmokeAlarm(AlarmStateEnum newInterconnectSmokeAlarm);
    bool SetInterconnectCOAlarm(AlarmStateEnum newInterconnectCOAlarm);
    void SetContaminationState(ContaminationStateEnum newContaminationState);
    void SetSmokeSensitivityLevel(SensitivityEnum newSmokeSensitivityLevel);
    void SetExpiryDate(uint32_t newExpiryDate);
    /**
     * @brief Sets Unmounted attribute and updates ExpressedState accordingly.
     * @param isUnmounted new unmounted state
     * @return true on success, false on failure
     */
    bool SetUnmountedState(bool isUnmounted);

    void SetInoperativeWhenUnmounted(bool v) { mInoperativeWhenUnmounted = v; }
    void SetDelegate(SmokeCoAlarmDelegate * delegate) { mDelegate = delegate; }

    ExpressedStateEnum GetExpressedState() const { return mExpressedState; }
    AlarmStateEnum GetSmokeState() const { return mSmokeState; }
    AlarmStateEnum GetCOState() const { return mCOState; }
    AlarmStateEnum GetBatteryAlert() const { return mBatteryAlert; }
    MuteStateEnum GetDeviceMuted() const { return mDeviceMuted; }
    bool GetTestInProgress() const { return mTestInProgress; }
    bool GetHardwareFaultAlert() const { return mHardwareFaultAlert; }
    EndOfServiceEnum GetEndOfServiceAlert() const { return mEndOfServiceAlert; }
    AlarmStateEnum GetInterconnectSmokeAlarm() const { return mInterconnectSmokeAlarm; }
    AlarmStateEnum GetInterconnectCOAlarm() const { return mInterconnectCOAlarm; }
    ContaminationStateEnum GetContaminationState() const { return mContaminationState; }
    SensitivityEnum GetSmokeSensitivityLevel() const { return mSmokeSensitivityLevel; }
    uint32_t GetExpiryDate() const { return mExpiryDate; }
    bool GetUnmountedState() const { return mUnmounted; }

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
    void SetExpressedState(ExpressedStateEnum newExpressedState);

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
    SensitivityEnum mSmokeSensitivityLevel     = SensitivityEnum::kStandard;
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
};

} // namespace Clusters
} // namespace app
} // namespace chip
