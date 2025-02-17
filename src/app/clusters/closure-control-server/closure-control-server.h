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
     virtual Protocols::InteractionModel::Status Stop() = 0;
     virtual Protocols::InteractionModel::Status MoveTo(const Optional<TagPositionEnum> tag,
                                                        const Optional<TagLatchEnum> latch,
                                                        const Optional<Globals::ThreeLevelAutoEnum> speed) = 0;
     virtual Protocols::InteractionModel::Status Calibrate() = 0;
     virtual Protocols::InteractionModel::Status ConfigureFallback(const Optional<RestingProcedureEnum> restingProcedure,
                                                                   const Optional<TriggerConditionEnum> triggerCondition,
                                                                   const Optional<TriggerPositionEnum> triggerPosition,
                                                                   const Optional<uint32_t> waitingDelay) = 0;
     virtual Protocols::InteractionModel::Status CancelFallback() = 0;

     // ------------------------------------------------------------------
     // Get attribute methods
     virtual DataModel::Nullable<uint32_t> GetCountdownTime() = 0;
     virtual MainStateEnum GetMainState() = 0;
     virtual DataModel::Nullable<Structs::OverallStateStruct::Type> GetOverallState() = 0;
     virtual DataModel::Nullable<Structs::OverallTargetStruct::Type> GetOverallTarget() = 0;
     virtual RestingProcedureEnum GetRestingProcedure() = 0;
     virtual TriggerConditionEnum GetTriggerCondition() = 0;
     virtual TriggerPositionEnum GetTriggerPosition() = 0;
     virtual uint32_t GetWaitingDelay() = 0;
     virtual uint32_t GetKickoffTimer() = 0;

     /* These functions are called by the ReadAttribute handler to iterate through lists
      * The cluster server will call Start<Type>Read to allow the delegate to create a temporary
      * lock on the data.
      * The delegate is expected to not change these values once Start<Type>Read has been called
      * until the End<Type>Read() has been called (e.g. releasing a lock on the data)
      */
     virtual CHIP_ERROR StartCurrentErrorListRead() = 0;
     virtual CHIP_ERROR GetCurrentErrorListAtIndex(size_t,ClosureErrorEnum  &) = 0;
     virtual CHIP_ERROR EndCurrentErrorListRead() = 0;

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
     Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature,
             OptionalAttributes aOptionalAttrs) :
         AttributeAccessInterface(MakeOptional(aEndpointId), Id),
         CommandHandlerInterface(MakeOptional(aEndpointId), Id), mDelegate(aDelegate),
         mFeature(aFeature), mOptionalAttrs(aOptionalAttrs)
     {
         /* set the base class delegates endpointId */
         mDelegate.SetEndpointId(aEndpointId);
     }
     ~Instance() { Shutdown(); }

     CHIP_ERROR Init();
     void Shutdown();

     bool HasFeature(Feature aFeature) const;
     bool SupportsOptAttr(OptionalAttributes aOptionalAttrs) const;

 private:
     Delegate & mDelegate;
     BitMask<Feature> mFeature;
     BitMask<OptionalAttributes> mOptionalAttrs;

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
