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

#ifdef __no_stub__
#include <dns_sd.h>
#endif /* __no_stub__ */
#include <lib/dnssd/platform/Dnssd.h>

#include <map>
#include <string>
#include <vector>

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
    ContextType type;
    void * context;
#ifdef __no_stub__
    DNSServiceRef serviceRef;
#endif /* __no_stub__ */
    uint32_t mSelectCount;

    virtual ~GenericContext() {}

#ifdef __no_stub__
    CHIP_ERROR Finalize(DNSServiceErrorType err = kDNSServiceErr_NoError);
    virtual void DispatchFailure(DNSServiceErrorType err) = 0;
#endif /* __no_stub__ */
    virtual void DispatchSuccess()                        = 0;
};

struct RegisterContext;

class MdnsContexts
{
public:
    MdnsContexts(const MdnsContexts &)             = delete;
    MdnsContexts & operator=(const MdnsContexts &) = delete;
    ~MdnsContexts();
    static MdnsContexts & GetInstance() { return sInstance; }

#ifdef __no_stub__
    CHIP_ERROR Add(GenericContext * context, DNSServiceRef sdRef);
#endif /* __no_stub__ */
    CHIP_ERROR Remove(GenericContext * context);
    CHIP_ERROR RemoveAllOfType(ContextType type);
    CHIP_ERROR Has(GenericContext * context);
    GenericContext * GetBySockFd(int fd);
    int GetSelectFd(fd_set * pSelectFd);

    /**
     * @brief
     *   Returns a pointer to a RegisterContext that has previously been registered
     *   with a given type.
     *
     * @param[in]  type     A service type. Service type are composed of
     *                      of the service name, the service protocol, and the PTR records.
     *                      Example:
     *                        _matterc._udp,_V65521,_S15,_L3840,_CM
     *                        _matter._tcp,_I4CEEAD044CC35B63
     * @param[out] context  A reference to the context previously registered
     *
     * @return     On success, the context parameter will point to the previously
     *             registered context.
     */
    CHIP_ERROR GetRegisterContextOfType(const char * type, RegisterContext ** context);

    void Delete(GenericContext * context);

private:
    MdnsContexts(){};
    static MdnsContexts sInstance;

    std::vector<GenericContext *> mContexts;
};

struct RegisterContext : public GenericContext
{
    DnssdPublishCallback callback;
    std::string mType;
    std::string mInstanceName;

    RegisterContext(const char * sType, const char * instanceName, DnssdPublishCallback cb, void * cbContext);
    virtual ~RegisterContext() {}

#ifdef __no_stub__
    void DispatchFailure(DNSServiceErrorType err) override;
#endif /* __no_stub__ */
    void DispatchSuccess() override;

    bool matches(const char * sType) { return mType.compare(sType) == 0; }
};

struct BrowseContext : public GenericContext
{
    DnssdBrowseCallback callback;
    std::vector<DnssdService> services;
    DnssdServiceProtocol protocol;

    BrowseContext(void * cbContext, DnssdBrowseCallback cb, DnssdServiceProtocol cbContextProtocol);
    virtual ~BrowseContext() {}

#ifdef __no_stub__
    void DispatchFailure(DNSServiceErrorType err) override;
#endif /* __no_stub__ */
    void DispatchSuccess() override;
};

struct InterfaceInfo
{
    InterfaceInfo();
    InterfaceInfo(InterfaceInfo && other);
    // Copying is not safe, because DnssdService bits need to be
    // copied/deallocated properly.
    InterfaceInfo(const InterfaceInfo & other) = delete;
    ~InterfaceInfo();

    DnssdService service;
    std::vector<Inet::IPAddress> addresses;
    std::string fullyQualifiedDomainName;
};

struct ResolveContext : public GenericContext
{
    DnssdResolveCallback callback;
    std::map<uint32_t, InterfaceInfo> interfaces;
#ifdef __no_stub__
    DNSServiceProtocol protocol;
#endif /* __no_stub__ */

    ResolveContext(void * cbContext, DnssdResolveCallback cb, chip::Inet::IPAddressType cbAddressType);
    virtual ~ResolveContext();

#ifdef __no_stub__
    void DispatchFailure(DNSServiceErrorType err) override;
#endif /* __no_stub__ */
    void DispatchSuccess() override;

    CHIP_ERROR OnNewAddress(uint32_t interfaceId, const struct sockaddr * address);
    CHIP_ERROR OnNewLocalOnlyAddress();
    bool HasAddress();

    void OnNewInterface(uint32_t interfaceId, const char * fullname, const char * hostname, uint16_t port, uint16_t txtLen,
                        const unsigned char * txtRecord);
    bool HasInterface();
};
} // namespace Dnssd
} // namespace chip
