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

#include <app/clusters/closure-dimension-server/closure-dimension-server.h>
#include <app/clusters/closure-dimension-server/closure-dimension-cluster-logic.h>
#include <app/clusters/closure-dimension-server/closure-dimension-delegate.h>
#include <app/clusters/closure-dimension-server/closure-dimension-cluster-objects.h>
#include <app/clusters/closure-dimension-server/closure-dimension-matter-context.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>
#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {
   
using Protocols::InteractionModel::Status;

/**
* @class ClosureDimensionDelegate
* @brief A delegate class that handles Closure Dimension commands at the application level.
* 
* This class is responsible for processing Closure Dimension commands such as Stop, MoveTo, and Calibrate
* according to specific business logic. It is designed to be used as a delegate for the Closure Dimension cluster.
* 
* @note This implementation is a "PrintOnly" delegate, which may primarily log or print command handling actions.
* 
* @param mEndpoint The endpoint ID associated with this delegate.
*/
class ClosureDimensionDelegate : public DelegateBase
{
public:

   ClosureDimensionDelegate(EndpointId endpoint) : mEndpoint(endpoint) 
   {}
   
   virtual ~ClosureDimensionDelegate() = default;


   // Override for the DelegateBase Virtual functions
   
   Status HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<bool> & latch,
   const Optional<Globals::ThreeLevelAutoEnum> & speed) override;
   Status HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
      const Optional<Globals::ThreeLevelAutoEnum> & speed) override;
      
   bool IsManualLatchingNeeded() override;

   /**
    * @brief Initializes the ClosureDimensionDelegate instance.
    *        Sets the Initial state of Closure.
    * 
    * @return CHIP_ERROR indicating the result of the initialization.
    */
   CHIP_ERROR Init();

   void SetLogic(ClusterLogic * logic) { mLogic = logic; }

   ClusterLogic * GetLogic() const { return mLogic; }

   StepDirectionEnum GetTargetDirection() const { return mTargetDirection; }

   void SetTargetDirection(StepDirectionEnum direction) { mTargetDirection = direction; }

private:
   EndpointId mEndpoint = kInvalidEndpointId;
   bool isMoving                      = false;
   bool isManualLatch                 = false;
   StepDirectionEnum mTargetDirection = StepDirectionEnum::kUnknownEnumValue;
   ClusterLogic * mLogic;
};

/**
* @class ClosureDimensionEndpoint
* @brief Represents a Closure Dimension cluster endpoint.
* 
* This class encapsulates the logic and interfaces required to manage a Closure Dimension cluster endpoint.
* It integrates the delegate, context, logic, and interface components for the endpoint.
* 
* @param mEndpoint The endpoint ID associated with this Closure Dimension endpoint.
* @param mContext The Matter context for the endpoint.
* @param mDelegate The delegate instance for handling commands.
* @param mLogic The cluster logic associated with the endpoint.
* @param mInterface The interface for interacting with the cluster.
*/
class ClosureDimensionEndpoint
{
public:

   ClosureDimensionEndpoint(EndpointId endpoint) :
       mEndpoint(endpoint), mContext(mEndpoint), mDelegate(mEndpoint), mLogic(mDelegate, mContext), mInterface(mEndpoint, mLogic)
   {
      mDelegate.SetLogic(&mLogic);
   }
   
   virtual ~ClosureDimensionEndpoint() = default;

   /**
    * @brief Initializes the ClosureDimensionEndpoint instance.
    * 
    * @return CHIP_ERROR indicating the result of the initialization.
    */
   CHIP_ERROR Init();

   /**
    * @brief Retrieves the delegate associated with this Closure Dimension endpoint.
    * 
    * @return Reference to the ClosureDimensionDelegate instance.
    */
   ClosureDimensionDelegate & GetDelegate() { return mDelegate; }
   ClusterLogic & GetLogic() { return mLogic; }

private:
   EndpointId mEndpoint = kInvalidEndpointId;
   MatterContext mContext; 
   ClosureDimensionDelegate mDelegate; 
   ClusterLogic mLogic; 
   Interface mInterface; 
};

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip