/*
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

#pragma once

#include <app/util/mock/MockNodeConfig.h>

namespace chip {
namespace Testing {

inline const MockNodeConfig & EventLoggingMockNodeConfig()
{
    static const MockNodeConfig config({
        MockEndpointConfig(2, { MockClusterConfig(0x22, {}, { 1 }) }),
        MockEndpointConfig(3, { MockClusterConfig(0x22, {}, { 1 }) }),
    });
    return config;
}

} // namespace Testing
} // namespace chip
