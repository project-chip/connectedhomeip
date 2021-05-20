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
#include <cstdio>
#include <inttypes.h>
#include <limits>
#include <string.h>

#include <mdns/Resolver.h>

namespace chip {
namespace Mdns {

namespace Internal {

namespace {
bool IsKey(const ByteSpan & key, const char * desired)
{
    if (key.size() != strlen(desired))
    {
        return false;
    }
    return memcmp(key.data(), desired, key.size()) == 0;
}

uint16_t MakeU16FromAsciiDecimal(const ByteSpan & val)
{
    uint32_t u32          = 0;
    const uint16_t errval = 0x0;
    for (size_t i = 0; i < val.size(); ++i)
    {
        char c = static_cast<char>(val.data()[i]);
        if (c < '0' || c > '9')
        {
            return errval;
        }
        u32 = u32 * 10;
        u32 += val.data()[i] - static_cast<uint8_t>('0');
        if (u32 > std::numeric_limits<uint16_t>::max())
        {
            return errval;
        }
    }
    return static_cast<uint16_t>(u32);
}

uint8_t MakeU8FromAsciiDecimal(const ByteSpan & val)
{
    uint16_t u16 = MakeU16FromAsciiDecimal(val);
    if (u16 > std::numeric_limits<uint8_t>::max())
    {
        return 0x0;
    }
    return static_cast<uint8_t>(u16);
}

CHIP_ERROR MakeU8FromAsciiHex(const ByteSpan & ascii, uint8_t * val)
{
    if (ascii.size() != 2)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    uint8_t ret = 0;
    for (size_t i = 0; i < ascii.size(); ++i)
    {
        ret    = ret << 4;
        char c = static_cast<char>(ascii.data()[i]);
        if (c >= '0' && c <= '9')
        {
            ret += ascii.data()[i] - static_cast<uint8_t>('0');
        }
        // Only uppercase is supported according to spec.
        else if (c >= 'A' && c <= 'F')
        {
            ret += ascii.data()[i] - static_cast<uint8_t>('A') + 0xA;
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    *val = ret;
    return CHIP_NO_ERROR;
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
        return MakeU16FromAsciiDecimal(ByteSpan(value.data() + plussign + 1, value.size() - plussign - 1));
    }
    return 0;
}

uint16_t GetVendor(const ByteSpan & value)
{
    size_t plussign = GetPlusSignIdx(value);
    return MakeU16FromAsciiDecimal(ByteSpan(value.data(), plussign));
}

uint16_t GetLongDisriminator(const ByteSpan & value)
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
    size_t len = std::min(value.size(), kMaxDeviceNameLen);
    memcpy(name, value.data(), len);
    name[len] = '\0';
}

void GetRotatingDeviceId(const ByteSpan & value, uint8_t * rotatingId, size_t * len)
{
    memset(rotatingId, 0, kMaxRotatingIdLen);
    *len = 0;
    // Octet string where each octet is 2 ascii digits representing the hex value
    // Each is represented by two ascii chars, so must be even number
    if ((value.size() & 0x1) != 0 || value.size() > (kMaxRotatingIdLen * 2))
    {
        return;
    }
    for (size_t i = 0; i < value.size(); i += 2)
    {
        if (MakeU8FromAsciiHex(ByteSpan(value.data() + i, 2), &rotatingId[i / 2]) != CHIP_NO_ERROR)
        {
            *len = 0;
            memset(rotatingId, 0, kMaxRotatingIdLen);
            return;
        }
        *len = *len + 1;
    }
}

uint16_t GetPairingHint(const ByteSpan & value)
{
    return MakeU16FromAsciiDecimal(value);
}

void GetPairingInstruction(const ByteSpan & value, char * pairingInstruction)
{
    size_t len = std::min(value.size(), kMaxPairingInstructionLen);
    memcpy(pairingInstruction, value.data(), len);
    pairingInstruction[len] = '\0';
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
    else
    {
        return TxtFieldKey::kUnknown;
    }
}

} // namespace Internal

void FillNodeDataFromTxt(const ByteSpan & key, const ByteSpan & val, CommissionableNodeData * nodeData)
{
    Internal::TxtFieldKey keyType = Internal::GetTxtFieldKey(key);
    switch (keyType)
    {
    case Internal::TxtFieldKey::kLongDiscriminator:
        nodeData->longDiscriminator = Internal::GetLongDisriminator(val);
        break;
    case Internal::TxtFieldKey::kVendorProduct:
        nodeData->vendorId  = Internal::GetVendor(val);
        nodeData->productId = Internal::GetProduct(val);
        break;
    case Internal::TxtFieldKey::kAdditionalPairing:
        nodeData->additionalPairing = Internal::GetAdditionalPairing(val);
        break;
    case Internal::TxtFieldKey::kCommissioningMode:
        nodeData->commissioningMode = Internal::GetCommissioningMode(val);
        break;
    case Internal::TxtFieldKey::kDeviceType:
        nodeData->deviceType = Internal::GetDeviceType(val);
        break;
    case Internal::TxtFieldKey::kDeviceName:
        Internal::GetDeviceName(val, nodeData->deviceName);
        break;
    case Internal::TxtFieldKey::kRotatingDeviceId:
        Internal::GetRotatingDeviceId(val, nodeData->rotatingId, &nodeData->rotatingIdLen);
        break;
    case Internal::TxtFieldKey::kPairingInstruction:
        Internal::GetPairingInstruction(val, nodeData->pairingInstruction);
        break;
    case Internal::TxtFieldKey::kPairingHint:
        nodeData->pairingHint = Internal::GetPairingHint(val);
        break;
    case Internal::TxtFieldKey::kUnknown:
        break;
    }
}

} // namespace Mdns
} // namespace chip
