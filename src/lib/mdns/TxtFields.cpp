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

#include "TxtFields.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdio>
#include <inttypes.h>
#include <limits>
#include <stdlib.h>
#include <string.h>

#include <lib/mdns/Advertiser.h>
#include <lib/mdns/Resolver.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/SafeInt.h>

namespace chip {
namespace Mdns {

namespace Internal {

namespace {

char SafeToLower(uint8_t ch)
{
    return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
}
bool IsKey(const ByteSpan & key, const char * desired)
{
    if (key.size() != strlen(desired))
    {
        return false;
    }
    for (size_t i = 0; i < key.size(); ++i)
    {
        if (SafeToLower(key.data()[i]) != SafeToLower(desired[i]))
        {
            return false;
        }
    }
    return true;
}

uint32_t MakeU32FromAsciiDecimal(const ByteSpan & val, uint32_t defaultValue = 0)
{
    // +1 because `digits10` means the number of decimal digits that fit in `uint32_t`,
    // not how many digits are enough to represent any `uint32_t`
    // +1 for null-terminator
    char nullTerminatedValue[std::numeric_limits<uint32_t>::digits10 + 2];

    // value is too long to store `uint32_t`
    if (val.size() >= sizeof(nullTerminatedValue))
        return defaultValue;

    // value contains leading zeros
    if (val.size() > 1 && *val.data() == static_cast<uint8_t>('0'))
        return defaultValue;

    Platform::CopyString(nullTerminatedValue, sizeof(nullTerminatedValue), val);

    char * endPtr;
    unsigned long num = strtoul(nullTerminatedValue, &endPtr, 10);

    if (endPtr > nullTerminatedValue && *endPtr == '\0' && num != ULONG_MAX && CanCastTo<uint32_t>(num))
        return static_cast<uint32_t>(num);

    return defaultValue;
}

uint16_t MakeU16FromAsciiDecimal(const ByteSpan & val)
{
    const uint32_t num = MakeU32FromAsciiDecimal(val);
    return CanCastTo<uint16_t>(num) ? static_cast<uint16_t>(num) : 0;
}

uint8_t MakeU8FromAsciiDecimal(const ByteSpan & val)
{
    const uint32_t num = MakeU32FromAsciiDecimal(val);
    return CanCastTo<uint8_t>(num) ? static_cast<uint8_t>(num) : 0;
}

bool MakeBoolFromAsciiDecimal(const ByteSpan & val)
{
    return val.size() == 1 && static_cast<char>(*val.data()) == '1';
}

size_t GetPlusSignIdx(const ByteSpan & value)
{
    // Fist value is the vendor id, second (after the +) is the product.
    for (int i = 0; i < static_cast<int>(value.size()); ++i)
    {
        if (static_cast<char>(value.data()[i]) == '+')
        {
            return i;
        }
    }
    return value.size();
}

} // namespace

uint16_t GetProduct(const ByteSpan & value)
{
    size_t plussign = GetPlusSignIdx(value);
    if (plussign < value.size() - 1)
    {
        const uint8_t * productStrStart = value.data() + plussign + 1;
        size_t productStrLen            = value.size() - plussign - 1;
        return MakeU16FromAsciiDecimal(ByteSpan(productStrStart, productStrLen));
    }
    return 0;
}

uint16_t GetVendor(const ByteSpan & value)
{
    size_t plussign = GetPlusSignIdx(value);
    return MakeU16FromAsciiDecimal(ByteSpan(value.data(), plussign));
}

uint16_t GetLongDiscriminator(const ByteSpan & value)
{
    return MakeU16FromAsciiDecimal(value);
}

uint8_t GetAdditionalPairing(const ByteSpan & value)
{
    return MakeU8FromAsciiDecimal(value);
}

uint8_t GetCommissioningMode(const ByteSpan & value)
{
    return MakeU8FromAsciiDecimal(value);
}

// TODO: possibly 32-bit? see spec issue #3226
uint16_t GetDeviceType(const ByteSpan & value)
{
    return MakeU16FromAsciiDecimal(value);
}

void GetDeviceName(const ByteSpan & value, char * name)
{
    Platform::CopyString(name, kMaxDeviceNameLen + 1, value);
}

void GetRotatingDeviceId(const ByteSpan & value, uint8_t * rotatingId, size_t * len)
{
    *len = Encoding::HexToBytes(reinterpret_cast<const char *>(value.data()), value.size(), rotatingId, kMaxRotatingIdLen);
}

uint16_t GetPairingHint(const ByteSpan & value)
{
    return MakeU16FromAsciiDecimal(value);
}

void GetPairingInstruction(const ByteSpan & value, char * pairingInstruction)
{
    Platform::CopyString(pairingInstruction, kMaxPairingInstructionLen + 1, value);
}

uint32_t GetRetryInterval(const ByteSpan & value)
{
    const auto retryInterval = MakeU32FromAsciiDecimal(value, kUndefinedRetryInterval);

    if (retryInterval != kUndefinedRetryInterval && retryInterval <= kMaxRetryInterval)
        return retryInterval;

    return kUndefinedRetryInterval;
}

TxtFieldKey GetTxtFieldKey(const ByteSpan & key)
{
    if (IsKey(key, "D"))
    {
        return TxtFieldKey::kLongDiscriminator;
    }
    else if (IsKey(key, "VP"))
    {
        return TxtFieldKey::kVendorProduct;
    }
    else if (IsKey(key, "AP"))
    {
        return TxtFieldKey::kAdditionalPairing;
    }
    else if (IsKey(key, "CM"))
    {
        return TxtFieldKey::kCommissioningMode;
    }
    else if (IsKey(key, "DT"))
    {
        return TxtFieldKey::kDeviceType;
    }
    else if (IsKey(key, "DN"))
    {
        return TxtFieldKey::kDeviceName;
    }
    else if (IsKey(key, "RI"))
    {
        return TxtFieldKey::kRotatingDeviceId;
    }
    else if (IsKey(key, "PI"))
    {
        return TxtFieldKey::kPairingInstruction;
    }
    else if (IsKey(key, "PH"))
    {
        return TxtFieldKey::kPairingHint;
    }
    else if (IsKey(key, "CRI"))
    {
        return TxtFieldKey::kMrpRetryIntervalIdle;
    }
    else if (IsKey(key, "CRA"))
    {
        return TxtFieldKey::kMrpRetryIntervalActive;
    }
    else if (IsKey(key, "T"))
    {
        return TxtFieldKey::kTcpSupport;
    }
    else
    {
        return TxtFieldKey::kUnknown;
    }
}

} // namespace Internal

void FillNodeDataFromTxt(const ByteSpan & key, const ByteSpan & val, DiscoveredNodeData & nodeData)
{
    Internal::TxtFieldKey keyType = Internal::GetTxtFieldKey(key);
    switch (keyType)
    {
    case Internal::TxtFieldKey::kLongDiscriminator:
        nodeData.longDiscriminator = Internal::GetLongDiscriminator(val);
        break;
    case Internal::TxtFieldKey::kVendorProduct:
        nodeData.vendorId  = Internal::GetVendor(val);
        nodeData.productId = Internal::GetProduct(val);
        break;
    case Internal::TxtFieldKey::kAdditionalPairing:
        nodeData.additionalPairing = Internal::GetAdditionalPairing(val);
        break;
    case Internal::TxtFieldKey::kCommissioningMode:
        nodeData.commissioningMode = Internal::GetCommissioningMode(val);
        break;
    case Internal::TxtFieldKey::kDeviceType:
        nodeData.deviceType = Internal::GetDeviceType(val);
        break;
    case Internal::TxtFieldKey::kDeviceName:
        Internal::GetDeviceName(val, nodeData.deviceName);
        break;
    case Internal::TxtFieldKey::kRotatingDeviceId:
        Internal::GetRotatingDeviceId(val, nodeData.rotatingId, &nodeData.rotatingIdLen);
        break;
    case Internal::TxtFieldKey::kPairingInstruction:
        Internal::GetPairingInstruction(val, nodeData.pairingInstruction);
        break;
    case Internal::TxtFieldKey::kPairingHint:
        nodeData.pairingHint = Internal::GetPairingHint(val);
        break;
    case Internal::TxtFieldKey::kMrpRetryIntervalIdle:
        nodeData.mrpRetryIntervalIdle = Internal::GetRetryInterval(val);
        break;
    case Internal::TxtFieldKey::kMrpRetryIntervalActive:
        nodeData.mrpRetryIntervalActive = Internal::GetRetryInterval(val);
        break;
    case Internal::TxtFieldKey::kTcpSupport:
        nodeData.supportsTcp = Internal::MakeBoolFromAsciiDecimal(val);
        break;
    default:
        break;
    }
}

void FillNodeDataFromTxt(const ByteSpan & key, const ByteSpan & value, ResolvedNodeData & nodeData)
{
    switch (Internal::GetTxtFieldKey(key))
    {
    case Internal::TxtFieldKey::kMrpRetryIntervalIdle:
        nodeData.mMrpRetryIntervalIdle = Internal::GetRetryInterval(value);
        break;
    case Internal::TxtFieldKey::kMrpRetryIntervalActive:
        nodeData.mMrpRetryIntervalActive = Internal::GetRetryInterval(value);
        break;
    case Internal::TxtFieldKey::kTcpSupport:
        nodeData.mSupportsTcp = Internal::MakeBoolFromAsciiDecimal(value);
        break;
    default:
        break;
    }
}

} // namespace Mdns
} // namespace chip
