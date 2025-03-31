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

 #include <app-common/zap-generated/cluster-enums.h>
 #include <app/data-model/Nullable.h>
 #include <app/util/basic-types.h>
 #include <lib/core/CHIPError.h>
 
 namespace chip {
 namespace app {
 namespace Clusters {
 namespace ClosureDimension {
 
 /** @brief
  *    Defines methods for implementing application-specific logic for the Closure Dimension Cluster.
  */
 
 class DelegateBase
 {
 public:
    DelegateBase(){};
    virtual ~DelegateBase() = default;
     
    virtual CHIP_ERROR HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<TargetLatchEnum> & latch,
        const Optional<Globals::ThreeLevelAutoEnum> & speed) = 0;
 
    virtual CHIP_ERROR HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
        const Optional<Globals::ThreeLevelAutoEnum> & speed) = 0;
 };
 
 } // namespace ClosureDimension
 } // namespace Clusters
 } // namespace app
 } // namespace chip
 