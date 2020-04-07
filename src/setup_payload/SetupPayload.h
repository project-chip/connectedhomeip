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

#include <stdint.h>
#include <string>

using namespace std;

namespace chip {
// TODO this should point to the spec
const int kVersionFieldLengthInBits              = 3;
const int kVendorIDFieldLengthInBits             = 16;
const int kProductIDFieldLengthInBits            = 16;
const int kCustomFlowRequiredFieldLengthInBits   = 1;
const int kRendezvousInfoFieldLengthInBits       = 8;
const int kPayloadDiscriminatorFieldLengthInBits = 8;
const int kSetupPINCodeFieldLengthInBits         = 27;
const int kReservedFieldLengthInBits             = 1;

const int kTotalPayloadDataSizeInBits =
    (kVersionFieldLengthInBits + kVendorIDFieldLengthInBits + kProductIDFieldLengthInBits + kCustomFlowRequiredFieldLengthInBits +
     kRendezvousInfoFieldLengthInBits + kPayloadDiscriminatorFieldLengthInBits + kSetupPINCodeFieldLengthInBits +
     kReservedFieldLengthInBits);
const int kTotalPayloadDataSizeInBytes = (kTotalPayloadDataSizeInBits + 7) / 8;

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

    // Test that the Setup Payload is within expected value ranges
    SetupPayload() :
        version(0), vendorID(0), productID(0), requiresCustomFlow(0), rendezvousInformation(0), discriminator(0), setUpPINCode(0){};

    bool isValid();
    bool operator==(const SetupPayload & input);
};

}; // namespace chip

#endif /* _SETUP_PAYLOAD_H_ */
