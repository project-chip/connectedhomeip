/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryWasherControls {

/** @brief
 *    Defines methods for implementing application-specific logic for the laundry washer controls cluster.
 */
class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    /**
     * Get the list of supported spin_speed list.
     * Fills in the provided spin_speed at index `index` if there is one,
     * or returns CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the list of spin_speed.
     * @param index The index of the spin_speed, with 0 representing the first one.
     * @param spinSpeed  The spin speed is filled.
     */
    virtual CHIP_ERROR GetSpinSpeedAtIndex(size_t index, MutableCharSpan & spinSpeed) = 0;

    /**
     * Get the list of supported rinses list.
     * Fills in the provided rinses  at index `index` if there is one,
     * or returns CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the list of rinses.
     * @param index The index of the supported rinses with 0 representing the first one.
     * @param supportedRinse  The supported rinse is filled.
     */
    virtual CHIP_ERROR GetSupportedRinseAtIndex(size_t index, NumberOfRinsesEnum & supportedRinse) = 0;
};

} // namespace LaundryWasherControls
} // namespace Clusters
} // namespace app
} // namespace chip
