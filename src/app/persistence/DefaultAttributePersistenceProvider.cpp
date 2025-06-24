/*
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
#include <app/persistence/DefaultAttributePersistenceProvider.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace app {
namespace {

constexpr uint8_t ShortPascalStringLength(const uint8_t * buffer)
{
    // The first byte specifies the length of the string.  A length of 0xFF means
    // the string is invalid and there is no character data.
    return (buffer[0] == 0xFF ? 0 : buffer[0]);
}

uint16_t LongPascalStringLength(const uint8_t * buffer)
{
    // The first two bytes specify the length of the long string.  A length of
    // 0xFFFF means the string is invalid and there is no character data.
    uint16_t length = Encoding::LittleEndian::Get16(buffer);
    return (length == 0xFFFF ? 0 : length);
}

} // namespace

CHIP_ERROR DefaultAttributePersistenceProvider::WriteValue(const ConcreteAttributePath & aPath,
                                                           const AttributeValueInformation & aInfo, const ByteSpan & aValue)
{
    return StorageDelegateWrapper::WriteValue(
        DefaultStorageKeyAllocator::AttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId), aValue);
}

CHIP_ERROR DefaultAttributePersistenceProvider::ReadValue(const ConcreteAttributePath & aPath,
                                                          const AttributeValueInformation & aInfo, MutableByteSpan & aValue)
{
    ReturnErrorOnFailure(StorageDelegateWrapper::ReadValue(
        DefaultStorageKeyAllocator::AttributeValue(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId), aValue));

    const size_t size = aValue.size();
    switch (aInfo.type)
    {
    case AttributeValueType::kShortPascal:
        // Ensure that we've read enough bytes that we are not ending up with
        // un-initialized memory.  Should have read length + 1 (for the length
        // byte).
        VerifyOrReturnError(size >= 1 && size - 1 >= ShortPascalStringLength(aValue.data()), CHIP_ERROR_INCORRECT_STATE);
        break;
    case AttributeValueType::kLongPascal:
        // Ensure that we've read enough bytes that we are not ending up with
        // un-initialized memory.  Should have read length + 2 (for the length
        // bytes).
        VerifyOrReturnError(size >= 2 && size - 2 >= LongPascalStringLength(aValue.data()), CHIP_ERROR_INCORRECT_STATE);
        break;
    case AttributeValueType::kFixedSize:
        // expect a specific size
        VerifyOrReturnError(size == aInfo.size, CHIP_ERROR_INVALID_ARGUMENT);
        break;
    case AttributeValueType::kVariableSize:
        // variable size fixed every time
        break;
    }
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
