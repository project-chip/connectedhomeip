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
#include "MainLoop.h"

#include <algorithm>
#include <cstring>
#include <net/if.h>
#include <sstream>
#include <utility>

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#include <dns-sd-internal.h>
#include <glib.h>
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ThreadStackManager.h>
#endif

using namespace chip::Dnssd;
using namespace chip::DeviceLayer::Internal;

namespace {

constexpr uint8_t kDnssdKeyMaxSize       = 32;
constexpr const char * kEmptyAddressIpv6 = "0000:0000:0000:0000:0000:0000:0000:0000";

// The number of miliseconds which must elapse without a new "found" event before
// mDNS browsing is considered finished. We need this timeout because Tizen Native
// API does not deliver all-for-now signal (such signal is delivered by e.g. Avahi)
// and the browsing callback is called multiple times (once for each service found).
constexpr unsigned int kDnssdBrowseTimeoutMs = 250;

bool IsSupportedProtocol(DnssdServiceProtocol protocol)
{
    return (protocol == DnssdServiceProtocol::kDnssdProtocolUdp) || (protocol == DnssdServiceProtocol::kDnssdProtocolTcp);
}

const char * GetProtocolString(DnssdServiceProtocol protocol)
{
    return protocol == DnssdServiceProtocol::kDnssdProtocolUdp ? kCommissionProtocol : kOperationalProtocol;
}

std::string GetFullType(const char * type, DnssdServiceProtocol protocol)
{
    std::ostringstream typeBuilder;
    typeBuilder << type << "." << GetProtocolString(protocol);
    return typeBuilder.str();
}

CHIP_ERROR GetChipError(int dnssdError)
{
    switch (dnssdError)
    {
    case DNSSD_ERROR_NONE:
        return CHIP_NO_ERROR;
    case DNSSD_ERROR_NAME_CONFLICT:
        return CHIP_ERROR_MDNS_COLLISION;
    case DNSSD_ERROR_OUT_OF_MEMORY:
        return CHIP_ERROR_NO_MEMORY;
    default:
        return CHIP_ERROR_INTERNAL;
    }
}

void OnRegister(dnssd_error_e result, dnssd_service_h service, void * data)
{
    auto rCtx = reinterpret_cast<RegisterContext *>(data);

    ChipLogDetail(DeviceLayer, "DNSsd %s: name: %s, type: %s, port: %u, interfaceId: %u", __func__, rCtx->mName, rCtx->mType,
                  rCtx->mPort, rCtx->mInterfaceId);

    rCtx->MainLoopQuit();

    if (result != DNSSD_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "DNSsd %s: Error: %d", __func__, result);
        rCtx->mCallback(rCtx->mCbContext, nullptr, nullptr, GetChipError(result));
        // After this point, the context might be no longer valid
        rCtx->mInstance->RemoveContext(rCtx);
        return;
    }

    rCtx->mCallback(rCtx->mCbContext, rCtx->mType, rCtx->mName, CHIP_NO_ERROR);
}

gboolean RegisterAsync(GMainLoop * mainLoop, gpointer userData)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s", __func__);

    auto rCtx       = reinterpret_cast<RegisterContext *>(userData);
    rCtx->mMainLoop = mainLoop;

    int ret = dnssd_register_local_service(rCtx->mServiceHandle, OnRegister, rCtx);
    VerifyOrReturnError(ret == DNSSD_ERROR_NONE,
                        (ChipLogError(DeviceLayer, "dnssd_register_local_service() failed. ret: %d", ret), false));

    rCtx->mIsRegistered = true;
    return true;
}

gboolean OnBrowseTimeout(void * userData)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s: all for now", __func__);

    auto * bCtx = reinterpret_cast<BrowseContext *>(userData);

    bCtx->MainLoopQuit();
    bCtx->mCallback(bCtx->mCbContext, bCtx->mServices.data(), bCtx->mServices.size(), true, CHIP_NO_ERROR);

    // After this point the context might be no longer valid
    bCtx->mInstance->RemoveContext(bCtx);

    // This is a one-shot timer
    return FALSE;
}

void OnBrowseAdd(BrowseContext * context, const char * type, const char * name, uint32_t interfaceId)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s: name: %s, type: %s, interfaceId: %u", __func__, name, type, interfaceId);

    char * tokens  = strdup(type);
    char * regtype = strtok(tokens, ".");

    DnssdService dnssdService = {};
    chip::Platform::CopyString(dnssdService.mName, name);
    chip::Platform::CopyString(dnssdService.mType, regtype);
    dnssdService.mProtocol  = context->mProtocol;
    dnssdService.mInterface = chip::Inet::InterfaceId(interfaceId);

    context->mServices.push_back(dnssdService);

    free(tokens);
}

void OnBrowseRemove(BrowseContext * context, const char * type, const char * name, uint32_t interfaceId)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s: name: %s, type: %s, interfaceId: %u", __func__, name, type, interfaceId);
    context->mServices.erase(std::remove_if(
        context->mServices.begin(), context->mServices.end(), [name, type, interfaceId](const DnssdService & service) {
            return strcmp(name, service.mName) == 0 && type == GetFullType(service.mType, service.mProtocol) &&
                interfaceId == service.mInterface.GetPlatformInterface();
        }));
}

void OnBrowse(dnssd_service_state_e state, dnssd_service_h service, void * data)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s", __func__);
    auto bCtx = reinterpret_cast<BrowseContext *>(data);
    int ret;

    // If there is already a timeout source, so we need to cancel it.
    if (bCtx->mTimeoutSource != nullptr)
    {
        g_source_destroy(bCtx->mTimeoutSource);
        g_source_unref(bCtx->mTimeoutSource);
    }

    // Start a timer, so we could detect when there is no more on-browse events.
    // The timeout callback function will be called in the same event loop as the
    // browse callback (this one), so locking is not required.
    auto * source = g_timeout_source_new(kDnssdBrowseTimeoutMs);
    g_source_set_callback(source, OnBrowseTimeout, bCtx, nullptr);
    g_source_attach(source, g_main_context_get_thread_default());
    bCtx->mTimeoutSource = source;

    char * type          = nullptr;
    char * name          = nullptr;
    char * ifaceName     = nullptr;
    uint32_t interfaceId = 0;

    ret = dnssd_service_get_type(service, &type);
    VerifyOrExit(ret == DNSSD_ERROR_NONE, ChipLogError(DeviceLayer, "dnssd_service_get_type() failed. ret: %d", ret));

    ret = dnssd_service_get_name(service, &name);
    VerifyOrExit(ret == DNSSD_ERROR_NONE, ChipLogError(DeviceLayer, "dnssd_service_get_name() failed. ret: %d", ret));

    ret = dnssd_service_get_interface(service, &ifaceName);
    VerifyOrExit(ret == DNSSD_ERROR_NONE, ChipLogError(DeviceLayer, "dnssd_service_get_interface() failed. ret: %d", ret));

    interfaceId = if_nametoindex(ifaceName);
    VerifyOrExit(interfaceId > 0,
                 (ChipLogError(DeviceLayer, "if_nametoindex() failed. errno: %d", errno), ret = DNSSD_ERROR_OPERATION_FAILED));

    if (state == DNSSD_SERVICE_STATE_AVAILABLE)
    {
        OnBrowseAdd(bCtx, type, name, interfaceId);
    }
    else
    {
        OnBrowseRemove(bCtx, type, name, interfaceId);
    }

exit:

    dnssd_destroy_remote_service(service);

    if (ret != DNSSD_ERROR_NONE)
    {
        bCtx->mCallback(bCtx->mCbContext, nullptr, 0, true, GetChipError(ret));
        // After this point the context might be no longer valid
        bCtx->mInstance->RemoveContext(bCtx);
    }

    g_free(type);
    g_free(name);
    g_free(ifaceName);
}

gboolean BrowseAsync(GMainLoop * mainLoop, gpointer userData)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s", __func__);

    auto * bCtx      = reinterpret_cast<BrowseContext *>(userData);
    auto interfaceId = bCtx->mInterfaceId;
    bCtx->mMainLoop  = mainLoop;
    int ret;

    if (interfaceId == 0)
    {
        ret = dnssd_browse_service(bCtx->mType, nullptr, &bCtx->mBrowserHandle, OnBrowse, bCtx);
    }
    else
    {
        char iface[IF_NAMESIZE + 1] = "";
        VerifyOrReturnError(if_indextoname(interfaceId, iface) != nullptr,
                            (ChipLogError(DeviceLayer, "if_indextoname() failed. errno: %d", errno), false));
        ret = dnssd_browse_service(bCtx->mType, iface, &bCtx->mBrowserHandle, OnBrowse, bCtx);
    }

    if (ret != DNSSD_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "dnssd_browse_service() failed. ret: %d", ret);
        return false;
    }

    bCtx->mIsBrowsing = true;
    return true;
}

void GetTextEntries(unsigned short txtLen, uint8_t * txtRecord, std::vector<TextEntry> & textEntries)
{
    VerifyOrReturn(txtLen > 1, ChipLogDetail(DeviceLayer, "DNSsd %s: No TXT records", __func__));
    const uint8_t * txtRecordEnd = txtRecord + txtLen;

    while (txtRecord < txtRecordEnd)
    {
        uint8_t txtRecordSize = txtRecord[0];
        txtRecord++;

        VerifyOrReturn(txtRecord + txtRecordSize <= txtRecordEnd,
                       ChipLogError(DeviceLayer, "DNSsd %s: Invalid TXT data", __func__));

        for (size_t i = 0; i < txtRecordSize; i++)
        {
            if (txtRecord[i] == '=')
            {
                // NULL-terminate the key string
                txtRecord[i] = '\0';

                char * key      = reinterpret_cast<char *>(txtRecord);
                uint8_t * data  = txtRecord + i + 1;
                size_t dataSize = txtRecordSize - i - 1;
                textEntries.push_back({ key, data, dataSize });

                break;
            }
        }

        // Move to the next text entry
        txtRecord += txtRecordSize;
    }
}

void OnResolve(dnssd_error_e result, dnssd_service_h service, void * data)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s", __func__);
    auto rCtx = reinterpret_cast<ResolveContext *>(data);

    char * name           = nullptr;
    char * ipv4           = nullptr;
    char * ipv6           = nullptr;
    int port              = 0;
    char * interface      = nullptr;
    unsigned short txtLen = 0;
    uint8_t * txtRecord   = nullptr;
    std::vector<TextEntry> textEntries;
    DnssdService dnssdService = {};
    chip::Inet::IPAddress ipAddr;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // In fact, if cancel resolve fails, we can not do anything about it
    int ret = dnssd_cancel_resolve_service(service);

    rCtx->MainLoopQuit();

    ret = result;
    VerifyOrExit(ret == DNSSD_ERROR_NONE, ChipLogError(DeviceLayer, "DNSsd %s: Error: %d", __func__, ret));

    ret = dnssd_service_get_name(service, &name);
    VerifyOrExit(ret == DNSSD_ERROR_NONE, ChipLogError(DeviceLayer, "dnssd_service_get_name() failed. ret: %d", ret));

    chip::Platform::CopyString(dnssdService.mName, name);
    g_free(name);

    ret = dnssd_service_get_ip(service, &ipv4, &ipv6);
    VerifyOrExit(ret == DNSSD_ERROR_NONE, ChipLogError(DeviceLayer, "dnssd_service_get_ip() failed. ret: %d", ret));

    // If both IPv4 and IPv6 are set, IPv6 address has higher priority.
    if (ipv6 != nullptr && strcmp(ipv6, kEmptyAddressIpv6) != 0)
    {
        if (!chip::Inet::IPAddress::FromString(ipv6, ipAddr) || ipAddr.Type() != chip::Inet::IPAddressType::kIPv6)
        {
            ret = DNSSD_ERROR_OPERATION_FAILED;
        }
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (ipv4 != nullptr)
    {
        if (!chip::Inet::IPAddress::FromString(ipv4, ipAddr) || ipAddr.Type() != chip::Inet::IPAddressType::kIPv4)
        {
            ret = DNSSD_ERROR_OPERATION_FAILED;
        }
    }
#endif

    ChipLogDetail(DeviceLayer, "DNSsd %s: IPv4: %s, IPv6: %s, ret: %d", __func__, ipv4, ipv6, ret);

    g_free(ipv4);
    g_free(ipv6);

    VerifyOrExit(ret == DNSSD_ERROR_NONE, );

    ret = dnssd_service_get_port(service, &port);
    VerifyOrExit(ret == DNSSD_ERROR_NONE, ChipLogError(DeviceLayer, "dnssd_service_get_port() failed. ret: %d", ret));

    dnssdService.mPort = static_cast<uint16_t>(port);

    ret = dnssd_service_get_interface(service, &interface);
    VerifyOrExit(ret == DNSSD_ERROR_NONE, ChipLogError(DeviceLayer, "dnssd_service_get_interface() failed. ret: %d", ret));

    err = chip::Inet::InterfaceId::InterfaceNameToId(interface, dnssdService.mInterface);
    VerifyOrExit(
        err == CHIP_NO_ERROR,
        ChipLogError(DeviceLayer, "chip::Inet::InterfaceId::InterfaceNameToId() failed. ret: %" CHIP_ERROR_FORMAT, err.Format()));

    ret = dnssd_service_get_all_txt_record(service, &txtLen, reinterpret_cast<void **>(&txtRecord));
    VerifyOrExit(ret == DNSSD_ERROR_NONE, ChipLogError(DeviceLayer, "dnssd_service_get_all_txt_record() failed. ret: %d", ret));

    GetTextEntries(txtLen, txtRecord, textEntries);
    dnssdService.mTextEntries   = textEntries.empty() ? nullptr : textEntries.data();
    dnssdService.mTextEntrySize = textEntries.size();

    { // Lock the stack mutex when calling the callback function, so that the callback
      // function could safely perform message exchange (e.g. PASE session pairing).
        chip::DeviceLayer::StackLock lock;
        rCtx->mCallback(rCtx->mCbContext, &dnssdService, chip::Span<chip::Inet::IPAddress>(&ipAddr, 1), CHIP_NO_ERROR);
    }

    g_free(txtRecord);

    rCtx->mInstance->RemoveContext(rCtx);
    return;

exit:
    if (err == CHIP_NO_ERROR)
    {
        rCtx->mCallback(rCtx->mCbContext, nullptr, chip::Span<chip::Inet::IPAddress>(), GetChipError(ret));
    }
    else
    {
        rCtx->mCallback(rCtx->mCbContext, nullptr, chip::Span<chip::Inet::IPAddress>(), err);
    }
    rCtx->mInstance->RemoveContext(rCtx);
}

gboolean ResolveAsync(GMainLoop * mainLoop, gpointer userData)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s", __func__);

    auto * rCtx     = reinterpret_cast<ResolveContext *>(userData);
    rCtx->mMainLoop = mainLoop;

    int ret = dnssd_resolve_service(rCtx->mServiceHandle, OnResolve, rCtx);
    VerifyOrReturnError(ret == DNSSD_ERROR_NONE,
                        (ChipLogError(DeviceLayer, "dnssd_resolve_service() failed. ret: %d", ret), false));

    rCtx->mIsResolving = true;
    return true;
}

} // namespace

namespace chip {
namespace Dnssd {

DnssdTizen DnssdTizen::sInstance;

void GenericContext::MainLoopQuit()
{
    VerifyOrReturn(mMainLoop != nullptr, );
    g_main_loop_quit(std::exchange(mMainLoop, nullptr));
}

RegisterContext::RegisterContext(DnssdTizen * instance, const char * type, const DnssdService & service,
                                 DnssdPublishCallback callback, void * context) :
    GenericContext(ContextType::Register, instance)
{
    Platform::CopyString(mName, service.mName);
    Platform::CopyString(mType, type);
    mInterfaceId = service.mInterface.GetPlatformInterface();
    mPort        = service.mPort;

    mCallback  = callback;
    mCbContext = context;
}

RegisterContext::~RegisterContext()
{
    if (mIsRegistered)
    {
        dnssd_deregister_local_service(mServiceHandle);
    }
    if (mServiceHandle != 0)
    {
        dnssd_destroy_local_service(mServiceHandle);
    }
}

BrowseContext::BrowseContext(DnssdTizen * instance, const char * type, DnssdServiceProtocol protocol, uint32_t interfaceId,
                             DnssdBrowseCallback callback, void * context) :
    GenericContext(ContextType::Browse, instance)
{
    Platform::CopyString(mType, type);
    mProtocol    = protocol;
    mInterfaceId = interfaceId;

    mCallback  = callback;
    mCbContext = context;
}

BrowseContext::~BrowseContext()
{
    if (mTimeoutSource != nullptr)
    {
        g_source_destroy(mTimeoutSource);
        g_source_unref(mTimeoutSource);
    }
    if (mIsBrowsing)
    {
        dnssd_cancel_browse_service(mBrowserHandle);
    }
}

ResolveContext::ResolveContext(DnssdTizen * instance, const char * name, const char * type, uint32_t interfaceId,
                               DnssdResolveCallback callback, void * context) :
    GenericContext(ContextType::Resolve, instance)
{
    Platform::CopyString(mName, name);
    Platform::CopyString(mType, type);
    mInterfaceId = interfaceId;

    mCallback  = callback;
    mCbContext = context;
}

ResolveContext::~ResolveContext()
{
    if (mIsResolving)
    {
        dnssd_cancel_resolve_service(mServiceHandle);
    }
}

CHIP_ERROR DnssdTizen::Init(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    int ret = dnssd_initialize();
    VerifyOrExit(ret == DNSSD_ERROR_NONE || ret == DNSSD_ERROR_INVALID_OPERATION /* Already initialized */, );

    initCallback(context, CHIP_NO_ERROR);
    return CHIP_NO_ERROR;

exit:
    errorCallback(context, CHIP_ERROR_INTERNAL);
    return CHIP_ERROR_INTERNAL;
}

void DnssdTizen::Shutdown()
{
    int ret = dnssd_deinitialize();
    if (ret != DNSSD_ERROR_NONE)
        ChipLogError(DeviceLayer, "DNSsd %s: Error: %d", __func__, ret);
}

CHIP_ERROR DnssdTizen::RegisterService(const DnssdService & service, DnssdPublishCallback callback, void * context)
{
    std::string fullType = GetFullType(service.mType, service.mProtocol);
    auto interfaceId     = service.mInterface.GetPlatformInterface();
    CHIP_ERROR err       = CHIP_NO_ERROR;
    bool ok              = false;

    ChipLogProgress(DeviceLayer, "DNSsd %s: name: %s, type: %s, interfaceId: %u, port: %u", __func__, service.mName,
                    fullType.c_str(), interfaceId, service.mPort);

    { // If the service was already registered, update it
        std::lock_guard<std::mutex> lock(mMutex);

        auto iServiceCtx = std::find_if(mContexts.begin(), mContexts.end(), [fullType, service, interfaceId](const auto & ctx) {
            VerifyOrReturnError(ctx->mContextType == ContextType::Register, false);
            auto * rCtx = static_cast<RegisterContext *>(ctx.get());
            return strcmp(rCtx->mName, service.mName) == 0 && strcmp(rCtx->mType, fullType.c_str()) == 0 &&
                rCtx->mPort == service.mPort && rCtx->mInterfaceId == interfaceId;
        });
        if (iServiceCtx != mContexts.end())
        {
            ChipLogDetail(DeviceLayer, "DNSsd %s: Updating TXT records", __func__);
            auto serviceHandle = static_cast<RegisterContext *>(iServiceCtx->get())->mServiceHandle;

            for (size_t i = 0; i < service.mTextEntrySize; ++i)
            {
                TextEntry entry = service.mTextEntries[i];
                VerifyOrReturnError(chip::CanCastTo<unsigned short>(entry.mDataSize), CHIP_ERROR_INVALID_ARGUMENT);
                auto dataSize = static_cast<unsigned short>(entry.mDataSize);
                int ret       = dnssd_service_add_txt_record(serviceHandle, entry.mKey, dataSize, entry.mData);
                if (ret != DNSSD_ERROR_NONE)
                {
                    ChipLogError(DeviceLayer, "dnssd_service_add_txt_record() failed. ret: %d", ret);
                    callback(context, nullptr, nullptr, err = GetChipError(ret));
                }
            }

            return err;
        }
    }

    auto serviceCtx = CreateRegisterContext(fullType.c_str(), service, callback, context);

    // Local service will be freed by the RegisterContext destructor
    int ret            = dnssd_create_local_service(fullType.c_str(), &serviceCtx->mServiceHandle);
    auto serviceHandle = serviceCtx->mServiceHandle;
    VerifyOrExit(ret == DNSSD_ERROR_NONE,
                 (ChipLogError(DeviceLayer, "dnssd_create_local_service() failed. ret: %d", ret), err = GetChipError(ret)));

    ret = dnssd_service_set_name(serviceHandle, service.mName);
    VerifyOrExit(ret == DNSSD_ERROR_NONE,
                 (ChipLogError(DeviceLayer, "dnssd_service_set_name() failed. ret: %d", ret), err = GetChipError(ret)));

    ret = dnssd_service_set_port(serviceHandle, service.mPort);
    VerifyOrExit(ret == DNSSD_ERROR_NONE,
                 (ChipLogError(DeviceLayer, "dnssd_service_set_port() failed. ret: %d", ret), err = GetChipError(ret)));

    if (interfaceId > 0)
    {
        char iface[IF_NAMESIZE + 1] = "";
        VerifyOrExit(if_indextoname(interfaceId, iface) != nullptr,
                     (ChipLogError(DeviceLayer, "if_indextoname() failed. errno: %d", errno), err = CHIP_ERROR_INTERNAL));
        ret = dnssd_service_set_interface(serviceHandle, iface);
        VerifyOrExit(ret == DNSSD_ERROR_NONE,
                     (ChipLogError(DeviceLayer, "dnssd_service_set_interface() failed. ret: %d", ret), err = GetChipError(ret)));
    }

    for (size_t i = 0; i < service.mTextEntrySize; ++i)
    {
        TextEntry entry = service.mTextEntries[i];
        VerifyOrReturnError(chip::CanCastTo<unsigned short>(entry.mDataSize), CHIP_ERROR_INVALID_ARGUMENT);
        ret = dnssd_service_add_txt_record(serviceHandle, entry.mKey, static_cast<unsigned short>(entry.mDataSize), entry.mData);
        VerifyOrExit(ret == DNSSD_ERROR_NONE,
                     (ChipLogError(DeviceLayer, "dnssd_service_add_txt_record() failed. ret: %d", ret), err = GetChipError(ret)));
    }

    ok = MainLoop::Instance().AsyncRequest(RegisterAsync, serviceCtx);
    VerifyOrExit(ok, err = CHIP_ERROR_INTERNAL);

exit:
    if (err != CHIP_NO_ERROR)
    { // Notify caller about error
        callback(context, nullptr, nullptr, err);
        RemoveContext(serviceCtx);
    }
    return err;
}

CHIP_ERROR DnssdTizen::UnregisterAllServices()
{
    std::lock_guard<std::mutex> lock(mMutex);

    unsigned int numServices = 0;
    for (auto it = mContexts.begin(); it != mContexts.end(); it++)
    {
        if ((*it)->mContextType == ContextType::Register)
        {
            mContexts.erase(it--);
            numServices++;
        }
    }

    ChipLogDetail(DeviceLayer, "DNSsd %s: %u", __func__, numServices);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnssdTizen::Browse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                              chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context)
{
    std::string fullType = GetFullType(type, protocol);
    auto interfaceId     = interface.GetPlatformInterface();
    CHIP_ERROR err       = CHIP_NO_ERROR;

    auto browseCtx = CreateBrowseContext(fullType.c_str(), protocol, interfaceId, callback, context);

    bool ok = MainLoop::Instance().AsyncRequest(BrowseAsync, browseCtx);
    VerifyOrExit(ok, err = CHIP_ERROR_INTERNAL);

exit:
    if (err != CHIP_NO_ERROR)
    { // Notify caller about error
        callback(context, nullptr, 0, true, err);
        RemoveContext(browseCtx);
    }
    return err;
}

CHIP_ERROR DnssdTizen::Resolve(const DnssdService & browseResult, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                               void * context)
{
    std::string fullType = GetFullType(browseResult.mType, browseResult.mProtocol);
    auto interfaceId     = interface.GetPlatformInterface();
    CHIP_ERROR err       = CHIP_NO_ERROR;
    bool ok              = false;
    int ret;

    ChipLogDetail(DeviceLayer, "DNSsd %s: name: %s, type: %s, interfaceId: %u", __func__, browseResult.mName, fullType.c_str(),
                  interfaceId);

    auto resolveCtx = CreateResolveContext(browseResult.mName, fullType.c_str(), interfaceId, callback, context);

    if (interfaceId == 0)
    {
        ret = dnssd_create_remote_service(fullType.c_str(), browseResult.mName, nullptr, &resolveCtx->mServiceHandle);
    }
    else
    {
        char iface[IF_NAMESIZE + 1] = "";
        VerifyOrExit(if_indextoname(interfaceId, iface) != nullptr,
                     (ChipLogError(DeviceLayer, "if_indextoname() failed. errno: %d", errno), err = CHIP_ERROR_INTERNAL));
        ret = dnssd_create_remote_service(fullType.c_str(), browseResult.mName, iface, &resolveCtx->mServiceHandle);
    }

    VerifyOrExit(ret == DNSSD_ERROR_NONE,
                 (ChipLogError(DeviceLayer, "dnssd_create_remote_service() failed. ret: %d", ret), err = GetChipError(ret)));

    ok = MainLoop::Instance().AsyncRequest(ResolveAsync, resolveCtx);
    VerifyOrExit(ok, err = CHIP_ERROR_INTERNAL);

exit:
    if (err != CHIP_NO_ERROR)
        RemoveContext(resolveCtx);
    return err;
}

RegisterContext * DnssdTizen::CreateRegisterContext(const char * type, const DnssdService & service, DnssdPublishCallback callback,
                                                    void * context)
{
    auto ctx    = std::make_unique<RegisterContext>(this, type, service, callback, context);
    auto ctxPtr = ctx.get();

    std::lock_guard<std::mutex> lock(mMutex);
    mContexts.emplace(std::move(ctx));

    return ctxPtr;
}

BrowseContext * DnssdTizen::CreateBrowseContext(const char * type, DnssdServiceProtocol protocol, uint32_t interfaceId,
                                                DnssdBrowseCallback callback, void * context)
{
    auto ctx    = std::make_unique<BrowseContext>(this, type, protocol, interfaceId, callback, context);
    auto ctxPtr = ctx.get();

    std::lock_guard<std::mutex> lock(mMutex);
    mContexts.emplace(std::move(ctx));

    return ctxPtr;
}

ResolveContext * DnssdTizen::CreateResolveContext(const char * name, const char * type, uint32_t interfaceId,
                                                  DnssdResolveCallback callback, void * context)
{
    auto ctx    = std::make_unique<ResolveContext>(this, name, type, interfaceId, callback, context);
    auto ctxPtr = ctx.get();

    std::lock_guard<std::mutex> lock(mMutex);
    mContexts.emplace(std::move(ctx));

    return ctxPtr;
}

CHIP_ERROR DnssdTizen::RemoveContext(GenericContext * context)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mContexts.erase(std::find_if(mContexts.begin(), mContexts.end(), [context](const auto & ctx) { return ctx.get() == context; }));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    VerifyOrReturnError(initCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(errorCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return DnssdTizen::GetInstance().Init(initCallback, errorCallback, context);
}

void ChipDnssdShutdown()
{
    DnssdTizen::GetInstance().Shutdown();
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(service->mProtocol), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    if (DeviceLayer::ThreadStackMgr().IsThreadEnabled())
    {
        std::string regtype = GetFullType(service->mType, service->mProtocol);
        Span<const char * const> subTypes(service->mSubTypes, service->mSubTypeSize);
        Span<const TextEntry> textEntries(service->mTextEntries, service->mTextEntrySize);
        CHIP_ERROR err;

        if (strcmp(service->mHostName, "") != 0)
        {
            err = DeviceLayer::ThreadStackMgr().SetupSrpHost(service->mHostName);
            VerifyOrReturnError(err == CHIP_NO_ERROR, err);
        }

        err = DeviceLayer::ThreadStackMgr().AddSrpService(service->mName, regtype.c_str(), service->mPort, subTypes, textEntries);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);

        return CHIP_NO_ERROR;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    return DnssdTizen::GetInstance().RegisterService(*service, callback, context);
}

CHIP_ERROR ChipDnssdRemoveServices()
{

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    if (DeviceLayer::ThreadStackMgr().IsThreadEnabled())
    {
        DeviceLayer::ThreadStackMgr().InvalidateAllSrpServices();
        return DeviceLayer::ThreadStackMgr().RemoveInvalidSrpServices();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    return DnssdTizen::GetInstance().UnregisterAllServices();
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                           intptr_t * browseIdentifier)
{
    VerifyOrReturnError(type != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(protocol), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    *browseIdentifier = reinterpret_cast<intptr_t>(nullptr);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
    if (DeviceLayer::ThreadStackMgr().IsThreadEnabled())
    {
        std::string fullType = GetFullType(type, protocol);
        return DeviceLayer::ThreadStackMgr().DnsBrowse(fullType.c_str(), callback, context);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT

    return DnssdTizen::GetInstance().Browse(type, protocol, addressType, interface, callback, context);
}

CHIP_ERROR ChipDnssdStopBrowse(intptr_t browseIdentifier)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipDnssdResolve(DnssdService * browseResult, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                            void * context)
{
    VerifyOrReturnError(browseResult != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(browseResult->mProtocol), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT
    if (DeviceLayer::ThreadStackMgr().IsThreadEnabled())
    {
        std::string fullType = GetFullType(browseResult->mType, browseResult->mProtocol);
        return DeviceLayer::ThreadStackMgr().DnsResolve(fullType.c_str(), browseResult->mName, callback, context);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT && CHIP_DEVICE_CONFIG_ENABLE_THREAD_DNS_CLIENT

    return DnssdTizen::GetInstance().Resolve(*browseResult, interface, callback, context);
}

CHIP_ERROR ChipDnssdReconfirmRecord(const char * hostname, chip::Inet::IPAddress address, chip::Inet::InterfaceId interface)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Dnssd
} // namespace chip
