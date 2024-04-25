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
static constexpr size_t kKeySessionIdleIntervalMaxLength         = 7; // [SII] 0-3600000
static constexpr size_t kKeySessionActiveIntervalMaxLength       = 7; // [SAI] 0-3600000
static constexpr size_t kKeySessionActiveThresholdMaxLength      = 5; // [SAT] 0-65535
static constexpr System::Clock::Milliseconds32 kMaxRetryInterval = 3600000_ms32;
static constexpr size_t kKeyTcpSupportedMaxLength                = 1;
static constexpr size_t kKeyLongIdleTimeICDMaxLength             = 1;

// Commissionable/commissioner node TXT entries
static constexpr size_t kKeyLongDiscriminatorMaxLength    = 5;
static constexpr size_t kKeyVendorProductMaxLength        = 11;
static constexpr size_t kKeyCommissioningModeMaxLength    = 1;
static constexpr size_t kKeyDeviceTypeMaxLength           = 10;
static constexpr size_t kKeyDeviceNameMaxLength           = 32;
static constexpr size_t kKeyRotatingDeviceIdMaxLength     = 100;
static constexpr size_t kKeyPairingInstructionMaxLength   = 128;
static constexpr size_t kKeyPairingHintMaxLength          = 10;
static constexpr size_t kKeyCommissionerPasscodeMaxLength = 1;

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
    kCommissioningMode,
    kDeviceType,
    kDeviceName,
    kRotatingDeviceId,
    kPairingInstruction,
    kPairingHint,
    kCommissionerPasscode,
    kSessionIdleInterval,
    kSessionActiveInterval,
    kSessionActiveThreshold,
    kTcpSupported,
    kLongIdleTimeICD,
    kCount,
};

namespace Internal {
struct TxtFieldInfo
{
    size_t valMaxSize;
    TxtFieldKey key;
    TxtKeyUse use;
    char keyStr[4];
};

constexpr const TxtFieldInfo txtFieldInfo[static_cast<size_t>(TxtFieldKey::kCount)] = {
    { 0, TxtFieldKey::kUnknown, TxtKeyUse::kNone, "" },
    { kKeyLongDiscriminatorMaxLength, TxtFieldKey::kLongDiscriminator, TxtKeyUse::kCommission, "D" },
    { kKeyVendorProductMaxLength, TxtFieldKey::kVendorProduct, TxtKeyUse::kCommission, "VP" },
    { kKeyCommissioningModeMaxLength, TxtFieldKey::kCommissioningMode, TxtKeyUse::kCommission, "CM" },
    { kKeyDeviceTypeMaxLength, TxtFieldKey::kDeviceType, TxtKeyUse::kCommission, "DT" },
    { kKeyDeviceNameMaxLength, TxtFieldKey::kDeviceName, TxtKeyUse::kCommission, "DN" },
    { kKeyRotatingDeviceIdMaxLength, TxtFieldKey::kRotatingDeviceId, TxtKeyUse::kCommission, "RI" },
    { kKeyPairingInstructionMaxLength, TxtFieldKey::kPairingInstruction, TxtKeyUse::kCommission, "PI" },
    { kKeyPairingHintMaxLength, TxtFieldKey::kPairingHint, TxtKeyUse::kCommission, "PH" },
    { kKeyCommissionerPasscodeMaxLength, TxtFieldKey::kCommissionerPasscode, TxtKeyUse::kCommission, "CP" },
    { kKeySessionIdleIntervalMaxLength, TxtFieldKey::kSessionIdleInterval, TxtKeyUse::kCommon, "SII" },
    { kKeySessionActiveIntervalMaxLength, TxtFieldKey::kSessionActiveInterval, TxtKeyUse::kCommon, "SAI" },
    { kKeySessionActiveThresholdMaxLength, TxtFieldKey::kSessionActiveThreshold, TxtKeyUse::kCommon, "SAT" },
    { kKeyTcpSupportedMaxLength, TxtFieldKey::kTcpSupported, TxtKeyUse::kCommon, "T" },
    { kKeyLongIdleTimeICDMaxLength, TxtFieldKey::kLongIdleTimeICD, TxtKeyUse::kCommon, "ICD" },
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
uint8_t GetCommissionerPasscode(const ByteSpan & value);
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
