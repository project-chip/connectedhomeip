/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
