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

#include "closure-control-cluster-objects.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/StatusIB.h>
#include <app/cluster-building-blocks/QuieterReporting.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

/**
 * @brief Defines methods for implementing application-specific logic for the Closure Control Cluster.
 */
class Delegate
{
public:
    virtual ~Delegate() = default;

    // Only Cluster Instance should be calling SetEdpointId.
    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }
    EndpointId GetEndpointId() { return mEndpointId; }

    // ------------------------------------------------------------------
    // Commands
    virtual Protocols::InteractionModel::Status Stop()                                                      = 0;
    virtual Protocols::InteractionModel::Status MoveTo(const Optional<TagPositionEnum> & tag, const Optional<TagLatchEnum> & latch,
                                                       const Optional<Globals::ThreeLevelAutoEnum> & speed) = 0;
    virtual Protocols::InteractionModel::Status Calibrate()                                                 = 0;

    // ------------------------------------------------------------------
    // Get attribute methods
    virtual DataModel::Nullable<uint32_t> GetCountdownTime() = 0;

    /* These functions are called by the ReadAttribute handler to iterate through lists
     * The cluster server will call Start<Type>Read to allow the delegate to create a temporary
     * lock on the data.
     * The delegate is expected to not change these values once Start<Type>Read has been called
     * until the End<Type>Read() has been called (i.e. releasing a lock on the data)
     */
    virtual CHIP_ERROR StartCurrentErrorListRead() = 0;
    // The delegate is expected to return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED to indicate end of list.
    virtual CHIP_ERROR GetCurrentErrorListAtIndex(size_t, ClosureErrorEnum &) = 0;
    virtual CHIP_ERROR EndCurrentErrorListRead()                              = 0;

protected:
    EndpointId mEndpointId = chip::kInvalidEndpointId;
};

enum class OptionalAttribute : uint32_t
{
    kCountdownTime = 0x1
};

class Instance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    /**
     * @brief Creates a closure control cluster instance. The Init() function needs to be called for
     *        this instance to be registered and called by the interaction model at the appropriate times.
     * @param[in] aEndpointId The endpoint on which this cluster exists.
     * @param[in] aDelegate The Delegate used by this Instance.
     * @param[in] aFeatures The bitmask value that identifies which features are supported by this instance.
     * @param[in] aOptionalAttrs The bitmask Value that identifies which optional attributes are supported by this instance.
     */
    Instance(EndpointId aEndpointId, Delegate & aDelegate, BitMask<Feature> aFeatures, BitMask<OptionalAttribute> aOptionalAttrs) :
        AttributeAccessInterface(MakeOptional(aEndpointId), ClosureControl::Id),
        CommandHandlerInterface(MakeOptional(aEndpointId), ClosureControl::Id), mDelegate(aDelegate), mFeatures(aFeatures),
        mOptionalAttrs(aOptionalAttrs)
    {
        /* set the base class delegates endpointId */
        mDelegate.SetEndpointId(aEndpointId);
        /* set Countdown Time quiet reporting policy as per reporting requirements in specification */
        mCountdownTime.policy()
            .Set(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement)
            .Set(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero);
    }
    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeatures) const;
    bool SupportsOptAttr(OptionalAttribute aOptionalAttrs) const;

    // Attribute setters
    /**
     * @brief Set Main State.
     * @param[in] aMainState The Main state that should now be the current one.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetMainState(MainStateEnum aMainState);

    /**
     * @brief Set OverallState.
     * @param[in] aOverallState The OverallState that should now be the current State.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetOverallState(const GenericOverallState & aOverallState);

    /**
     * @brief Set OverallTarget.
     * @param[in] aOverallTarget The OverallTarget that should now be the Overall Target.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetOverallTarget(const GenericOverallTarget & aOverallTarget);

    // Attribute getters
    /**
     * @brief Get MainState.
     * @return Current MainState.
     */
    MainStateEnum GetMainState() const;

    /**
     * @brief Get OverallState.
     * @return Current OverallState.
     */
    const GenericOverallState & GetOverallState() const;

    /**
     * @brief Get OverallTarget.
     * @return Current OverallTarget.
     */
    const GenericOverallTarget & GetOverallTarget() const;

    /**
     * @brief Whenever application wants to trigger an update to report a new updated time,
     *        call this method. The `GetCountdownTime()` method will be called on the application.
     */
    inline void UpdateCountdownTimeFromDelegate() { UpdateCountdownTime(/* fromDelegate = */ true); }

    /**
     * @brief This function checks if Main State is supported or not based on features supported.
     * @param[in] aMainState MainState
     * @return true if State is supported, false if State is not supported
     */
    bool IsSupportedState(MainStateEnum aMainState);

protected:
    /**
     * @brief Causes reporting/udpating of CountdownTime attribute from driver if sufficient changes have
     *        occurred (based on Q quality definition for operational state). Calls the Delegate::GetCountdownTime() method.
     *
     * @param[in] fromDelegate true if the change notice was triggered by the delegate, false if internal to cluster logic.
     */
    void UpdateCountdownTime(bool fromDelegate);

    /**
     * @brief Whenever cluster logic wants to trigger an update to report a new updated time,
     *        call this method.
     */
    inline void UpdateCountdownTimeFromClusterLogic() { UpdateCountdownTime(/* fromDelegate=*/false); }

private:
    Delegate & mDelegate;
    BitMask<Feature> mFeatures;
    BitMask<OptionalAttribute> mOptionalAttrs;

    app::QuieterReportingAttribute<uint32_t> mCountdownTime{ DataModel::NullNullable };
    MainStateEnum mMainState;
    GenericOverallState mOverallState;
    GenericOverallTarget mOverallTarget;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    CHIP_ERROR EncodeCurrentErrorList(const AttributeValueEncoder::ListEncodeHelper & aEncoder);

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & handlerContext) override;

    void HandleStop(HandlerContext & ctx, const Commands::Stop::DecodableType & commandData);
    void HandleMoveTo(HandlerContext & ctx, const Commands::MoveTo::DecodableType & commandData);
    void HandleCalibrate(HandlerContext & ctx, const Commands::Calibrate::DecodableType & commandData);
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
