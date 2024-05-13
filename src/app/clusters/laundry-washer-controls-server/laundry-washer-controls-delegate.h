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
     * Get the spin speed string at the given index in the list.
     * @param index The index of the spin speed, with 0 representing the first one.
     * @param spinSpeed The MutableCharSpan to copy the string data into.  On success, the callee must update
     *        the length to the length of the copied data.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the list of spin speeds.
     */
    virtual CHIP_ERROR GetSpinSpeedAtIndex(size_t index, MutableCharSpan & spinSpeed) = 0;

    /**
     * Get the supported rinses value at the given index in the list.
     * @param index The index of the supported rinses with 0 representing the first one.
     * @param supportedRinse The supported rinse at the given index
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the list of supported rinses.
     */
    virtual CHIP_ERROR GetSupportedRinseAtIndex(size_t index, NumberOfRinsesEnum & supportedRinse) = 0;
};

} // namespace LaundryWasherControls
} // namespace Clusters
} // namespace app
} // namespace chip
