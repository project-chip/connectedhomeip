/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#ifndef __STM32HAL_H__
#define __STM32HAL_H__

#include "stm32wbaxx.h"
#include "stm32wbaxx_hal.h"
#include "stm32wbaxx_hal_crc.h"

<<<<<<< HEAD:examples/platform/stm32/STM32WBA65I-DK1/Inc/stm32_hal.h
#endif /*__STM32HAL_H__*/
    == == ==
    =
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>

        namespace chip::app::Clusters::PowerTopology
{

    /// Create and register the Power Topology cluster on `endpointId` with `features`.
    /// The app instantiates this cluster on more than one endpoint, and the two carry different
    /// feature sets, so the caller chooses. Call from ApplicationInit().
    CHIP_ERROR PowerTopologyInit(EndpointId endpointId, BitMask<Feature> features);

    /// Unregister and destroy every instance. Call from ApplicationShutdown().
    void PowerTopologyShutdown();

} // namespace chip::app::Clusters::PowerTopology
>>>>>>> aed3a54 ([examples] Add an Electrical Circuit Breaker endpoint to electrical-protection-app (#73815)):examples/electrical-protection-app/electrical-protection-common/include/power-topology-stub.h
