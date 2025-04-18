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

#include "chef-operational-state-delegate-impl.h"

#ifdef MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER

namespace chef {
namespace OvenCavityOperationalState {

class Delegate : public chip::app::Clusters::OperationalState::OperationalStateDelegate
{
private:
    const CharSpan kPhaseList[3] = { "pre-heating"_span, "pre-heated"_span, "cooling down"_span };

public:
    Delegate() { GenericOperationalStateDelegateImpl::mOperationalPhaseList = Span<const CharSpan>(kPhaseList); }
};

void InitChefOvenCavityOperationalStateCluster();

} // namespace OvenCavityOperationalState
} // namespace chef

#endif // MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER
