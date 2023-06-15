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
#include <app/CommandHandlerInterface.h>
#include <app/util/util.h>
#include <utility>

using chip::Protocols::InteractionModel::Status;
using ModeOptionStructType  = chip::app::Clusters::ModeSelect::Structs::ModeOptionStruct::Type;
using SemanticTagStructType = chip::app::Clusters::ModeSelect::Structs::SemanticTagStruct::Type;

template <typename T>
using List = chip::app::DataModel::List<T>;

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

class Delegate
{
public:
    /**
     * This is a helper function to build a mode option structure. It takes the label/name of the mode,
     * the value of the mode and a list of semantic tags that apply to this mode.
     */
    static ModeOptionStructType BuildModeOptionStruct(const char * label, uint8_t mode,
                                                      const List<const SemanticTagStructType> semanticTags)
    {
        Structs::ModeOptionStruct::Type option;
        option.label        = CharSpan::fromCharString(label);
        option.mode         = mode;
        option.semanticTags = semanticTags;
        return option;
    }

    explicit Delegate() = default;

    virtual CHIP_ERROR Init() = 0;

    /**
     * Returns the number of modes provided and managed by the delegate.
     */
    virtual uint8_t NumberOfModes();

    /**
     * This function returns true if the mode value given matches one of the supported modes, otherwise it returns false.
     *
     * @param mode
     */
    bool IsSupportedMode(uint8_t mode);

    /**
     * When a ChangeToMode command is received, if the NewMode value is a supported made, this function is called to decide if we
     * should go ahead with transitioning to this mode. If this function returns a success status, the change request is accepted
     * and the CurrentMode is set to the NewMode. Else, the CurrentMode is left untouched and the returned status in added to the
     * CommandHandler.
     *
     * This function is to be overridden by a user implemented function that makes this decision based on the application logic.
     * @param mode
     */
    virtual Status HandleChangeToMode(uint8_t mode);

    /**
     * When a ChangeToModeWithStatus command is received, if the NewMode value is a supported made, this function is called to
     * 1) decide if we should go ahead with transitioning to this mode and 2) formulate the ChangeToModeResponse that will be
     * sent back to the client. If this function returns a response.status of ChangeToModeResponseStatus success, the change
     * request is accepted and the CurrentMode is set to the NewMode. Else, the CurrentMode is left untouched. The response is
     * sent as a ChangeToModeResponse command.
     *
     * This function is to be overridden by a user implemented function that makes this decision based on the application logic.
     * @param mode
     * @param response
     */
    virtual void HandleChangeToModeWitheStatus(uint8_t mode, ModeSelect::Commands::ChangeToModeResponse::Type & response);

    virtual ~Delegate() = default;

    /**
     * Get the mode label of the Nth mode in the list of modes. This is mostly useful for SDK code.
     * @param modeIndex The index in the list of modes of the mode to be returned.
     * @param found is set to true if a mode is found. If set to false, the return should be ignored.
     * @return the mode label of the mode at modeIndex.
     */
    virtual CHIP_ERROR getModeLabelByIndex(uint8_t modeIndex, MutableCharSpan &label);

    /**
     * Get the mode value of the Nth mode in the list of modes. This is mostly useful for SDK code.
     * @param modeIndex The index in the list of modes of the mode to be returned.
     * @param found is set to true if a mode is found. If set to false, the return should be ignored.
     * @return the mode value of the mode at modeIndex.
     */
    virtual CHIP_ERROR getModeValueByIndex(uint8_t modeIndex, uint8_t &value);

    /**
     * Get the mode tags of the Nth mode in the list of modes. This is mostly useful for SDK code.
     * The caller must make sure the List points to an existing buffer of sufficient size to hold the spec-required number of tags, and the size of the List is the size of the buffer.
     * 
     * The implementation must place its desired SemanticTagStructType instances in that buffer and call reduce_size
     * on the list to indicate how many entries were initialized.
     * @param modeIndex The index in the list of modes of the mode to be returned.
     * @param found is set to true if a mode is found. If set to false, the return should be ignored.
     * @return a list of the mode tags of the mode at modeIndex.
     */
    virtual CHIP_ERROR getModeTagsByIndex(uint8_t modeIndex, List<SemanticTagStructType> &tags);
};

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
