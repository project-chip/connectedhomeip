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

#include "core/Types.h"

#include <app/server/Server.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace matter {
namespace casting {
namespace support {

/**
 * @brief Generic DataProvider template that allows the implementer to provide a pointer to some data with type T
 *
 * @tparam T type of the data to be provided
 */
template <typename T>
class DataProvider
{
public:
    virtual ~DataProvider(){};
    virtual T * Get() { return nullptr; };
};

class MutableByteSpanDataProvider : public DataProvider<chip::MutableByteSpan>
{
public:
    virtual chip::MutableByteSpan * Get() { return nullptr; };
};

class ServerInitParamsProvider : public DataProvider<chip::ServerInitParams>
{
public:
    virtual chip::ServerInitParams * Get() { return nullptr; };
};

}; // namespace support
}; // namespace casting
}; // namespace matter
