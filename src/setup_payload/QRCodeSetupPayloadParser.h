/**
 *
 *    <COPYRIGHT>
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
 *      This file describes a QRCode Setup Payload parser based on the
 *      CHIP specification.
 */

#include "SetupPayload.h"

#include <core/CHIPError.h>
#include <string>
using namespace std;

namespace chip {

/**
 * @class QRCodeSetupPayloadParser
 * A class that can be used to convert a base45 encoded payload to a SetupPayload object
 * */
class QRCodeSetupPayloadParser
{
private:
    string mBase45Representation;

public:
    QRCodeSetupPayloadParser(string base45Representation) : mBase45Representation(base45Representation){};
    CHIP_ERROR populatePayload(SetupPayload & outPayload);
};

}; // namespace chip
