/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file describes a Additional Data Payload class to hold
 *      data enumerated from a byte stream.
 *      Additional data may be conveyed during the provisioning phase via multiple
 *      protocols (BLE, DNS-SD and SoftAP). It contains Rotating Device Id which
 *      is an optional feature for a Node to implement and an optional feature for
 *      a Commissioner to utilize.
 *      The Rotating Device Identifier provides a per-device unique non-trackable
 *      identifier that could be used in one or more of the following ways:
 *      1) Provided to the vendor’s customer support for help in pairing or
 *      establishing Node provenance
 *      2) Used programmatically to obtain a Node’s Setup PIN or other information
 *      in order to provide a simplified setup flow. Note that the mechanism(s) by
 *      which the PIN may be obtained is outside of this specification.
 */

#pragma once

#include <cstdint>
#include <string>

namespace chip {
namespace SetupPayloadData {

constexpr uint8_t kRotatingDeviceIdLength = 18;
constexpr uint8_t kRotatingDeviceIdTag    = 0x00;

struct AdditionalDataPayload
{
public:
    std::string rotatingDeviceId;
};

} // namespace SetupPayloadData
} // namespace chip
