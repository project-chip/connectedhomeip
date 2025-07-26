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
#pragma once

#include <app/ConcreteAttributePath.h>
#include <app/persistence/AttributePersistenceProvider.h>

#include <list>
#include <vector>

namespace chip::Testing {

/// A class that stores persistence data in RAM
class RamAttributePersistenceProvider : public app::AttributePersistenceProvider
{
public:
    CHIP_ERROR WriteValue(const app::ConcreteAttributePath & aPath, const ByteSpan & aValue) override;
    CHIP_ERROR ReadValue(const app::ConcreteAttributePath & aPath, MutableByteSpan & aValue) override;

    CHIP_ERROR DeleteValue(const app::ConcreteAttributePath & aPath);

private:
    struct Entry
    {
        app::ConcreteAttributePath path;
        std::vector<uint8_t> data;
    };

    // We are not trying to be very efficient here as this is for testing only:
    // a single list of values we store. Invariant is that there is only one entry per path.
    std::list<Entry> mEntries;
};

} // namespace chip::Testing
