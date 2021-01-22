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

#include "lib/mdns/Scanner.h"

namespace chip {
namespace Mdns {

class NoneScanner : public Scanner
{
public:
    CHIP_ERROR RegisterScannerDelegate(ScannerDelegate * delegate) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    CHIP_ERROR SubscribeNode(uint64_t nodeId, uint64_t fabricId, Inet::InterfaceId interface,
                             Inet::IPAddressType addressType) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR UnsubscribeNode(uint64_t nodeId, uint64_t fabricId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
};

Scanner & Scanner::Instance()
{
    static NoneScanner sScanner;
    return sScanner;
}

} // namespace Mdns
} // namespace chip
