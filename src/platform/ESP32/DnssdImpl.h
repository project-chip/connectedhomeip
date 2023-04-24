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
#include <lib/support/CHIPMemString.h>
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
    char mType[kDnssdTypeMaxSize + 1];
    DnssdServiceProtocol mProtocol;
    Inet::InterfaceId mInterfaceId;
    void * mCbContext;
    DnssdService * mService = nullptr;
    size_t mServiceSize     = 0;
    GenericContext(ContextType ctxType, const char * type, DnssdServiceProtocol protocol, Inet::InterfaceId ifId, void * cbCtx) :
        mContextType(ctxType), mProtocol(protocol), mInterfaceId(ifId), mCbContext(cbCtx)
    {
        Platform::CopyString(mType, type);
    }
    ~GenericContext()
    {
        if (mService && mServiceSize > 0)
        {
            for (size_t serviceIndex = 0; serviceIndex < mServiceSize; serviceIndex++)
            {
                if (mService[serviceIndex].mTextEntries)
                {
                    chip::Platform::MemoryFree(mService[serviceIndex].mTextEntries);
                }
            }
            chip::Platform::MemoryFree(mService);
        }
    }
};

struct BrowseContext : public GenericContext
{
    DnssdBrowseCallback mBrowseCb;
    Inet::IPAddressType mAddressType;
    mdns_search_once_t * mPtrQueryHandle;
    mdns_result_t * mPtrQueryResult = nullptr;
    BrowseContext(const char * type, DnssdServiceProtocol protocol, Inet::InterfaceId ifId, mdns_search_once_t * queryHandle,
                  Inet::IPAddressType addrType, DnssdBrowseCallback cb, void * cbCtx) :
        GenericContext(ContextType::Browse, type, protocol, ifId, cbCtx),
        mBrowseCb(cb), mAddressType(addrType), mPtrQueryHandle(queryHandle)

    {}

    ~BrowseContext()
    {
        if (mPtrQueryResult)
        {
            mdns_query_results_free(mPtrQueryResult);
        }

        if (mPtrQueryHandle)
        {
            mdns_query_async_delete(mPtrQueryHandle);
        }
    }
};

struct ResolveContext : public GenericContext
{
    char mInstanceName[Common::kInstanceNameMaxLength + 1];
    DnssdResolveCallback mResolveCb;
    Inet::IPAddress * mAddresses = nullptr;
    size_t mAddressCount         = 0;

    mdns_search_once_t * mSrvQueryHandle;
    mdns_result_t * mSrvQueryResult  = nullptr;
    mdns_result_t * mAddrQueryResult = nullptr;
    bool mSrvAddrQueryFinished       = false;

    mdns_search_once_t * mTxtQueryHandle;
    mdns_result_t * mTxtQueryResult = nullptr;
    bool mTxtQueryFinished          = false;

    ResolveContext(DnssdService * service, Inet::InterfaceId ifId, mdns_search_once_t * srvQuery, mdns_search_once_t * txtQuery,
                   DnssdResolveCallback cb, void * cbCtx) :
        GenericContext(ContextType::Resolve, service->mType, service->mProtocol, ifId, cbCtx),
        mResolveCb(cb), mSrvQueryHandle(srvQuery), mTxtQueryHandle(txtQuery)
    {
        Platform::CopyString(mInstanceName, service->mName);
    }

    ~ResolveContext()
    {
        if (mAddresses)
        {
            chip::Platform::MemoryFree(mAddresses);
        }
        if (mSrvQueryResult)
        {
            mdns_query_results_free(mSrvQueryResult);
        }
        if (mAddrQueryResult && mAddrQueryResult != mSrvQueryResult)
        {
            mdns_query_results_free(mAddrQueryResult);
        }
        if (mTxtQueryResult)
        {
            mdns_query_results_free(mTxtQueryResult);
        }
        if (mSrvQueryHandle)
        {
            mdns_query_async_delete(mSrvQueryHandle);
        }
        if (mTxtQueryHandle)
        {
            mdns_query_async_delete(mTxtQueryHandle);
        }
    }
};

} // namespace Dnssd
} // namespace chip
