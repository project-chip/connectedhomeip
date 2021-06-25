/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <core/CHIPError.h>
#include <mdns/Resolver.h>
#include <support/Span.h>

#include <cstddef>
#include <cstdint>

namespace chip {
namespace Mdns {

#ifdef CHIP_CONFIG_TEST
namespace Internal {

enum class TxtFieldKey : uint8_t
{
    kUnknown,
    kLongDiscriminator,
    kVendorProduct,
    kAdditionalPairing,
    kCommissioningMode,
    kDeviceType,
    kDeviceName,
    kRotatingDeviceId,
    kPairingInstruction,
    kPairingHint,
};

TxtFieldKey GetTxtFieldKey(const ByteSpan & key);

uint16_t GetProduct(const ByteSpan & value);
uint16_t GetVendor(const ByteSpan & value);
uint16_t GetLongDisriminator(const ByteSpan & value);
uint8_t GetAdditionalPairing(const ByteSpan & value);
uint8_t GetCommissioningMode(const ByteSpan & value);
// TODO: possibly 32-bit? see spec issue #3226
uint16_t GetDeviceType(const ByteSpan & value);
void GetDeviceName(const ByteSpan & value, char * name);
void GetRotatingDeviceId(const ByteSpan & value, uint8_t * rotatingId, size_t * len);
uint16_t GetPairingHint(const ByteSpan & value);
void GetPairingInstruction(const ByteSpan & value, char * pairingInstruction);

} // namespace Internal
#endif

void FillNodeDataFromTxt(const ByteSpan & key, const ByteSpan & value, DiscoveredNodeData * nodeData);

} // namespace Mdns
} // namespace chip
