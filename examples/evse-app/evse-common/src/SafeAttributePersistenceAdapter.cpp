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

#include <SafeAttributePersistenceAdapter.h>

#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {

namespace {

SafeAttributePersistenceAdapter sAdapter;

} // namespace

SafeAttributePersistenceAdapter & GetSafeAttributePersistenceAdapter()
{
    return sAdapter;
}

CHIP_ERROR SafeAttributePersistenceAdapter::WriteValue(const ConcreteAttributePath & aPath, const ByteSpan & aValue)
{
    SafeAttributePersistenceProvider * provider = GetSafeAttributePersistenceProvider();
    VerifyOrReturnError(provider != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return provider->SafeWriteValue(aPath, aValue);
}

CHIP_ERROR SafeAttributePersistenceAdapter::ReadValue(const ConcreteAttributePath & aPath, MutableByteSpan & aValue)
{
    SafeAttributePersistenceProvider * provider = GetSafeAttributePersistenceProvider();
    VerifyOrReturnError(provider != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return provider->SafeReadValue(aPath, aValue);
}

} // namespace app
} // namespace chip
