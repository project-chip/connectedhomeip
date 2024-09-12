/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include "service-area-cluster-objects.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ServiceArea {

class Instance;

/**
 * ServiceArea::Delegate Defines methods for implementing application-specific
 * logic for the Service Area Cluster.
 */
class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    /**
     * Due to the coupling between the Delegate and Instance classes via the references they have to each other,
     * copying a Delegate object might make things confusing.
     */
    Delegate(const Delegate &)             = delete;
    Delegate & operator=(const Delegate &) = delete;

    /**
     * @brief This method will be called during the ServiceArea server initialization after the Instance information has been
     * validated and the Instance has been registered. This can be used to initialise app logic.
     */
    virtual CHIP_ERROR Init() { return CHIP_NO_ERROR; };

    /**
     * @brief Can the selected locations be set by the client in the current operating mode?
     * @param[out] statusText text describing why the selected locations cannot be set (if return is false).
     * Max size kMaxSizeStatusText.
     * Note: statusText must be successfully set if the return is false. Use CopyCharSpanToMutableCharSpanWithTruncation to
     * ensure that a message is copied successfully. Otherwise, ensure that if setting the statusText can fail (e.g., due
     * to exceeding kMaxSizeStatusText) the size of this value is set to 0 with .reduce_size(0) to avoid callers using
     * un-initialized memory.
     * @return true if the current device state allows selected locations to be set by user.
     *
     * @note The statusText field SHOULD indicate why the request is not allowed, given the current mode
     *       of the device, which may involve other clusters.
     */
    virtual bool IsSetSelectedAreasAllowed(MutableCharSpan & statusText) = 0;

    /**
     * Given a set of locations to be set to the SelectedAreas attribute, this method should check that
     * the set of locations as a whole is valid and reachable by the device.
     * If the set of locations is invalid, the locationStatus should be set to InvalidSet and
     * the statusText SHALL include a vendor-defined error description.
     *
     * The caller of this method will ensure that there are no duplicates in the list
     * and that all the locations in the set are valid supported locations.
     *
     * @param[in] selectedAreas List of new selected locations.
     * @param[out] locationStatus Success if all checks pass, error code if failure.
     * @param[out] statusText text describing failure (see description above). Max size kMaxSizeStatusText.
     * Note: statusText must be successfully set if the return is false. Use CopyCharSpanToMutableCharSpanWithTruncation to
     * ensure that a message is copied successfully. Otherwise, ensure that if setting the statusText can fail (e.g., due
     * to exceeding kMaxSizeStatusText) the size of this value is set to 0 with .reduce_size(0) to avoid callers using
     * un-initialized memory.
     * @return true if success.
     *
     * @note If the SelectAreas command is allowed when the device is operating and the selected locations change to none, the
     * device must stop.
     */
    virtual bool IsValidSelectAreasSet(const Span<const uint32_t> & selectedAreas, SelectAreasStatus & locationStatus,
                                       MutableCharSpan & statusText) = 0;

    /**
     * @brief The server instance ensures that the SelectedAreas and CurrentArea attributes are not null before
     * calling this method.
     * @param[in] skippedArea the area ID to skip.
     * @param[out] skipStatusText text describing why the current location cannot be skipped. Max size kMaxSizeStatusText.
     * Note: skipStatusText must be successfully set if the return is false. Use CopyCharSpanToMutableCharSpanWithTruncation to
     * ensure that a message is copied successfully. Otherwise, ensure that if setting the skipStatusText can fail (e.g., due
     * to exceeding kMaxSizeStatusText) the size of this value is set to 0 with .reduce_size(0) to avoid callers using
     * un-initialized memory.
     * @return true if command is successful, false if the received skip request cannot be handled due to the current mode of the
     * device.
     *
     * @note skipStatusText must be filled out by the function on failure.
     *
     * @note If the device accepts the request:
     * - If the device is currently operating at the area identified by SkippedArea, as indicated by either the CurrentArea or
     *     the Progress attributes, if implemented, the device SHALL stop operating at that area.
     * - If the Progress attribute is implemented, the entry corresponding to SkippedArea SHALL be updated to indicate that the
     *     area was skipped.
     * - The server SHALL attempt to operate only at the areas in the SelectedAreas attribute list where operating has not been
     *     skipped or completed, using a vendor defined order.
     * - If the server has either skipped or completed operating at all areas on the SelectedAreas attribute list, the server
     *     SHALL stop operating.
     *
     * @note If the Status field is set to InvalidAreaList, the StatusText field SHALL be an empty string.
     *       If the Status field is not set to Success, or InvalidAreaList, the StatusText field SHALL include a vendor defined
     *       error description which can be used to explain the error to the user. For example, if the Status field is set to
     *       InvalidInMode, the StatusText field SHOULD indicate why the request is not allowed, given the current mode of the
     *       device, which may involve other clusters.
     */
    virtual bool HandleSkipArea(uint32_t skippedArea, MutableCharSpan & skipStatusText)
    {
        // device support of this command is optional
        CopyCharSpanToMutableCharSpanWithTruncation("Skip Current Area command not supported by device"_span, skipStatusText);
        return false;
    }

    /**
     * @return true if the current device state allows the SupportedAreas attribute to be updated.
     *
     * @note The SupportedAreas attribute list changes (adding or deleting entries,
     *       changing their MapID fields, changing the AreaID fields, or nulling the entire list)
     *       SHOULD NOT be allowed while the device is operating, to reduce the impact on the clients,
     *       and the potential confusion for the users.
     *
     * @note The device implementation MAY allow supported location changes while operating if the device
     *       repopulates the SupportedMaps, SupportedAreas, CurrentArea, and Progress attributes with
     *       data matching the constraints listed in the requirements for each attribute.
     */
    virtual bool IsSupportedAreasChangeAllowed() = 0;

    /**
     * @return true if the current device state allows the SupportedMaps attribute to be updated.
     *
     * @note The SupportedMaps attribute list changes (adding or deleting entries,
     *       changing their MapID fields, or nulling the entire list)
     *       SHOULD NOT be allowed while the device is operating, to reduce the impact on the clients,
     *       and the potential confusion for the users.
     *
     * @note The device implementation MAY allow supported maps changes while operating if the device
     *       repopulates the SupportedAreas, CurrentArea, and Progress attributes with
     *       data matching the constraints listed in the requirements for each attribute.
     */
    virtual bool IsSupportedMapChangeAllowed() = 0;

    Instance * GetInstance() { return mInstance; }

    void SetInstance(Instance * aInstance) { mInstance = aInstance; }

private:
    Instance * mInstance = nullptr;
};

} // namespace ServiceArea
} // namespace Clusters
} // namespace app
} // namespace chip
