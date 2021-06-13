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

#include <dns_sd.h>
#include <lib/mdns/platform/Mdns.h>
#include <string>
#include <vector>

namespace chip {
namespace Mdns {

enum class ContextType
{
    Register,
    Browse,
    Resolve,
    GetAddrInfo,
};

struct GenericContext
{
    ContextType type;
    void * context;
    DNSServiceRef serviceRef;
};

struct RegisterContext : public GenericContext
{
    char mType[kMdnsTypeMaxSize + 1];
    RegisterContext(const char * sType, void * cbContext)
    {
        type = ContextType::Register;
        strncpy(mType, sType, sizeof(mType));
        context = cbContext;
    }

    bool matches(const char * sType) { return (strcmp(mType, sType) == 0); }
};

struct BrowseContext : public GenericContext
{
    MdnsBrowseCallback callback;
    std::vector<MdnsService> services;
    MdnsServiceProtocol protocol;

    BrowseContext(void * cbContext, MdnsBrowseCallback cb, MdnsServiceProtocol cbContextProtocol)
    {
        type     = ContextType::Browse;
        context  = cbContext;
        callback = cb;
        protocol = cbContextProtocol;
    }
};

struct ResolveContext : public GenericContext
{
    MdnsResolveCallback callback;

    char name[kMdnsInstanceNameMaxSize + 1];
    chip::Inet::IPAddressType addressType;

    ResolveContext(void * cbContext, MdnsResolveCallback cb, const char * cbContextName, chip::Inet::IPAddressType cbAddressType)
    {
        type     = ContextType::Resolve;
        context  = cbContext;
        callback = cb;
        strncpy(name, cbContextName, sizeof(name));
        addressType = cbAddressType;
    }
};

struct GetAddrInfoContext : public GenericContext
{
    MdnsResolveCallback callback;
    std::vector<TextEntry> textEntries;
    char name[kMdnsInstanceNameMaxSize + 1];
    uint32_t interfaceId;
    uint16_t port;

    GetAddrInfoContext(void * cbContext, MdnsResolveCallback cb, const char * cbContextName, uint32_t cbInterfaceId,
                       uint16_t cbContextPort)
    {
        type        = ContextType::GetAddrInfo;
        context     = cbContext;
        callback    = cb;
        interfaceId = cbInterfaceId;
        port        = cbContextPort;
        strncpy(name, cbContextName, sizeof(name));
    }
};

class MdnsContexts
{
public:
    MdnsContexts(const MdnsContexts &) = delete;
    MdnsContexts & operator=(const MdnsContexts &) = delete;
    ~MdnsContexts();
    static MdnsContexts & GetInstance() { return sInstance; }

    void PrepareSelect(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet, int & maxFd, timeval & timeout);
    void HandleSelectResult(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet);

    CHIP_ERROR Add(GenericContext * context, DNSServiceRef sdRef);
    CHIP_ERROR Remove(GenericContext * context);
    CHIP_ERROR Removes(ContextType type);
    CHIP_ERROR Get(ContextType type, GenericContext ** context);
    CHIP_ERROR GetRegisterType(const char * type, GenericContext ** context);

    void SetHostname(const char * name) { mHostname = name; }
    const char * GetHostname() { return mHostname.c_str(); }

private:
    MdnsContexts(){};
    static MdnsContexts sInstance;
    std::string mHostname;

    void Delete(GenericContext * context);
    std::vector<GenericContext *> mContexts;
};

} // namespace Mdns
} // namespace chip
