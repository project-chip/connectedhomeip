/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

#include "DnssdHostNameRegistrar.h"
#include "DnssdError.h"
#include "DnssdImpl.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/ethernet.h>
#include <net/if_dl.h>
#include <netdb.h>

#include <set>

#include <platform/CHIPDeviceLayer.h>

constexpr DNSServiceFlags kRegisterRecordFlags = kDNSServiceFlagsShared;
constexpr uint16_t kRegisterRecordTimeoutInSeconds = 30;

namespace chip {
namespace Dnssd {

    HostNameRegistrar::~HostNameRegistrar()
    {
        Unregister();
    }

    DNSServiceErrorType HostNameRegistrar::Init(const char * hostname, Inet::IPAddressType addressType, uint32_t interfaceId)
    {
        mHostname = hostname;
        mServiceRef = nullptr;

        VerifyOrReturnError(CHIP_NO_ERROR == NetworkMonitor::Init(chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue(), addressType, interfaceId),
            kDNSServiceErr_BadState);

        return kDNSServiceErr_NoError;
    }

    CHIP_ERROR HostNameRegistrar::Register(OnRegisterRecordCallback callback)
    {
        VerifyOrReturnError(nullptr != callback, CHIP_ERROR_INVALID_ARGUMENT);

        mOnRegisterRecordCallback = callback;

        CHIP_ERROR error = CHIP_NO_ERROR;

        // If the target interface is kDNSServiceInterfaceIndexLocalOnly, just
        // register the loopback addresses.
        if (IsLocalOnly()) {
            ReturnErrorOnFailure(ResetSharedConnection());

            // Just register the loopback IPv6 address, on mInterfaceId so that the
            // resolution code finds it there.
            auto loopbackIPAddress = Inet::IPAddress::Loopback(Inet::IPAddressType::kIPv6);
            in6_addr loopbackAddr = loopbackIPAddress.ToIPv6();

            error = RegisterInterface(kDNSServiceInterfaceIndexLocalOnly, loopbackAddr, kDNSServiceType_AAAA);
        } else {
            error = StartMonitorInterfaces(
                ^(Inet::Darwin::InetInterfacesVector inetInterfaces, Inet::Darwin::Inet6InterfacesVector inet6Interfaces) {
                    ReturnOnFailure(ResetSharedConnection());
                    RegisterInterfaces(inetInterfaces, kDNSServiceType_A);
                    RegisterInterfaces(inet6Interfaces, kDNSServiceType_AAAA);
                });
        }
        ReturnErrorOnFailure(error);

        std::shared_ptr<bool> livenessTracker = mLivenessTracker;

        auto onTimeout = ^{
            VerifyOrReturn(*livenessTracker);
            VerifyOrReturn(nullptr != this->mOnRegisterRecordCallback);

            auto registerRecordCallback = this->mOnRegisterRecordCallback;
            this->mOnRegisterRecordCallback = nullptr;
            registerRecordCallback(kDNSServiceErr_Timeout);
        };
        auto & systemLayer = static_cast<System::LayerDispatch &>(chip::DeviceLayer::SystemLayer());
        auto delay = System::Clock::Seconds16(kRegisterRecordTimeoutInSeconds);
        return systemLayer.StartTimerWithBlock(onTimeout, delay);
    }

    CHIP_ERROR HostNameRegistrar::RegisterInterface(uint32_t interfaceId, uint16_t rtype, const void * rdata, uint16_t rdlen)
    {
        DNSRecordRef dnsRecordRef;
        auto err = DNSServiceRegisterRecord(mServiceRef, &dnsRecordRef, kRegisterRecordFlags, interfaceId, mHostname.c_str(), rtype,
            kDNSServiceClass_IN, rdlen, rdata, 0, OnRegisterRecord, this);
        return Error::ToChipError(err);
    }

    void HostNameRegistrar::Unregister()
    {
        if (!IsLocalOnly()) {
            NetworkMonitor::Stop();
        }
        StopSharedConnection();
        mOnRegisterRecordCallback = nullptr;
    }

    CHIP_ERROR HostNameRegistrar::StartSharedConnection()
    {
        VerifyOrReturnError(mServiceRef == nullptr, CHIP_ERROR_INCORRECT_STATE);

        auto err = DNSServiceCreateConnection(&mServiceRef);
        VerifyOrReturnValue(kDNSServiceErr_NoError == err, Error::ToChipError(err));

        err = DNSServiceSetDispatchQueue(mServiceRef, chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue());
        if (kDNSServiceErr_NoError != err) {
            StopSharedConnection();
        }

        return Error::ToChipError(err);
    }

    void HostNameRegistrar::StopSharedConnection()
    {
        if (mServiceRef != nullptr) {
            // All the DNSRecordRefs registered to the shared DNSServiceRef will be deallocated.
            DNSServiceRefDeallocate(mServiceRef);
            mServiceRef = nullptr;
        }
    }

    CHIP_ERROR HostNameRegistrar::ResetSharedConnection()
    {
        StopSharedConnection();
        ReturnLogErrorOnFailure(StartSharedConnection());
        return CHIP_NO_ERROR;
    }

    void HostNameRegistrar::OnRegisterRecord(DNSServiceRef sdRef, DNSRecordRef recordRef, DNSServiceFlags flags, DNSServiceErrorType err,
        void * context)
    {
        ChipLogProgress(Discovery, "Mdns: %s flags: %d", __func__, flags);

        if (kDNSServiceErr_NoError != err) {
            ChipLogError(Discovery, "%s (%s)", __func__, Error::ToString(err));
            return;
        }

        auto * self = static_cast<HostNameRegistrar *>(context);
        VerifyOrReturn(nullptr != self->mOnRegisterRecordCallback);
        auto registerRecordCallback = self->mOnRegisterRecordCallback;
        self->mOnRegisterRecordCallback = nullptr;
        registerRecordCallback(err);
    }

} // namespace Dnssd
} // namespace chip
