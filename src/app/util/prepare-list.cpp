/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "prepare-list.h"

EmberAfStatus PrepareListFromTLV(chip::TLV::TLVReader * tlvData, const uint8_t *& message, uint16_t & messageLen)
{
    CHIP_ERROR tlvError = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;
    chip::TLV::TLVType type;
    reader.Init(*tlvData);
    reader.EnterContainer(type);
    tlvError = reader.Next();
    if (tlvError != CHIP_NO_ERROR && tlvError != CHIP_END_OF_TLV && chip::CanCastTo<uint16_t>(reader.GetLength()))
    {
        return EMBER_ZCL_STATUS_INVALID_VALUE;
    }
    if (tlvError == CHIP_NO_ERROR)
    {
        tlvError   = reader.GetDataPtr(message);
        messageLen = static_cast<uint16_t>(reader.GetLength());
    }
    if (tlvError != CHIP_NO_ERROR)
    {
        return EMBER_ZCL_STATUS_INVALID_VALUE;
    }
    reader.ExitContainer(type);
    return EMBER_ZCL_STATUS_SUCCESS;
}
