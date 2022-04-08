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
#include <string>
#include <sys/param.h>
#include <vector>

#include <dns-sd.h>
#include <glib.h>

namespace chip {
namespace Dnssd {

enum class ContextType
{
    Register,
    Browse,
    Resolve,
};

struct GenericContext
{
    ContextType contextType;
    void * context;
};

struct RegisterContext : public GenericContext
{
    dnssd_service_h service;
    char type[kDnssdTypeMaxSize + kDnssdProtocolTextMaxSize + 1];
    char name[Common::kInstanceNameMaxLength + 1];
    uint16_t port;
    uint32_t interfaceId;
    bool isRegistered;

    RegisterContext()
    {
        contextType  = ContextType::Register;
        context      = nullptr;
        service      = 0;
        port         = 0;
        interfaceId  = 0;
        isRegistered = false;
    }
};

struct BrowseContext : public GenericContext
{
    dnssd_browser_h browser;
    char type[kDnssdTypeMaxSize + kDnssdProtocolTextMaxSize + 1];
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
    char type[kDnssdTypeMaxSize + kDnssdProtocolTextMaxSize + 1];
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

class DnssdContexts
{
public:
    DnssdContexts(const DnssdContexts &) = delete;
    DnssdContexts & operator=(const DnssdContexts &) = delete;
    ~DnssdContexts();
    static DnssdContexts & GetInstance() { return sInstance; }

    CHIP_ERROR Add(RegisterContext * context, dnssd_service_h service, const char * type, const char * name, uint16_t port,
                   uint32_t interfaceId);
    CHIP_ERROR Add(BrowseContext * context, dnssd_browser_h browser);
    CHIP_ERROR Add(ResolveContext * context, dnssd_service_h service);
    CHIP_ERROR Remove(GenericContext * context);
    CHIP_ERROR Remove(const char * type, const char * name, uint16_t port, uint32_t interfaceId);
    CHIP_ERROR Remove(ContextType type);
    RegisterContext * Get(const char * type, const char * name, uint16_t port, uint32_t interfaceId);

private:
    DnssdContexts(){};
    static DnssdContexts sInstance;

    void Delete(GenericContext * context);
    std::vector<GenericContext *> mContexts;
};

} // namespace Dnssd
} // namespace chip
