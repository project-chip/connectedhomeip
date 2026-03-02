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

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include <dns-sd.h>
#include <glib.h>

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/dnssd/Constants.h>
#include <lib/dnssd/platform/Dnssd.h>
#include <platform/GLibTypeDeleter.h>

namespace chip {
namespace Dnssd {

class DnssdTizen;

struct RegisterContext
{
    DnssdTizen & mInstance;
    char mName[Common::kInstanceNameMaxLength + 1];
    char mType[kDnssdTypeAndProtocolMaxSize + 1];
    uint32_t mInterfaceId;
    uint16_t mPort;

    DnssdPublishCallback mCallback;
    void * mCbContext;

    dnssd_service_h mServiceHandle = 0;
    bool mIsRegistered             = false;

    RegisterContext(DnssdTizen & instance, const char * type, const DnssdService & service, DnssdPublishCallback callback,
                    void * context);
    ~RegisterContext();
};

struct BrowseContext
{
    DnssdTizen & mInstance;
    char mType[kDnssdTypeAndProtocolMaxSize + 1];
    Dnssd::DnssdServiceProtocol mProtocol;
    uint32_t mInterfaceId;

    DnssdBrowseCallback mCallback;
    void * mCbContext;

    dnssd_browser_h mBrowserHandle = 0;
    // The timeout source used to stop browsing
    GSource * mTimeoutSource = nullptr;

    std::vector<DnssdService> mServices;
    bool mIsBrowsing = false;

    BrowseContext(DnssdTizen & instance, const char * type, Dnssd::DnssdServiceProtocol protocol, uint32_t interfaceId,
                  DnssdBrowseCallback callback, void * context);
    ~BrowseContext();
};

struct ResolveContext
{
    DnssdTizen & mInstance;
    char mName[Common::kInstanceNameMaxLength + 1];
    char mType[kDnssdTypeAndProtocolMaxSize + 1];
    uint32_t mInterfaceId;

    DnssdResolveCallback mCallback;
    void * mCbContext;

    dnssd_service_h mServiceHandle = 0;
    bool mIsResolving              = false;

    // Resolved service
    DnssdService mResult = {};
    GAutoPtr<char> mResultTxtRecord;
    unsigned short mResultTxtRecordLen = 0;

    ResolveContext(DnssdTizen & instance, const char * name, const char * type, uint32_t interfaceId, DnssdResolveCallback callback,
                   void * context);
    ~ResolveContext() = default;

    void Finalize(CHIP_ERROR error);
};

class DnssdTizen
{
public:
    DnssdTizen(const DnssdTizen &)             = delete;
    DnssdTizen & operator=(const DnssdTizen &) = delete;

    CHIP_ERROR Init(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context);
    void Shutdown();

    CHIP_ERROR RegisterService(const DnssdService & service, DnssdPublishCallback callback, void * context);
    CHIP_ERROR UnregisterAllServices();

    CHIP_ERROR Browse(const char * type, Dnssd::DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                      chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context);

    CHIP_ERROR Resolve(const DnssdService & browseResult, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                       void * context);

    // TODO (a.bokowy): Make these methods private
    friend class RegisterContext;
    void RemoveContext(RegisterContext * context);
    friend class BrowseContext;
    void RemoveContext(BrowseContext * context);
    friend class ResolveContext;
    void RemoveContext(ResolveContext * context);

    static DnssdTizen & GetInstance() { return sInstance; }

private:
    DnssdTizen() = default;
    static DnssdTizen sInstance;

    RegisterContext * CreateRegisterContext(const char * type, const DnssdService & service, DnssdPublishCallback callback,
                                            void * context);
    BrowseContext * CreateBrowseContext(const char * type, Dnssd::DnssdServiceProtocol protocol, uint32_t interfaceId,
                                        DnssdBrowseCallback callback, void * context);
    ResolveContext * CreateResolveContext(const char * name, const char * type, uint32_t interfaceId, DnssdResolveCallback callback,
                                          void * context);

    std::mutex mMutex;
    std::set<std::unique_ptr<RegisterContext>> mRegisterContexts;
    std::set<std::unique_ptr<BrowseContext>> mBrowseContexts;
    std::set<std::unique_ptr<ResolveContext>> mResolveContexts;
};

} // namespace Dnssd
} // namespace chip
