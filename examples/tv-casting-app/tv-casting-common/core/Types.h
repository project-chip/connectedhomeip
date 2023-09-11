/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "core/CastingApp.h"
#include "support/AppParameters.h"

#include <cstdint>
#include <memory>

namespace matter {
namespace casting {

namespace memory {

template <typename T>
using Weak = std::weak_ptr<T>;

template <typename T>
using Strong = std::shared_ptr<T>;

} // namespace memory

namespace core {

class CastingApp;

}; // namespace core

namespace support {

class AppParameters;
class ByteSpanDataProvider;
class ServerInitParamsProvider;

} // namespace support

}; // namespace casting
}; // namespace matter
