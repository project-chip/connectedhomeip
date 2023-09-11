/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
