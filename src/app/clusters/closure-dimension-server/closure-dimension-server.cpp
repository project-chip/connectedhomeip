/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
 *
 */

 #include "closure-dimension-server.h"
 #include "closure-dimension-cluster-logic.h"

 #include <app-common/zap-generated/cluster-objects.h>
 #include <app-common/zap-generated/ids/Attributes.h>
 #include <app-common/zap-generated/ids/Clusters.h>
 #include <app/AttributeAccessInterface.h>
 #include <app/AttributeAccessInterfaceRegistry.h>
 #include <app/CommandHandlerInterface.h>
 #include <app/CommandHandlerInterfaceRegistry.h>
 #include <app/ConcreteCommandPath.h>
 #include <app/data-model/Encode.h>
 #include <app/util/config.h>
 #include <lib/core/CHIPError.h>
 
 namespace chip {
 namespace app {
 namespace Clusters {
 namespace ClosureDimension {
 
 using namespace Attributes;
 using namespace Commands;
 using namespace Protocols::InteractionModel;
 namespace {
 
 CHIP_ERROR TranslateErrorToIMStatus(CHIP_ERROR err)
 {
     if (err == CHIP_NO_ERROR)
     {
         return err;
     }
     if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
     {
         return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
     }
     if (err == CHIP_ERROR_INCORRECT_STATE)
     {
         // what actually gets returned here? This is really an internal error, so failure seems perhaps correct.
         return CHIP_IM_GLOBAL_STATUS(Failure);
     }
     if (err == CHIP_ERROR_INVALID_ARGUMENT)
     {
         return CHIP_IM_GLOBAL_STATUS(ConstraintError);
     }
     // Catch-all error
     return CHIP_IM_GLOBAL_STATUS(Failure);
 }
 
 template <typename T, typename F>
 CHIP_ERROR EncodeRead(AttributeValueEncoder & aEncoder, const F & getter)
 {
     T ret;
     CHIP_ERROR err = getter(ret);
     if (err == CHIP_NO_ERROR)
     {
         err = aEncoder.Encode(ret);
     }

     return TranslateErrorToIMStatus(err);
 }
 
 } // namespace
 
 CHIP_ERROR Interface::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
 {
     switch (aPath.mAttributeId)
     {
     case Current::Id: {
         typedef GenericCurrentStruct T;
         return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetCurrent(ret); });
     }
     case Target::Id: {
         typedef GenericTargetStruct T;
         return EncodeRead<T>(aEncoder,
                              [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetTarget(ret); });
     }
     case Resolution::Id: {
         typedef Resolution::TypeInfo::Type T;
         return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetResolution(ret); });
     }
     case StepValue::Id: {
         typedef StepValue::TypeInfo::Type T;
         return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetStepValue(ret); });
     }
     case Unit::Id: {
         typedef Unit::TypeInfo::Type T;
         return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetUnit(ret); });
     }
     case UnitRange::Id: {
         typedef UnitRange::TypeInfo::Type T;
         return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetUnitRange(ret); });
     }
     case LimitRange::Id: {
         typedef LimitRange::TypeInfo::Type T;
         return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetLimitRange(ret); });
     }
     case TranslationDirection::Id: {
         typedef TranslationDirection::TypeInfo::Type T;
         return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetTranslationDirection(ret); });
     }
     case RotationAxis::Id: {
         typedef RotationAxis::TypeInfo::Type T;
         return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetRotationAxis(ret); });
     }
     case Overflow::Id: {
         typedef Overflow::TypeInfo::Type T;
         return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetOverflow(ret); });
     }
     case ModulationType::Id: {
         typedef ModulationType::TypeInfo::Type T;
         return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetModulationType(ret); });
     }
     case FeatureMap::Id: {
         typedef FeatureMap::TypeInfo::Type T;
         return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetFeatureMap(ret); });
     }
     case ClusterRevision::Id: {
         typedef ClusterRevision::TypeInfo::Type T;
         return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetClusterRevision(ret); });
     }
     default:
         return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
     }
 }
 
 CHIP_ERROR Interface::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
 {
     switch (aPath.mAttributeId)
     {
     default:
         return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
     }
 }
 
 // CommandHandlerInterface
 void Interface::InvokeCommand(HandlerContext & handlerContext)
 {
     switch (handlerContext.mRequestPath.mCommandId)
     {
     case SetTarget::Id:
         HandleCommand<SetTarget::DecodableType>(
             handlerContext, [&logic = mClusterLogic](HandlerContext & ctx, const auto & commandData) {
                 CHIP_ERROR err =
                     logic.HandleSetTargetCommand(commandData.position.std_optional(), commandData.latch.std_optional(), commandData.speed.std_optional());
                 Status status = Status::Success;
                 if (err == CHIP_ERROR_INVALID_ARGUMENT)
                 {
                     status = Status::ConstraintError;
                 }
                 if (err != CHIP_NO_ERROR)
                 {
                     status = Status::Failure;
                 }
                 ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
             });
         return;
     case Step::Id:
         HandleCommand<Step::DecodableType>(handlerContext,
                                             [&logic = mClusterLogic](HandlerContext & ctx, const auto & commandData) {
                                                 CHIP_ERROR err = logic.HandleStepCommand(commandData.direction, commandData.numberOfSteps, commandData.speed.std_optional());
                                                 Status status  = Status::Success;
                                                 if (err == CHIP_ERROR_INVALID_ARGUMENT)
                                                 {
                                                     status = Status::ConstraintError;
                                                 }
                                                 if (err != CHIP_NO_ERROR)
                                                 {
                                                     status = Status::Failure;
                                                 }
                                                 ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
                                             });
         return;
     }
 }
 
 CHIP_ERROR Interface::Init()
 {
     AttributeAccessInterfaceRegistry::Instance().Register(this);
     CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this);
     return CHIP_NO_ERROR;
 }
 
 } // namespace ClosureDimension
 } // namespace Clusters
 } // namespace app
 } // namespace chip
 