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
using ModeOptionStructType  = chip::app::Clusters::detail::Structs::ModeOptionStruct::Type;
using SemanticTagStructType = chip::app::Clusters::detail::Structs::ModeTagStruct::Type;

template <typename T>
using List = chip::app::DataModel::List<T>;

namespace chip {
namespace app {
namespace Clusters {
namespace ModeBase {

class Delegate
{
public:
   /**
    * This is a helper function to build a mode option structure. It takes the label/name of the mode,
    * the value of the mode and a list of semantic tags that apply to this mode. NOTE, the caller must
    * ensure that the lifetime of the label and semanticTags is as long as the returned structure.
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

   /**
    * This init function will be called during the ModeBase server initialization after the Instance information has been validated
    * and the Instance has been registered. This can be used to initialise app logic.
    */
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
    * When a ChangeToMode command is received, if the NewMode value is a supported made, this function is called to 1) decide if
    * we should go ahead with transitioning to this mode and 2) formulate the ChangeToModeResponse that will be sent back to the
    * client. If this function returns a response.status of ChangeToModeResponseStatus success, the change request is accepted
    * and the CurrentMode is set to the NewMode. Else, the CurrentMode is left untouched. The response is sent as a
    * ChangeToModeResponse command.
    *
    * This function is to be overridden by a user implemented function that makes this decision based on the application logic.
    * @param NewMode The new made that the device is requested to transition to.
    * @param response A reference to a response that will be sent to the client. The contents of which con be modified by the
    * application.
    *
    * todo can the detail namespace work for commands? If not, should we manually create cluster objects for these in a ModeBase namespace?
    */
   virtual void HandleChangeToMode(uint8_t NewMode, detail::Commands::ChangeToModeResponse::Type & response);

   virtual ~Delegate() = default;

   /**
    * Get the mode label of the Nth mode in the list of modes.
    * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
    * @param label a reference to the MutableCharSpan that is to contain the mode label. Use CopyCharSpanToMutableCharSpan to copy
    * into the MutableCharSpan.
    * @return Returns a CHIP_NO_ERROR if there was no error.
    */
   virtual CHIP_ERROR getModeLabelByIndex(uint8_t modeIndex, MutableCharSpan &label);

   /**
    * Get the mode value of the Nth mode in the list of modes.
    * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
    * @param value a reference to the uint8_t variable that is to contain the mode value.
    * @return Returns a CHIP_NO_ERROR if there was no error.
    */
   virtual CHIP_ERROR getModeValueByIndex(uint8_t modeIndex, uint8_t &value);

   /**
    * Get the mode tags of the Nth mode in the list of modes.
    * The caller must make sure the List points to an existing buffer of sufficient size to hold the spec-required number
    * of tags, and the size of the List is the size of the buffer.
    *
    * The implementation must place its desired ModeTagStructType instances in that buffer and call tags.reduce_size
    * on the list to indicate how many entries were initialized.
    * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
    * @param tags a reference to an existing and initialised buffer that is to contain the mode tags. std::copy can be used
    * to copy into the buffer.
    * @return Returns a CHIP_NO_ERROR if there was no error.
    */
   virtual CHIP_ERROR getModeTagsByIndex(uint8_t modeIndex, List<SemanticTagStructType> &tags);
};

} // namespace ModeBase
} // namespace Clusters
} // namespace app
} // namespace chip
