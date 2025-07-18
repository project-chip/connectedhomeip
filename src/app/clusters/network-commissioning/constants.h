/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <cstddef>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

// For WiFi and Thread scan results, each item will cost ~60 bytes in TLV,
// thus 15 is a safe upper bound of scan results.
static constexpr size_t kMaxNetworksInScanResponse = 15;
static constexpr size_t kPossessionNonceSize       = 32;

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
