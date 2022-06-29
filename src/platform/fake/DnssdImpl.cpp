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
#include "DnssdImpl.h"
#include "lib/dnssd/platform/Dnssd.h"

namespace chip {
namespace Dnssd {

namespace test {
namespace {
static constexpr size_t kMaxExpectedCalls = 10;
size_t numExpectedCalls                   = 0;
ExpectedCall expectedCalls[kMaxExpectedCalls];
bool foundCall[kMaxExpectedCalls] = {};
} // namespace

CHIP_ERROR AddExpectedCall(const ExpectedCall & call)
{
    if (!call.IsValid())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (numExpectedCalls >= kMaxExpectedCalls)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    expectedCalls[numExpectedCalls++] = call;
    return CHIP_NO_ERROR;
}

void Reset()
{
    numExpectedCalls = 0;
    for (auto & found : foundCall)
    {
        found = false;
    }
}

CHIP_ERROR CheckExpected(CallType type, const DnssdService * service)
{
    for (size_t i = 0; i < test::numExpectedCalls; ++i)
    {
        if (test::foundCall[i])
        {
            continue;
        }
        if (test::expectedCalls[i].CheckMatch(type, service))
        {
            return CHIP_NO_ERROR;
        }
    }
    ChipLogError(Discovery, "Unexpected event of type %d", static_cast<int>(type));
    ChipLogProgress(Discovery, "mName = %s", service->mName);
    ChipLogProgress(Discovery, "mHostName = %s", service->mHostName);
    ChipLogProgress(Discovery, "mType = %s", service->mType);
    ChipLogProgress(Discovery, "mProtocol = %d", static_cast<int>(service->mProtocol));
    ChipLogProgress(Discovery, "num subtypes = %lu", static_cast<unsigned long>(service->mSubTypeSize));
    for (size_t i = 0; i < service->mSubTypeSize; ++i)
    {
        ChipLogProgress(Discovery, "\t%s", service->mSubTypes[i]);
    }
    ChipLogProgress(Discovery, "num text entries = %lu", static_cast<unsigned long>(service->mTextEntrySize));
    for (size_t i = 0; i < service->mTextEntrySize; ++i)
    {
        ChipLogProgress(Discovery, "\t%s", service->mTextEntries[i].mKey);
    }
    ChipLogProgress(Discovery, "\nExpected:");
    for (size_t i = 0; i < test::numExpectedCalls; ++i)
    {
        ChipLogProgress(Discovery, "Call %lu", static_cast<unsigned long>(i));
        test::expectedCalls[i].PrintForDebugging();
    }
    return CHIP_ERROR_UNEXPECTED_EVENT;
}

} // namespace test

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    initCallback(context, CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

void ChipDnssdShutdown() {}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    return test::CheckExpected(test::CallType::kStart, service);
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipDnssdResolve(DnssdService * browseResult, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                            void * context)

{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void GetMdnsTimeout(timeval & timeout) {}
void HandleMdnsTimeout() {}

} // namespace Dnssd
} // namespace chip
