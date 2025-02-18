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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/StatusIB.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>
#include <app/cluster-building-blocks/QuieterReporting.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

/** @brief
 *    Defines methods for implementing application-specific logic for the Closure Control Cluster.
 */
class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }
    EndpointId GetEndpointId() { return mEndpointId; }

    // ------------------------------------------------------------------
    // Commands
    virtual Protocols::InteractionModel::Status Stop()                                                    = 0;
    virtual Protocols::InteractionModel::Status MoveTo(const Optional<TagPositionEnum> tag, const Optional<TagLatchEnum> latch,
                                                       const Optional<Globals::ThreeLevelAutoEnum> speed) = 0;
    virtual Protocols::InteractionModel::Status Calibrate()                                               = 0;
    virtual Protocols::InteractionModel::Status ConfigureFallback(const Optional<RestingProcedureEnum> restingProcedure,
                                                                  const Optional<TriggerConditionEnum> triggerCondition,
                                                                  const Optional<TriggerPositionEnum> triggerPosition,
                                                                  const Optional<uint32_t> waitingDelay)  = 0;
    virtual Protocols::InteractionModel::Status CancelFallback()                                          = 0;

    // ------------------------------------------------------------------
    // Get attribute methods
    virtual DataModel::Nullable<uint32_t> GetCountdownTime()                           = 0;
    virtual RestingProcedureEnum GetRestingProcedure()                                 = 0;
    virtual TriggerConditionEnum GetTriggerCondition()                                 = 0;
    virtual TriggerPositionEnum GetTriggerPosition()                                   = 0;
    virtual uint32_t GetWaitingDelay()                                                 = 0;
    virtual uint32_t GetKickoffTimer()                                                 = 0;

    /* These functions are called by the ReadAttribute handler to iterate through lists
     * The cluster server will call Start<Type>Read to allow the delegate to create a temporary
     * lock on the data.
     * The delegate is expected to not change these values once Start<Type>Read has been called
     * until the End<Type>Read() has been called (e.g. releasing a lock on the data)
     */
    virtual CHIP_ERROR StartCurrentErrorListRead()                            = 0;
    virtual CHIP_ERROR GetCurrentErrorListAtIndex(size_t, ClosureErrorEnum &) = 0;
    virtual CHIP_ERROR EndCurrentErrorListRead()                              = 0;

protected:
    EndpointId mEndpointId = 0;
};

enum class OptionalAttributes : uint32_t
{
    kCountdownTime = 0x1
};

class Instance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature, OptionalAttributes aOptionalAttrs, ClusterId aClusterId) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id), CommandHandlerInterface(MakeOptional(aEndpointId), Id),
        mDelegate(aDelegate), mClusterId(aClusterId), mFeature(aFeature), mOptionalAttrs(aOptionalAttrs)
    {
        /* set the base class delegates endpointId */
        mDelegate.SetEndpointId(aEndpointId);
    }
    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;
    bool SupportsOptAttr(OptionalAttributes aOptionalAttrs) const;
    
    // Attribute setters
    /**
     * Set Main State.
     * @param aMainState The Main state that should now be the current one.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetMainState(const MainStateEnum & aMainState);
    
    /**
     * Set OverallState.
     * @param aMainState The OverallState that should now be the current State.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetOverallState(const DataModel::Nullable<Structs::OverallStateStruct::Type> & aOverallState);

    /**
     * Set OverallTarget.
     * @param aMainState The OverallTarget that should be set.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetOverallTarget(const DataModel::Nullable<Structs::OverallTargetStruct::Type> & aOverallTarget);

    // Attribute getters
    /**
     * Get Main State.
     * @return The Main State.
     */
    MainStateEnum GetMainState() const;
    
    /**
     * Get OverallState.
     * @return The OverallState.
     */
    DataModel::Nullable<Structs::OverallStateStruct::Type> GetOverallState() const;

    /**
     * Get OverallTarget.
     * @return The OverallTarget.
     */
    DataModel::Nullable<Structs::OverallTargetStruct::Type> GetOverallTarget() const;

    /**
     * @brief Whenever application delegate wants to possibly report a new updated time,
     *        call this method. The `GetCountdownTime()` method will be called on the delegate.
     */
    void UpdateCountdownTimeFromDelegate() { UpdateCountdownTime(/* fromDelegate = */ true); }
    
    /**
     * This function returns true if the phase value given exists in the PhaseList attribute, otherwise it returns false.
     */
    bool IsSupportedState(MainStateEnum aMainState);

protected:
    /**
     * Causes reporting/udpating of CountdownTime attribute from driver if sufficient changes have
     * occurred (based on Q quality definition for operational state). Calls the Delegate::GetCountdownTime() method.
     *
     * @param fromDelegate true if the change notice was triggered by the delegate, false if internal to cluster logic.
     */
    void UpdateCountdownTime(bool fromDelegate);

    /**
     * @brief Whenever the cluster logic thinks time should be updated, call this.
     */
    void UpdateCountdownTimeFromClusterLogic() { UpdateCountdownTime(/* fromDelegate=*/false); }

private:
    Delegate & mDelegate;
    const ClusterId mClusterId;
    BitMask<Feature> mFeature;
    BitMask<OptionalAttributes> mOptionalAttrs;

    app::QuieterReportingAttribute<uint32_t> mCountdownTime{ DataModel::NullNullable };
    MainStateEnum mMainState;
    DataModel::Nullable<Structs::OverallStateStruct::Type> & mOverallState;
    DataModel::Nullable<Structs::OverallTargetStruct::Type> & mOverallTarget;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    CHIP_ERROR EncodeCurrentErrorList(const AttributeValueEncoder::ListEncodeHelper & aEncoder);

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & handlerContext) override;

    void HandleStop(HandlerContext & ctx, const Commands::Stop::DecodableType & commandData);
    void HandleMoveTo(HandlerContext & ctx, const Commands::MoveTo::DecodableType & commandData);
    void HandleCalibrate(HandlerContext & ctx, const Commands::Calibrate::DecodableType & commandData);
    void HandleConfigureFallback(HandlerContext & ctx, const Commands::ConfigureFallback::DecodableType & commandData);
    void HandleCancelFallback(HandlerContext & ctx, const Commands::CancelFallback::DecodableType & commandData);
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
