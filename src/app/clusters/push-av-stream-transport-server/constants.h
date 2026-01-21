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

#include <app-common/zap-generated/cluster-objects.h>
#include <cstddef>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

using SupportedFormatStruct                   = Structs::SupportedFormatStruct::Type;
using CMAFContainerOptionsStruct              = Structs::CMAFContainerOptionsStruct::Type;
using ContainerOptionsStruct                  = Structs::ContainerOptionsStruct::Type;
using TransportZoneOptionsStruct              = Structs::TransportZoneOptionsStruct::Type;
using TransportTriggerOptionsStruct           = Structs::TransportTriggerOptionsStruct::Type;
using TransportMotionTriggerTimeControlStruct = Structs::TransportMotionTriggerTimeControlStruct::Type;
using TransportOptionsStruct                  = Structs::TransportOptionsStruct::Type;
using TransportConfigurationStruct            = Structs::TransportConfigurationStruct::Type;
using StreamUsageEnum                         = chip::app::Clusters::Globals::StreamUsageEnum;

static constexpr size_t kMinUrlLength        = 13u;
static constexpr size_t kMaxUrlLength        = 2000u;
static constexpr size_t kMaxCENCKeyLength    = 16u;
static constexpr size_t kMaxCENCKeyIDLength  = 16u;
static constexpr size_t kMaxTrackNameLength  = 16u;
static constexpr size_t kMinSegmentDuration  = 500u;
static constexpr size_t kMaxSegmentDuration  = 65500u;
static constexpr size_t kMinStreamNameLength = 1u;
static constexpr size_t kMaxStreamNameLength = 16u;

enum class PushAvStreamTransportStatusEnum : uint8_t
{
    kBusy    = 0x00,
    kIdle    = 0x01,
    kUnknown = 0x02
};

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
