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

#ifndef _SETUP_PAYLOAD_H_
#define _SETUP_PAYLOAD_H_

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include <core/CHIPError.h>

using namespace std;

namespace chip {

// TODO this should point to the spec
const int kVersionFieldLengthInBits                  = 3;
const int kVendorIDFieldLengthInBits                 = 16;
const int kProductIDFieldLengthInBits                = 16;
const int kCustomFlowRequiredFieldLengthInBits       = 1;
const int kRendezvousInfoFieldLengthInBits           = 8;
const int kPayloadDiscriminatorFieldLengthInBits     = 12;
const int kManualSetupDiscriminatorFieldLengthInBits = 4;
const int kSetupPINCodeFieldLengthInBits             = 27;
const int kPaddingFieldLengthInBits                  = 5;

const int kRendezvousInfoReservedFieldLengthInBits = 4;
const int kRawVendorTagLengthInBits                = 7;
const uint16_t kSerialNumberTag                    = 128;
const uint32_t kTag_QRCodeExensionDescriptor       = 0x00;

const int kManualSetupShortCodeCharLength = 10;
const int kManualSetupLongCodeCharLength  = 20;
const int kManualSetupVendorIdCharLength  = 5;
const int kManualSetupProductIdCharLength = 5;

// clang-format off
const int kTotalPayloadDataSizeInBits =
    kVersionFieldLengthInBits +
    kVendorIDFieldLengthInBits +
    kProductIDFieldLengthInBits +
    kCustomFlowRequiredFieldLengthInBits +
    kRendezvousInfoFieldLengthInBits +
    kPayloadDiscriminatorFieldLengthInBits +
    kSetupPINCodeFieldLengthInBits +
    kPaddingFieldLengthInBits;
// clang-format on

const int kTotalPayloadDataSizeInBytes = kTotalPayloadDataSizeInBits / 8;

const char * const kQRCodePrefix = "CH:";

enum optionalQRCodeInfoType
{
    optionalQRCodeInfoTypeString,
    optionalQRCodeInfoTypeInt
};

/**
 * @brief A struct to hold optional QR Code Info
 * @param tag The tag number of the optional info
 * @param type The type (String or Int) of the optional info
 * @param data  If type is optionalQRCodeInfoTypeString,
 *              the string value of the optional info,
 *              otherwise should not be set.
 * @param integer If type is optionalQRCodeInfoTypeInt,
 *              the integer value of the optional info,
 *              otherwise should not be set.
 **/
struct OptionalQRCodeInfo
{
    uint8_t tag;
    enum optionalQRCodeInfoType type;
    int integer;
    string data;
};

class SetupPayload
{
public:
    uint8_t version;
    uint16_t vendorID;
    uint16_t productID;
    uint8_t requiresCustomFlow;
    uint16_t rendezvousInformation;
    uint16_t discriminator;
    uint32_t setUpPINCode;
    string serialNumber;

    /**
     * @brief A function to retrieve the vector of OptionalQRCodeInfo infos
     * @return Returns a vector of optionalQRCodeInfos
     **/
    vector<OptionalQRCodeInfo> getAllOptionalData();

    /** @brief A function to add an optional QR Code info vendor object
     * @param info Optional QR code info object to add
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addVendorOptionalData(OptionalQRCodeInfo info);

    /** @brief A function to add an optional QR Code info CHIP object
     * @param info Optional QR code info object to add
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addCHIPOptionalData(OptionalQRCodeInfo info);

    /** @brief A function to remove an optional QR Code info object
     * @param tag Optional QR code info tag number to remove
     * @return Returns CHIP_ERROR_KEY_NOT_FOUND if info could not be found in existing optional data structs,
     *                 CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR removeOptionalData(uint8_t tag);

    // Test that the Setup Payload is within expected value ranges
    SetupPayload() :
        version(0), vendorID(0), productID(0), requiresCustomFlow(0), rendezvousInformation(0), discriminator(0), setUpPINCode(0){};

    bool isValidQRCodePayload();
    bool isValidManualCode();
    bool operator==(const SetupPayload & input);

private:
    map<uint64_t, OptionalQRCodeInfo> optionalData;
};

}; // namespace chip

#endif /* _SETUP_PAYLOAD_H_ */
