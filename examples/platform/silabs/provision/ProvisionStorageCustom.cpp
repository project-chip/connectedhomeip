/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <algorithm>
#include <headers/ProvisionStorage.h>
#include <lib/support/CodeUtils.h>
#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

namespace {
constexpr uint8_t kExample1 = 0x0001;
constexpr uint8_t kExample2 = 0x0002;
uint8_t sExample1           = 0;
uint8_t sExample2[32]       = { 0 };
} // namespace

CHIP_ERROR CustomStorage::Set(uint16_t id, const uint8_t * value)
{
    switch (id)
    {
    case kExample1:
        sExample1 = (nullptr == value) ? 0 : *value;
        break;

    default:
        return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CustomStorage::Get(uint16_t id, uint8_t & value)
{
    switch (id)
    {
    case kExample1:
        value = sExample1;
        break;

    default:
        return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CustomStorage::Set(uint16_t id, const uint16_t * value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR CustomStorage::Get(uint16_t id, uint16_t & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR CustomStorage::Set(uint16_t id, const uint32_t * value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR CustomStorage::Get(uint16_t id, uint32_t & value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR CustomStorage::Set(uint16_t id, const uint64_t * value)
{
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR CustomStorage::Get(uint16_t id, uint64_t & value)
{
    return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
}

CHIP_ERROR CustomStorage::Set(uint16_t id, const uint8_t * value, size_t size)
{
    switch (id)
    {
    case kExample2:
        memset(sExample2, 0x00, sizeof(sExample2));
        if (nullptr != value)
        {
            memcpy(sExample2, value, std::min(size, sizeof(sExample2)));
        }
        break;

    default:
        return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CustomStorage::Get(uint16_t id, uint8_t * value, size_t max_size, size_t & size)
{
    VerifyOrReturnError((nullptr != value) && (max_size > 0), CHIP_ERROR_INVALID_ARGUMENT);
    switch (id)
    {
    case kExample2:
        size = std::min(max_size, sizeof(sExample2));
        memcpy(value, sExample2, size);
        break;

    default:
        return CHIP_ERROR_UNKNOWN_RESOURCE_ID;
    }
    return CHIP_NO_ERROR;
}

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
