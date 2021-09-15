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
 *      This file describes a QRCode Setup Payload class to hold
 *      data enumerated from a byte stream
 */

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>

namespace chip {

// See section 5.1.2. QR Code in the Matter specification
const int kVersionFieldLengthInBits              = 3;
const int kVendorIDFieldLengthInBits             = 16;
const int kProductIDFieldLengthInBits            = 16;
const int kCommissioningFlowFieldLengthInBits    = 2;
const int kRendezvousInfoFieldLengthInBits       = 8;
const int kPayloadDiscriminatorFieldLengthInBits = 12;
const int kSetupPINCodeFieldLengthInBits         = 27;
const int kPaddingFieldLengthInBits              = 4;
const int kRawVendorTagLengthInBits              = 7;

// See section 5.1.3. Manual Pairing Code in the Matter specification
const int kManualSetupDiscriminatorFieldLengthInBits  = 4;
const int kManualSetupChunk1DiscriminatorMsbitsPos    = 0;
const int kManualSetupChunk1DiscriminatorMsbitsLength = 2;
const int kManualSetupChunk1VidPidPresentBitPos =
    (kManualSetupChunk1DiscriminatorMsbitsPos + kManualSetupChunk1DiscriminatorMsbitsLength);
const int kManualSetupChunk2PINCodeLsbitsPos       = 0;
const int kManualSetupChunk2PINCodeLsbitsLength    = 14;
const int kManualSetupChunk2DiscriminatorLsbitsPos = (kManualSetupChunk2PINCodeLsbitsPos + kManualSetupChunk2PINCodeLsbitsLength);
const int kManualSetupChunk2DiscriminatorLsbitsLength = 2;
const int kManualSetupChunk3PINCodeMsbitsPos          = 0;
const int kManualSetupChunk3PINCodeMsbitsLength       = 13;

const int kManualSetupShortCodeCharLength  = 10;
const int kManualSetupLongCodeCharLength   = 20;
const int kManualSetupCodeChunk1CharLength = 1;
const int kManualSetupCodeChunk2CharLength = 5;
const int kManualSetupCodeChunk3CharLength = 4;
const int kManualSetupVendorIdCharLength   = 5;
const int kManualSetupProductIdCharLength  = 5;

const uint8_t kSerialNumberTag = 128;

// The largest value of the 12-bit Payload discriminator
const uint16_t kMaxDiscriminatorValue = 0xFFF;

// clang-format off
const int kTotalPayloadDataSizeInBits =
    kVersionFieldLengthInBits +
    kVendorIDFieldLengthInBits +
    kProductIDFieldLengthInBits +
    kCommissioningFlowFieldLengthInBits +
    kRendezvousInfoFieldLengthInBits +
    kPayloadDiscriminatorFieldLengthInBits +
    kSetupPINCodeFieldLengthInBits +
    kPaddingFieldLengthInBits;
// clang-format on

const int kTotalPayloadDataSizeInBytes = kTotalPayloadDataSizeInBits / 8;

const char * const kQRCodePrefix = "MT:";

/// The rendezvous type this device supports.
enum class RendezvousInformationFlag : uint8_t
{
    kNone      = 0,      ///< Device does not support any method for rendezvous
    kSoftAP    = 1 << 0, ///< Device supports Wi-Fi softAP
    kBLE       = 1 << 1, ///< Device supports BLE
    kOnNetwork = 1 << 2, ///< Device supports Setup on network
};
using RendezvousInformationFlags = chip::BitFlags<RendezvousInformationFlag, uint8_t>;

enum class CommissioningFlow : uint8_t
{
    kStandard = 0,       ///< Device automatically enters pairing mode upon power-up
    kUserActionRequired, ///< Device requires a user interaction to enter pairing mode
    kCustom,             ///< Commissioning steps should be retrieved from the distributed compliance ledger
};

enum optionalQRCodeInfoType
{
    optionalQRCodeInfoTypeUnknown,
    optionalQRCodeInfoTypeString,
    optionalQRCodeInfoTypeInt32,
    optionalQRCodeInfoTypeInt64,
    optionalQRCodeInfoTypeUInt32,
    optionalQRCodeInfoTypeUInt64
};

/**
 * A structure to hold optional QR Code info
 */
struct OptionalQRCodeInfo
{
    OptionalQRCodeInfo() { int32 = 0; }

    /*@{*/
    uint8_t tag;                      /**< the tag number of the optional info */
    enum optionalQRCodeInfoType type; /**< the type (String or Int) of the optional info */
    std::string data;                 /**< the string value if type is optionalQRCodeInfoTypeString, otherwise should not be set */
    int32_t int32;                    /**< the integer value if type is optionalQRCodeInfoTypeInt, otherwise should not be set */
    /*@}*/
};

struct OptionalQRCodeInfoExtension : OptionalQRCodeInfo
{
    OptionalQRCodeInfoExtension()
    {
        int32  = 0;
        int64  = 0;
        uint32 = 0;
        uint64 = 0;
    }

    int64_t int64;
    uint64_t uint32;
    uint64_t uint64;
};

bool IsCHIPTag(uint8_t tag);
bool IsVendorTag(uint8_t tag);

class SetupPayload
{

    friend class QRCodeSetupPayloadGenerator;
    friend class QRCodeSetupPayloadParser;

public:
    uint8_t version                                  = 0;
    uint16_t vendorID                                = 0;
    uint16_t productID                               = 0;
    CommissioningFlow commissioningFlow              = CommissioningFlow::kStandard;
    RendezvousInformationFlags rendezvousInformation = RendezvousInformationFlag::kNone;
    uint16_t discriminator                           = 0;
    uint32_t setUpPINCode                            = 0;

    /** @brief A function to add an optional vendor data
     * @param tag 7 bit [0-127] tag number
     * @param data String representation of data to add
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addOptionalVendorData(uint8_t tag, std::string data);

    /** @brief A function to add an optional vendor data
     * @param tag 7 bit [0-127] tag number
     * @param data Integer representation of data to add
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addOptionalVendorData(uint8_t tag, int32_t data);

    /** @brief A function to remove an optional vendor data
     * @param tag 7 bit [0-127] tag number
     * @return Returns a CHIP_ERROR_KEY_NOT_FOUND on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR removeOptionalVendorData(uint8_t tag);
    /**
     * @brief A function to retrieve the vector of OptionalQRCodeInfo infos
     * @return Returns a vector of optionalQRCodeInfos
     **/
    std::vector<OptionalQRCodeInfo> getAllOptionalVendorData() const;

    /** @brief A function to add a string serial number
     * @param serialNumber string serial number
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addSerialNumber(std::string serialNumber);

    /** @brief A function to add a uint32_t serial number
     * @param serialNumber uint32_t serial number
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addSerialNumber(uint32_t serialNumber);

    /** @brief A function to retrieve serial number as a string
     * @param outSerialNumber retrieved string serial number
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR getSerialNumber(std::string & outSerialNumber) const;

    /** @brief A function to remove the serial number from the payload
     * @return Returns a CHIP_ERROR_KEY_NOT_FOUND on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR removeSerialNumber();

    bool isValidQRCodePayload();
    bool isValidManualCode();
    bool operator==(SetupPayload & input);

private:
    std::map<uint8_t, OptionalQRCodeInfo> optionalVendorData;
    std::map<uint8_t, OptionalQRCodeInfoExtension> optionalExtensionData;

    /** @brief A function to add an optional QR Code info vendor object
     * @param info Optional QR code info object to add
     * @return Returns a CHIP_ERROR_INVALID_ARGUMENT on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addOptionalVendorData(const OptionalQRCodeInfo & info);

    /** @brief A function to add an optional QR Code info CHIP object
     * @param info Optional QR code info object to add
     * @return Returns a CHIP_ERROR_INVALID_ARGUMENT on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addOptionalExtensionData(const OptionalQRCodeInfoExtension & info);

    /**
     * @brief A function to retrieve the vector of CHIPQRCodeInfo infos
     * @return Returns a vector of CHIPQRCodeInfos
     **/
    std::vector<OptionalQRCodeInfoExtension> getAllOptionalExtensionData();

    /** @brief A function to retrieve an optional QR Code info vendor object
     * @param tag 7 bit [0-127] tag number
     * @param info retrieved OptionalQRCodeInfo object
     * @return Returns a CHIP_ERROR_KEY_NOT_FOUND on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR getOptionalVendorData(uint8_t tag, OptionalQRCodeInfo & info);

    /** @brief A function to retrieve an optional QR Code info extended object
     * @param tag 8 bit [128-255] tag number
     * @param info retrieved OptionalQRCodeInfoExtension object
     * @return Returns a CHIP_ERROR_KEY_NOT_FOUND on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR getOptionalExtensionData(uint8_t tag, OptionalQRCodeInfoExtension & info) const;

    /** @brief A function to retrieve the associated expected numeric value for a tag
     * @param tag 8 bit [0-255] tag number
     * @return Returns an optionalQRCodeInfoType value
     **/
    optionalQRCodeInfoType getNumericTypeFor(uint8_t tag);
};

} // namespace chip
