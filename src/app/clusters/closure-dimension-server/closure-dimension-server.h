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

#include "closure-dimension-cluster-objects.h"
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

using namespace chip;

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

/**
 * @brief Defines methods for implementing application-specific logic for the Closure Dimension Cluster.
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
    virtual Protocols::InteractionModel::Status SetTarget(const Optional<Percent100ths> & pos, const Optional<TargetLatchEnum> & latch,
                                                       const Optional<Globals::ThreeLevelAutoEnum> & speed) = 0;
    virtual Protocols::InteractionModel::Status Step(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                                                        const Optional<Globals::ThreeLevelAutoEnum> & speed) = 0;
    // ------------------------------------------------------------------

protected:
    EndpointId mEndpointId = chip::kInvalidEndpointId;
};

class Instance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    /**
     * @brief Creates a closure dimension cluster instance. The Init() function needs to be called for
     *        this instance to be registered and called by the interaction model at the appropriate times.
     * @param[in] aEndpointId The endpoint on which this cluster exists.
     * @param[in] aDelegate The Delegate used by this Instance.
     * @param[in] aFeatures The bitmask value that identifies which features are supported by this instance.
     */
    Instance(EndpointId aEndpointId, Delegate & aDelegate, BitMask<Feature> aFeatures) :
        AttributeAccessInterface(MakeOptional(aEndpointId), ClosureDimension::Id),
        CommandHandlerInterface(MakeOptional(aEndpointId), ClosureDimension::Id), mDelegate(aDelegate), mFeatures(aFeatures)
    {
        /* set the base class delegates endpointId */
        mDelegate.SetEndpointId(aEndpointId);
    }
    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeatures) const;

    // Attribute setters
    /**
     * @brief Set Current.
     * @param[in] aCurrent Current Position, Latching and/or Speed.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetCurrent(GenericCurrentStruct & aCurrent);
    /**
     * @brief Set Target.
     * @param[in] aTarget Target Position, Latching and/or Speed.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetTarget(GenericTargetStruct & aTarget);
    /**
     * @brief Set Resolution.
     * @param[in] aResolution Minimal acceptable change of Position fields of attributes.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetResolution(const Percent100ths aResolution);
    /**
     * @brief Set StepValue.
     * @param[in] aStepValue One step value for Step command
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetStepValue(const Percent100ths aStepValue);
    /**
     * @brief Set Unit.
     * @param[in] aUnit Unit related to the Positioning.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetUnit(const ClosureUnitEnum aUnit);
    /**
     * @brief Set UnitRange.
     * @param[in] aUnitRange Minimum and Maximum values expressed by positioning following the unit.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetUnitRange(Structs::UnitRangeStruct::Type & aUnitRange);
    /**
     * @brief Set LimitRange.
     * @param[in] aLimitRange Range of possible values for the position field in Current attribute.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetLimitRange(Structs::RangePercent100thsStruct::Type & aLimitRange);
    /**
     * @brief Set TranslationDirection.
     * @param[in] aTranslationDirection Direction of the translation.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetTranslationDirection(const TranslationDirectionEnum aTranslationDirection);
    /**
     * @brief Set RotationAxis.
     * @param[in] aRotationAxis Axis of the rotation.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetRotationAxis(const RotationAxisEnum aRotationAxis);
    /**
     * @brief Set Overflow.
     * @param[in] aOverflow Overflow related to Rotation.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetOverflow(const OverflowEnum aOverflow);
    /**
     * @brief Set ModulationType.
     * @param[in] aModulationType Modulation type.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetModulationType(const ModulationTypeEnum aModulationType);

    // Attribute getters
    /**
     * @brief Get Current.
     * @return Current Position, Latching and/or Speed.
     */
    const GenericCurrentStruct & GetCurrent() const;
    /**
     * @brief Get Target.
     * @return Target Position, Latching and/or Speed.
     */
    const GenericTargetStruct & GetTarget() const;
    /**
     * @brief Get Resolution.
     * @return Minimal acceptable change of Position fields in Target and Current attributes.
     */
    Percent100ths GetResolution() const;
    /**
     * @brief Get StepValue.
     * @return one Step value, which is used by Step Command.
     */
    Percent100ths GetStepValue() const;
    /**
     * @brief Get Unit.
     * @return Unit related to the Positioning.
     */
    ClosureUnitEnum GetUnit() const;
    /**
     * @brief Get UnitRange.
     * @return Struct of minimum and maximum value expressed by Positioning following the unit indicated by "Unit Attribute".
     */
    const Structs::UnitRangeStruct::Type & GetUnitRange() const;
    /**
     * @brief Get LimitRange.
     * @return Struct of minimum and maximum value for the Position field of Current attribute..
     */
    const Structs::RangePercent100thsStruct::Type & GetLimitRange() const;
    /**
     * @brief Get TranslationDirection.
     * @return Direction of Translation.
     */
    TranslationDirectionEnum GetTranslationDirection() const;
    /**
     * @brief Get RotationAxis.
     * @return Axis of the rotation.
     */
    RotationAxisEnum GetRotationAxis() const;
    /**
     * @brief Get Overflow.
     * @return Overflow related to Rotation.
     */
    OverflowEnum GetOverflow() const;
    /**
     * @brief Get ModulationType.
     * @return Modulation Type.
     */
    ModulationTypeEnum GetModulationType() const;
    
    
private:
    Delegate & mDelegate;
    BitMask<Feature> mFeatures;

    GenericCurrentStruct mCurrent;
    GenericTargetStruct mTarget;
    Percent100ths mResolution;
    Percent100ths mStepValue;
    ClosureUnitEnum mUnit;
    Structs::UnitRangeStruct::Type mUnitRange;
    Structs::RangePercent100thsStruct::Type mLimitRange;
    TranslationDirectionEnum mTranslationDirection;
    RotationAxisEnum mRotationAxis;
    OverflowEnum mOverflow;
    ModulationTypeEnum mModulationType;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & handlerContext) override;

    void HandleSetTarget(HandlerContext & ctx, const Commands::SetTarget::DecodableType & commandData);
    void HandleStep(HandlerContext & ctx, const Commands::Step::DecodableType & commandData);
};

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
