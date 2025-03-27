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
 */

#pragma once

#include "PresetStructWithOwnedMembers.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

/** @brief
 *  Defines methods for implementing application-specific logic for handling Presets in the thermostat cluster.
 *  It defines the interfaces that a thermostat should implement to enable support for reading and writing the
 *  Presets attribute and reading and writing the ActivePresetHandle attribute.
 */
class Delegate
{
public:
    Delegate() = default;

    virtual ~Delegate() = default;

    /**
     * @brief Get the maximum timeout for atomically writing to an attribute
     *
     * @param[in] attributeId The attribute to write to.
     * @return The maximum allowed timeout; nullopt if the request is invalid.
     */
    virtual std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) = 0;

    /**
     * @brief Get the preset type at a given index in the PresetTypes attribute
     *
     * @param[in] index The index of the preset type in the list.
     * @param[out] presetType The preset type  at the given index in the list.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the preset types list.
     */
    virtual CHIP_ERROR GetPresetTypeAtIndex(size_t index, Structs::PresetTypeStruct::Type & presetType) = 0;

    /**
     * @brief Get the NumberOfPresets attribute value.
     *
     * @return The max number of  presets supported. Return 0 if not set.
     */
    virtual uint8_t GetNumberOfPresets() = 0;

    /**
     * @brief Get the preset at a given index in the Presets attribute.
     *
     * @param[in] index The index of the preset in the list.
     * @param[out] preset The PresetStructWithOwnedMembers struct that has the data from the preset
     *             at the given index in the Presets attribute list.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the presets list.
     */
    virtual CHIP_ERROR GetPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset) = 0;

    /**
     * @brief Get the ActivePresetHandle attribute value.
     *
     * @param[out] activePresetHandle The nullable MutableByteSpan to copy the active preset handle into. On success,
     *             the size of the activePresetHandle is updated to the length of the copied data.
     */
    virtual CHIP_ERROR GetActivePresetHandle(DataModel::Nullable<MutableByteSpan> & activePresetHandle) = 0;

    /**
     * @brief Set the ActivePresetHandle attribute value.
     *
     * @param[in] newActivePresetHandle The octet string to set the active preset handle to.
     */
    virtual CHIP_ERROR SetActivePresetHandle(const DataModel::Nullable<ByteSpan> & newActivePresetHandle) = 0;

    /**
     * @brief Copies existing presets to the pending preset list
     */
    virtual void InitializePendingPresets() = 0;

    /**
     * @brief Appends a preset to the pending presets list maintained by the delegate.
     *        The delegate must ensure it makes a copy of the provided preset and the data
     *        of its preset handle, if any.  For example, it could create a PresetStructWithOwnedMembers
     *        from the provided preset.
     *
     * @param[in] preset The preset to add to the list.
     *
     * @return CHIP_NO_ERROR if the preset was appended to the list successfully.
     * @return CHIP_ERROR if there was an error adding the preset to the list.
     */
    virtual CHIP_ERROR AppendToPendingPresetList(const PresetStructWithOwnedMembers & preset) = 0;

    /**
     * @brief Get the Preset at a given index in the pending presets list.
     *
     * @param[in] index The index of the preset in the list.
     * @param[out] preset The PresetStructWithOwnedMembers struct that has the data from the pending preset
     *             list at the given index.
     * @return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is out of range for the pending presets list.
     */
    virtual CHIP_ERROR GetPendingPresetAtIndex(size_t index, PresetStructWithOwnedMembers & preset) = 0;

    /**
     * @brief Updates the presets attribute with the content of the pending presets list. If the preset in the pending presets list
     * matches i.e. has the same presetHandle as an existing entry in the Presets attribute, the thermostat will update the entry
     * with the new preset values, otherwise it will add a new preset to the Presets attribute. For new presets that get added,
     * it is the responsibility of this API to allocate unique preset handles to the presets before saving the preset. This will be
     * called when the Thermostat receives a AtomicRequest command of type CommitWrite to commit the pending preset changes.
     *
     * @return CHIP_NO_ERROR if the updates to the presets attribute has been committed successfully.
     * @return CHIP_ERROR if the updates to the presets attribute failed to commit for some reason.
     *
     */
    virtual CHIP_ERROR CommitPendingPresets() = 0;

    /**
     * @brief Clears the pending presets list.
     *
     */
    virtual void ClearPendingPresetList() = 0;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
