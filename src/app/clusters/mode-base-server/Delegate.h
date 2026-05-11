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

#pragma once

#include <app/clusters/mode-base-server/mode-base-cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeBase {

class Instance;

class Delegate
{
public:
    Delegate() = default;

    virtual ~Delegate() = default;

    // The following functions should be overridden by the SDK user to implement the business logic of their application.
    /**
     * This init function will be called during the ModeBase server initialization after the Instance information has been
     * validated and the Instance has been registered. This can be used to initialise app logic.
     */
    virtual CHIP_ERROR Init() = 0;

    /**
     * Get the mode label of the Nth mode in the list of modes.
     * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
     * @param label A reference to the mutable char span which will be mutated to receive the label on success. Use
     * CopyCharSpanToMutableCharSpan to copy into the MutableCharSpan.
     * @return Returns a CHIP_NO_ERROR if there was no error and the label was returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the modeIndex in beyond the list of available labels.
     *
     * Note: This is used by the SDK to populate the supported modes attribute. If the contents of this list change,
     * the device SHALL call the Instance's ReportSupportedModesChange method to report that this attribute has changed.
     */
    virtual CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) = 0;

    /**
     * Get the mode value of the Nth mode in the list of modes.
     * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
     * @param value a reference to the uint8_t variable that is to contain the mode value.
     * @return Returns a CHIP_NO_ERROR if there was no error and the value was returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the modeIndex in beyond the list of available values.
     *
     * Note: This is used by the SDK to populate the supported modes attribute. If the contents of this list change,
     * the device SHALL call the Instance's ReportSupportedModesChange method to report that this attribute has changed.
     */
    virtual CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) = 0;

    /**
     * Get the mode tags of the Nth mode in the list of modes.
     * The caller will make sure the List points to an existing buffer of sufficient size to hold the spec-required number
     * of tags, and the size of the List is the size of the buffer.
     *
     * The implementation must place its desired ModeTagStructType instances in that buffer and call tags.reduce_size
     * on the list to indicate how many entries were initialized.
     * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
     * @param tags a reference to an existing and initialised buffer that is to contain the mode tags. std::copy can be used
     * to copy into the buffer.
     * @return Returns a CHIP_NO_ERROR if there was no error and the mode tags were returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the modeIndex in beyond the list of available mode tags.
     *
     * Note: This is used by the SDK to populate the supported modes attribute. If the contents of this list change,
     * the device SHALL call the Instance's ReportSupportedModesChange method to report that this attribute has changed.
     */
    virtual CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<detail::Structs::ModeTagStruct::Type> & modeTags) = 0;

    /**
     * When a ChangeToMode command is received, if the NewMode value is a supported mode, this method is called to 1) decide if
     * we should go ahead with transitioning to this mode and 2) formulate the ChangeToModeResponse that will be sent back to the
     * client. If this function returns a response.status of StatusCode::kSuccess, the change request is accepted
     * and the CurrentMode is set to the NewMode. Else, the CurrentMode is left untouched. The response is sent as a
     * ChangeToModeResponse command.
     *
     * This function is to be overridden by a user implemented function that makes this decision based on the application logic.
     * @param NewMode The new made that the device is requested to transition to.
     * @param response A reference to a response that will be sent to the client. The contents of which con be modified by the
     * application.
     *
     */
    virtual void HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type & response) = 0;

    /**
     * This method is used by the SDK to set the instance pointer. This is done during the instantiation of an Instance object.
     * @param aInstance A pointer to the Instance object related to this delegate object.
     */
    void SetInstance(Instance * aInstance) { mInstance = aInstance; }

private:
    Instance * mInstance = nullptr;

protected:
    /**
     * @brief Provides access to the const Instance pointer.
     *
     * @return A const pointer to the Instance object associated with this delegate.
     */
    const Instance * GetInstance() const { return mInstance; }

    /**
     * @brief Provides access to the Instance pointer.
     *
     * @return A pointer to the Instance object associated with this delegate.
     */
    Instance * GetInstance() { return mInstance; }
};

} // namespace ModeBase
} // namespace Clusters
} // namespace app
} // namespace chip
