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

#ifndef _QRCODE_SETUP_PAYLOAD_H_
#define _QRCODE_SETUP_PAYLOAD_H_

#include <stdint.h>

namespace chip
{

class QRCodeSetupPayload
{
    public:
        uint8_t version;
        uint16_t vendorID;
        uint16_t productID;
        uint8_t requiresCustomFlow;
        uint16_t rendezvousInformation;
        uint16_t discriminator;
        uint32_t setUpPINCode;
};

};

#endif