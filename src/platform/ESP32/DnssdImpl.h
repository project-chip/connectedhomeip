/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <lib/dnssd/platform/Dnssd.h>
#include <lib/support/CodeUtils.h>
#include <mdns.h>

namespace chip {
namespace Dnssd {
enum class ContextType
{
    Browse = 1,
    Resolve,
    Reserved,
};

struct GenericContext
{
    ContextType mContextType;
    void * mCbContext;
    char mType[kDnssdTypeMaxSize + 1];
    DnssdServiceProtocol mProtocol;
    Inet::InterfaceId mInterfaceId;
    mdns_search_once_t * mSearchHandle;
    mdns_result_t * mResult;
};

struct BrowseContext : public GenericContext
{
    DnssdBrowseCallback mBrowseCb;
    Inet::IPAddressType mAddressType;
    DnssdService * mServices;
    size_t mServiceSize;
    BrowseContext(const char * type, DnssdServiceProtocol protocol, Inet::InterfaceId ifId, mdns_search_once_t * searchHandle,
                  Inet::IPAddressType addrType, DnssdBrowseCallback cb, void * cbCtx)

    {
        memset(mType, 0, sizeof(mType));
        strncpy(mType, type, strnlen(type, kDnssdTypeMaxSize));
        mContextType  = ContextType::Browse;
        mAddressType  = addrType;
        mProtocol     = protocol;
        mBrowseCb     = cb;
        mCbContext    = cbCtx;
        mInterfaceId  = ifId;
        mSearchHandle = searchHandle;
        mResult       = nullptr;
        mServices     = nullptr;
        mServiceSize  = 0;
    }

    ~BrowseContext()
    {
        if (mServices && mServiceSize > 0)
        {
            for (size_t serviceIndex = 0; serviceIndex < mServiceSize; serviceIndex++)
            {
                if (mServices[serviceIndex].mTextEntries)
                {
                    chip::Platform::MemoryFree(mServices[serviceIndex].mTextEntries);
                }
            }
            chip::Platform::MemoryFree(mServices);
        }
        if (mResult)
        {
            mdns_query_results_free(mResult);
        }
        if (mSearchHandle)
        {
            mdns_query_async_delete(mSearchHandle);
        }
    }
};

struct ResolveContext : public GenericContext
{
    char mInstanceName[Common::kInstanceNameMaxLength + 1];
    DnssdResolveCallback mResolveCb;
    DnssdService * mService;
    Inet::IPAddress * mAddresses;
    size_t mAddressCount;

    enum class ResolveState
    {
        QuerySrv,
        QueryTxt,
    } mResolveState;

    ResolveContext(DnssdService * service, Inet::InterfaceId ifId, mdns_search_once_t * searchHandle, DnssdResolveCallback cb,
                   void * cbCtx)
    {
        memset(mType, 0, sizeof(mType));
        memset(mInstanceName, 0, sizeof(mInstanceName));
        strncpy(mType, service->mType, strnlen(service->mType, kDnssdTypeMaxSize));
        mType[kDnssdTypeMaxSize] = 0;
        strncpy(mInstanceName, service->mName, strnlen(service->mName, Common::kInstanceNameMaxLength));
        mInstanceName[Common::kInstanceNameMaxLength] = 0;
        mContextType                                  = ContextType::Resolve;
        mProtocol                                     = service->mProtocol;
        mResolveCb                                    = cb;
        mCbContext                                    = cbCtx;
        mInterfaceId                                  = ifId;
        mSearchHandle                                 = searchHandle;
        mResolveState                                 = ResolveState::QuerySrv;
        mResult                                       = nullptr;
        mService                                      = nullptr;
        mAddresses                                    = nullptr;
        mAddressCount                                 = 0;
    }

    ~ResolveContext()
    {
        if (mService)
        {
            if (mService->mTextEntries)
            {
                chip::Platform::MemoryFree(mService->mTextEntries);
            }
            chip::Platform::MemoryFree(mService);
        }
        if (mAddresses)
        {
            chip::Platform::MemoryFree(mAddresses);
        }
        if (mResult)
        {
            mdns_query_results_free(mResult);
        }
        if (mSearchHandle)
        {
            mdns_query_async_delete(mSearchHandle);
        }
    }
};

} // namespace Dnssd
} // namespace chip
