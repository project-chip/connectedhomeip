/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
