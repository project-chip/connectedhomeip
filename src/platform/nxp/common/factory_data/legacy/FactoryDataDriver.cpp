/*
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

#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <platform/nxp/common/factory_data/legacy/FactoryDataDriver.h>
#include <platform/nxp/common/factory_data/legacy/FactoryDataProvider.h>

namespace chip {
namespace DeviceLayer {

FactoryDataDriver::~FactoryDataDriver() {}

CHIP_ERROR FactoryDataDriver::UpdateValueInRam(uint8_t tag, ByteSpan & newValue)
{
    uint16_t oldLength                   = 0;
    uint16_t newLength                   = newValue.size();
    uint32_t offset                      = 0;
    uint8_t * factoryData                = mFactoryDataRamBuff;
    FactoryDataProvider::Header * header = (FactoryDataProvider::Header *) factoryData;
    uint8_t * data                       = factoryData + sizeof(FactoryDataProvider::Header);

    while (offset < header->size)
    {
        memcpy(&oldLength, &data[offset + FactoryDataProvider::kLengthOffset], sizeof(oldLength));

        if (tag != data[offset])
        {
            offset += FactoryDataProvider::kValueOffset + oldLength;
            continue;
        }

        if (oldLength == newLength)
        {
            memcpy(&data[offset + FactoryDataProvider::kValueOffset], newValue.data(), newLength);
        }
        else
        {
            uint32_t oldEndOffset = offset + FactoryDataProvider::kValueOffset + oldLength;

            memcpy(&data[offset + FactoryDataProvider::kLengthOffset], &newLength, sizeof(newLength));
            memmove(&data[offset + FactoryDataProvider::kValueOffset + newLength], &data[oldEndOffset],
                    header->size - oldEndOffset);
            memcpy(&data[offset + FactoryDataProvider::kValueOffset], newValue.data(), newLength);
        }

        header->size = header->size - oldLength + newLength;

        uint8_t sha256Output[SHA256_HASH_SIZE] = { 0 };
        SHA256_Hash(data, header->size, sha256Output);
        memcpy(header->hash, sha256Output, sizeof(header->hash));

        ChipLogProgress(DeviceLayer, "Value at tag %d updated successfully.", tag);
        return CHIP_NO_ERROR;
    }

    ChipLogError(DeviceLayer, "Failed to find tag %d.", tag);
    return CHIP_ERROR_NOT_FOUND;
}

} // namespace DeviceLayer
} // namespace chip
