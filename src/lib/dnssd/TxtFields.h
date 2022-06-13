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

#include <lib/core/CHIPError.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/Span.h>
#include <system/SystemClock.h>

#include <cstddef>
#include <cstdint>

namespace chip {
namespace Dnssd {

using namespace System::Clock::Literals;

// Operational node TXT entries
static constexpr size_t kKeySleepyIdleIntervalMaxLength          = 7; // [SII] 0-3600000
static constexpr size_t kKeySleepyActiveIntervalMaxLength        = 7; // [SAI] 0-3600000
static constexpr System::Clock::Milliseconds32 kMaxRetryInterval = 3600000_ms32;
static constexpr size_t kKeyTcpSupportedMaxLength                = 1;

// Commissionable/commissioner node TXT entries
static constexpr size_t kKeyLongDiscriminatorMaxLength       = 5;
static constexpr size_t kKeyVendorProductMaxLength           = 11;
static constexpr size_t kKeyAdditionalCommissioningMaxLength = 1;
static constexpr size_t kKeyCommissioningModeMaxLength       = 1;
static constexpr size_t kKeyDeviceTypeMaxLength              = 10;
static constexpr size_t kKeyDeviceNameMaxLength              = 32;
static constexpr size_t kKeyRotatingDeviceIdMaxLength        = 100;
static constexpr size_t kKeyPairingInstructionMaxLength      = 128;
static constexpr size_t kKeyPairingHintMaxLength             = 10;

enum class TxtKeyUse : uint8_t
{
    kNone,
    kCommon,
    kCommission,
};

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
    kSleepyIdleInterval,
    kSleepyActiveInterval,
    kTcpSupported,
    kCount,
};

namespace Internal {
struct TxtFieldInfo
{
    TxtFieldKey key;
    size_t valMaxSize;
    char keyStr[4];
    TxtKeyUse use;
};

constexpr const TxtFieldInfo txtFieldInfo[static_cast<size_t>(TxtFieldKey::kCount)] = {
    { TxtFieldKey::kUnknown, 0, "", TxtKeyUse::kNone },
    { TxtFieldKey::kLongDiscriminator, kKeyLongDiscriminatorMaxLength, "D", TxtKeyUse::kCommission },
    { TxtFieldKey::kVendorProduct, kKeyVendorProductMaxLength, "VP", TxtKeyUse::kCommission },
    { TxtFieldKey::kAdditionalPairing, kKeyAdditionalCommissioningMaxLength, "AP", TxtKeyUse::kCommission },
    { TxtFieldKey::kCommissioningMode, kKeyCommissioningModeMaxLength, "CM", TxtKeyUse::kCommission },
    { TxtFieldKey::kDeviceType, kKeyDeviceTypeMaxLength, "DT", TxtKeyUse::kCommission },
    { TxtFieldKey::kDeviceName, kKeyDeviceNameMaxLength, "DN", TxtKeyUse::kCommission },
    { TxtFieldKey::kRotatingDeviceId, kKeyRotatingDeviceIdMaxLength, "RI", TxtKeyUse::kCommission },
    { TxtFieldKey::kPairingInstruction, kKeyPairingInstructionMaxLength, "PI", TxtKeyUse::kCommission },
    { TxtFieldKey::kPairingHint, kKeyPairingHintMaxLength, "PH", TxtKeyUse::kCommission },
    { TxtFieldKey::kSleepyIdleInterval, kKeySleepyIdleIntervalMaxLength, "SII", TxtKeyUse::kCommon },
    { TxtFieldKey::kSleepyActiveInterval, kKeySleepyActiveIntervalMaxLength, "SAI", TxtKeyUse::kCommon },
    { TxtFieldKey::kTcpSupported, kKeyTcpSupportedMaxLength, "T", TxtKeyUse::kCommon },
};
#ifdef CHIP_CONFIG_TEST

TxtFieldKey GetTxtFieldKey(const ByteSpan & key);

uint16_t GetProduct(const ByteSpan & value);
uint16_t GetVendor(const ByteSpan & value);
uint16_t GetLongDiscriminator(const ByteSpan & value);
uint8_t GetCommissioningMode(const ByteSpan & value);
uint32_t GetDeviceType(const ByteSpan & value);
void GetDeviceName(const ByteSpan & value, char * name);
void GetRotatingDeviceId(const ByteSpan & value, uint8_t * rotatingId, size_t * len);
uint16_t GetPairingHint(const ByteSpan & value);
void GetPairingInstruction(const ByteSpan & value, char * pairingInstruction);
#endif
} // namespace Internal

constexpr size_t MaxKeyLen(TxtKeyUse use)
{
    size_t max = 0;
    for (auto & info : Internal::txtFieldInfo)
    {
        if (use == info.use)
        {
            max = sizeof(info.keyStr) > max ? sizeof(info.keyStr) : max;
        }
    }
    // minus 1 because sizeof includes the null terminator.
    return max - 1;
}
constexpr size_t TotalKeyLen(TxtKeyUse use)
{
    size_t total = 0;
    for (auto & info : Internal::txtFieldInfo)
    {
        if (use == info.use)
        {
            total += sizeof(info.keyStr) - 1;
        }
    }
    return total;
}

constexpr size_t MaxValueLen(TxtKeyUse use)
{
    size_t max = 0;
    for (auto & info : Internal::txtFieldInfo)
    {
        if (use == info.use)
        {
            max = info.valMaxSize > max ? info.valMaxSize : max;
        }
    }
    // minus 1 because sizeof includes the null terminator.
    return max - 1;
}
constexpr size_t TotalValueLen(TxtKeyUse use)
{
    size_t total = 0;
    for (auto & info : Internal::txtFieldInfo)
    {
        if (use == info.use)
        {
            total += info.valMaxSize - 1;
        }
    }
    return total;
}
constexpr uint8_t KeyCount(TxtKeyUse use)
{
    uint8_t count = 0;
    for (auto & info : Internal::txtFieldInfo)
    {
        if (use == info.use)
        {
            count++;
        }
    }
    return count;
}
constexpr size_t KeySize(TxtFieldKey key)
{
    return sizeof(Internal::txtFieldInfo[static_cast<int>(key)].keyStr) - 1;
}
constexpr size_t ValSize(TxtFieldKey key)
{
    return Internal::txtFieldInfo[static_cast<int>(key)].valMaxSize;
}

void FillNodeDataFromTxt(const ByteSpan & key, const ByteSpan & value, CommonResolutionData & nodeData);
void FillNodeDataFromTxt(const ByteSpan & key, const ByteSpan & value, CommissionNodeData & nodeData);

} // namespace Dnssd
} // namespace chip
