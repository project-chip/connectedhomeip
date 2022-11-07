/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        https://urldefense.com/v3/__http://www.apache.org/licenses/LICENSE-2.0__;!!N30Cs7Jr!UgbMbEQ59BIK-1Xslc7QXYm0lQBh92qA3ElecRe1CF_9YhXxbwPOZa6j4plru7B7kCJ7bKQgHxgQrket3-Dnk268sIdA7Qb8$
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "ExtensionFieldsSets.h"

namespace chip {
namespace scenes {

ExtensionFieldsSets::ExtensionFieldsSets()
{
    // check if any of the clusters with scene attributes are enabled
    if (this->kExentesionFieldsSetsSize == 1)
    {
        // a size of 1 byte indicates an empty struct, or a struct that only contains :
        // the on-off cluster
        // the Mode select cluster
        // the door lock cluster
#ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
        this->enabledFieldSets.onOff = false;

#elif defined(ZCL_USING_MODE_SELECT_CLUSTER_SERVER)
        this->enabledFieldSets.currentMode = 0;
#elif defined(ZCL_USING_DOOR_LOCK_CLUSTER_SERVER)
        this->enabledFieldSets.lockState = 0;
#else
        this->empty = true;
#endif
    }
    else if (this->kExentesionFieldsSetsSize > 1)
    {
        memset(&this->enabledFieldSets, 0, kExentesionFieldsSetsSize);
    }
}

CHIP_ERROR ExtensionFieldsSets::Serialize(TLV::TLVWriter & writer) const
{
    if (!this->empty)
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(
            writer.PutBytes(TagEnabledFielsSets(), (uint8_t *) &this->enabledFieldSets, kExentesionFieldsSetsSize));

        return writer.EndContainer(container);
    }
    else
    {
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR ExtensionFieldsSets::Deserialize(TLV::TLVReader & reader)
{
    if (!this->empty)
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(1)));
        ReturnErrorOnFailure(reader.EnterContainer(container));

        ReturnErrorOnFailure(reader.Next(TagEnabledFielsSets()));
        ReturnErrorOnFailure(reader.GetBytes((uint8_t *) &this->enabledFieldSets, kExentesionFieldsSetsSize));

        return reader.ExitContainer(container);
    }
    else
    {
        return CHIP_NO_ERROR;
    }
}
void ExtensionFieldsSets::clear()
{
    if (!this->empty)
    {
        memset(&this->enabledFieldSets, 0, sizeof(this->enabledFieldSets));
    }
}

} // namespace scenes

} // namespace chip