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

class EndpointListLoader;

} // namespace support

}; // namespace casting
}; // namespace matter
