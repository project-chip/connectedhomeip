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

#include "lib/mdns/platform/Mdns.h"

using namespace chip::Mdns;

namespace {

} // namespace

namespace chip {
namespace Mdns {

CHIP_ERROR ChipMdnsInit(MdnsAsyncReturnCallback successCallback, MdnsAsyncReturnCallback errorCallback, void * context)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipMdnsShutdown()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipMdnsSetHostname(const char * hostname)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipMdnsPublishService(const MdnsService * service)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipMdnsStopPublish()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipMdnsBrowse(const char * type, MdnsServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                          chip::Inet::InterfaceId interface, MdnsBrowseCallback callback, void * context)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipMdnsResolve(MdnsService * service, chip::Inet::InterfaceId interface, MdnsResolveCallback callback, void * context)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void GetMdnsTimeout(timeval & timeout) {}

void HandleMdnsTimeout() {}

} // namespace Mdns
} // namespace chip
