/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include <lib/support/jsontlv/JsonToTlv.h>
#include <string>

using namespace chip;

extern "C" {
// Returns the number of TLV encoded bytes extracted from the json string
// Input:
//    json - JSON string containing the MatterTLVJson style json for one cluster
//    buf - output buffer
//    size - output buffer size
// Returns:
//    Number of TLV bytes encoded. If this fails, 0 will be returned.
size_t pychip_JsonToTlv(const char * json, uint8_t * buf, size_t size)
{
    MutableByteSpan tlvSpan(buf, size);
    CHIP_ERROR err = JsonToTlv(json, tlvSpan);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Error parsing JsonTlv %s", err.AsString());
        return 0;
    }
    return tlvSpan.size();
}
} // extern "C"
