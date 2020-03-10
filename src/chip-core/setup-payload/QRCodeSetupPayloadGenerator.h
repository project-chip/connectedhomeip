/*
<COPYRIGHT>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "QRCodeSetupPayload.h"

#include <bitset>
#include <string>
using namespace std;

#ifndef _QR_CODE_SETUP_PAYLOAD_GENERATOR
#define _QR_CODE_SETUP_PAYLOAD_GENERATOR

const int kVersionFieldLengthInBits = 3;
const int kVendorIDFieldLengthInBits = 16; 
const int kProductIDFieldLengthInBits = 16; 
const int kCustomFlowRequiredFieldLengthInBits = 1;
const int kRendezvousInfoFieldLengthInBits = 8; 
const int kPayloadDiscriminatorFieldLengthInBits = 8;
const int kSetupPINCodeFieldLengthInBits = 27; 
const int kReservedFieldLengthInBits = 1;


const int kTotalPayloadDataSizeInBits = (  kVersionFieldLengthInBits + \
                                            kVendorIDFieldLengthInBits + \
                                            kProductIDFieldLengthInBits + \
                                            kCustomFlowRequiredFieldLengthInBits +
                                            kRendezvousInfoFieldLengthInBits + \
                                            kPayloadDiscriminatorFieldLengthInBits + \
                                            kSetupPINCodeFieldLengthInBits + \
                                            kReservedFieldLengthInBits
                                            );

namespace chip
{

class QRCodeSetupPayloadGenerator
{
    private: 
        bitset <kTotalPayloadDataSizeInBits> payloadBits;
        QRCodeSetupPayload payload;
        int payloadBitsIndex; // points to the index of where we are in the bitset.

        void _populateInteger(uint64_t input, size_t numberOfBits);
        void _populateVersion();
        void _populateVendorID();
        void _populateProductID();
        void _populateCustomFlowRequiredField();
        void _populateRendezVousInfo();
        void _populateDiscriminator();
        void _populateSetupPIN();
        void _populateReservedField();
        void _resetBitSet();
        void _generateBitSet();
        
    public:
        bool enableDebugLogs;
        QRCodeSetupPayloadGenerator(QRCodeSetupPayload setupPayload):payload(setupPayload){enableDebugLogs = false;};
        string getQRCodePayloadBinaryRepresentation();
        string getQRCodePayloadBase45Representation();
};

};

#endif