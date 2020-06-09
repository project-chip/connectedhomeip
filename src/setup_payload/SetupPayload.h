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

const int kManualSetupShortCodeCharLength = 10;
const int kManualSetupLongCodeCharLength  = 20;
const int kManualSetupVendorIdCharLength  = 5;
const int kManualSetupProductIdCharLength = 5;

const uint8_t kSerialNumberTag = 128;

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

enum chipQRCodeInfoType
{
    chipQRCodeInfoTypeString,
    chipQRCodeInfoTypeUInt32
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

struct CHIPQRCodeInfo
{
    uint8_t tag;
    enum chipQRCodeInfoType type;
    uint32_t unsignedInt32;
    string stringData;
};

bool IsCHIPTag(uint8_t tag);
bool IsVendorTag(uint8_t tag);
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

    /**
     * @brief A function to retrieve the vector of OptionalQRCodeInfo infos
     * @return Returns a vector of optionalQRCodeInfos
     **/
    vector<OptionalQRCodeInfo> getAllVendorOptionalData();

    /**
     * @brief A function to retrieve the vector of CHIPQRCodeInfo infos
     * @return Returns a vector of CHIPQRCodeInfos
     **/
    vector<CHIPQRCodeInfo> getAllCHIPOptionalData();

    /** @brief A function to add a string serial number
     * @param serialNumber string serial number
     * @return Returns CHIP_ERROR_KEY_NOT_FOUND if serial number not set,
     *                 CHIP_ERROR on error
     *                 CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addSerialNumber(string serialNumber);

    /** @brief A function to add a uint32_t serial number
     * @param serialNumber uint32_t serial number
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addSerialNumber(uint32_t serialNumber);

    /** @brief A function to remove the serial number from the payload
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR removeSerialNumber(void);

    /** @brief A function to retrieve serial number as a string
     * @param outSerialNumber retrieved string serial number
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR retrieveSerialNumber(string & outSerialNumber);

    /** @brief A function to add an optional vendor data
     * @param tag 7 bit [0-127] tag number
     * @param data String representation of data to add
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addVendorOptionalData(uint8_t tag, string data);

    /** @brief A function to add an optional vendor data
     * @param tag 7 bit [0-127] tag number
     * @param data Integer representation of data to add
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addVendorOptionalData(uint8_t tag, int data);

    /** @brief A function to add an optional QR Code info vendor object
     * @param info Optional QR code info object to add
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addVendorOptionalData(OptionalQRCodeInfo info);

    /** @brief A function to add an optional QR Code info CHIP object
     * @param info Optional QR code info object to add
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR addCHIPOptionalData(CHIPQRCodeInfo info);

    /** @brief A function to remove an optional QR Code vendor info object
     * @param tag Optional QR code vendor info tag number to remove
     * @return Returns CHIP_ERROR_KEY_NOT_FOUND if info could not be found in existing optional data structs,
     *                 CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR removeVendorOptionalData(uint8_t tag);

    /** @brief A function to remove an optional QR Code chip info object
     * @param tag Optional QR code info tag number to remove
     * @return Returns CHIP_ERROR_KEY_NOT_FOUND if info could not be found in existing optional data structs,
     *                 CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR removeCHIPOptionalData(uint8_t tag);

    // Test that the Setup Payload is within expected value ranges
    SetupPayload() :
        version(0), vendorID(0), productID(0), requiresCustomFlow(0), rendezvousInformation(0), discriminator(0), setUpPINCode(0){};

    bool isValidQRCodePayload();
    bool isValidManualCode();
    bool operator==(const SetupPayload & input);

private:
    map<uint8_t, OptionalQRCodeInfo> vendorOptionalData;
    map<uint8_t, CHIPQRCodeInfo> chipOptionalData;
};

}; // namespace chip

#endif /* _SETUP_PAYLOAD_H_ */
