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

#pragma once

#include <lib/dnssd/platform/Dnssd.h>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <sys/param.h>
#include <vector>

#include <dns-sd.h>
#include <glib.h>

namespace chip {
namespace Dnssd {

class DnssdTizen;

enum class ContextType
{
    Browse,
    Resolve,
};

struct GenericContext
{
    ContextType contextType;
    void * context;
};

struct RegisterContext
{
    DnssdTizen * mInstance;
    char mName[Common::kInstanceNameMaxLength + 1];
    char mType[kDnssdTypeAndProtocolMaxSize + 1];
    uint32_t mInterfaceId;
    uint16_t mPort;

    DnssdPublishCallback mCallback;
    void * mCallbackContext;

    dnssd_service_h mServiceHandle = 0;
    bool mIsRegistered             = false;
    void * mContext                = nullptr;

    RegisterContext(DnssdTizen * instance, const char * type, const DnssdService & service, DnssdPublishCallback callback,
                    void * context);
    ~RegisterContext();
};

struct BrowseContext : public GenericContext
{
    dnssd_browser_h browser;
    char type[kDnssdTypeAndProtocolMaxSize + 1];
    uint32_t interfaceId;

    std::vector<DnssdService> services;
    DnssdServiceProtocol protocol;
    bool isBrowsing;
    DnssdBrowseCallback callback;
    void * cbContext;

    BrowseContext(DnssdServiceProtocol cbContextProtocol, const char * bType, uint32_t ifId, DnssdBrowseCallback cb, void * cbCtx)
    {
        contextType = ContextType::Browse;
        protocol    = cbContextProtocol;
        callback    = cb;
        cbContext   = cbCtx;
        browser     = 0;
        g_strlcpy(type, bType, sizeof(type));
        interfaceId = ifId;
        isBrowsing  = false;
    }
};

struct ResolveContext : public GenericContext
{
    dnssd_service_h service;
    char type[kDnssdTypeAndProtocolMaxSize + 1];
    char name[Common::kInstanceNameMaxLength + 1];
    uint32_t interfaceId;
    bool isResolving;
    DnssdResolveCallback callback;
    void * cbContext;

    ResolveContext(const char * rType, const char * rName, uint32_t interface, DnssdResolveCallback cb, void * cbCtx)
    {
        contextType = ContextType::Resolve;
        g_strlcpy(type, rType, sizeof(type));
        g_strlcpy(name, rName, sizeof(name));
        interfaceId = interface;
        isResolving = false;
        callback    = cb;
        cbContext   = cbCtx;
    }
};

class DnssdTizen
{
public:
    DnssdTizen(const DnssdTizen &) = delete;
    DnssdTizen & operator=(const DnssdTizen &) = delete;
    ~DnssdTizen();

    CHIP_ERROR Init(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context);
    CHIP_ERROR Shutdown();

    CHIP_ERROR RegisterService(const DnssdService & service, DnssdPublishCallback callback, void * context);
    CHIP_ERROR UnregisterService(dnssd_service_h serviceHandle);
    CHIP_ERROR UnregisterAllServices();

    CHIP_ERROR Browse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                      chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context);

    CHIP_ERROR Resolve(const DnssdService & browseResult, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                       void * context);

    CHIP_ERROR Add(BrowseContext * context, dnssd_browser_h browser);
    CHIP_ERROR Add(ResolveContext * context, dnssd_service_h service);
    CHIP_ERROR Remove(GenericContext * context);

    static DnssdTizen & GetInstance() { return sInstance; }

private:
    DnssdTizen() = default;
    static DnssdTizen sInstance;

    void Delete(GenericContext * context);

    std::mutex mMutex;
    std::vector<GenericContext *> mContexts;
    std::set<std::shared_ptr<RegisterContext>> mRegisteredServices;
};

} // namespace Dnssd
} // namespace chip
