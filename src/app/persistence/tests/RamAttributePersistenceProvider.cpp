/*
 *    Copyr2025 Project CHIP Authors
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
#include <app/persistence/tests/RamAttributePersistenceProvider.h>

#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>

namespace chip::Testing {

CHIP_ERROR RamAttributePersistenceProvider::WriteValue(const app::ConcreteAttributePath & aPath, const ByteSpan & aValue)
{
    auto it = std::find_if(mEntries.begin(), mEntries.end(), [&aPath](const Entry & entry) { return entry.path == aPath; });

    if (it != mEntries.end())
    {
        // Found an existing entry, update it.
        it->data.assign(aValue.begin(), aValue.end());
    }
    else
    {
        // No existing entry, create a new one.
        mEntries.push_back({ aPath, std::vector<uint8_t>(aValue.begin(), aValue.end()) });
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR RamAttributePersistenceProvider::ReadValue(const app::ConcreteAttributePath & aPath, MutableByteSpan & aValue)
{
    auto it = std::find_if(mEntries.cbegin(), mEntries.cend(), [&aPath](const Entry & entry) { return entry.path == aPath; });

    if (it == mEntries.cend())
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    if (aValue.size() < it->data.size())
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    memcpy(aValue.data(), it->data.data(), it->data.size());
    aValue.reduce_size(it->data.size());

    return CHIP_NO_ERROR;
}

CHIP_ERROR RamAttributePersistenceProvider::DeleteValue(const app::ConcreteAttributePath & aPath)
{
    const auto original_size = mEntries.size();
    mEntries.remove_if([&aPath](const Entry & entry) { return entry.path == aPath; });

    if (mEntries.size() < original_size)
    {
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
}

} // namespace chip::Testing
