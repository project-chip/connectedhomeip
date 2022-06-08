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

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#include <dns-sd-internal.h>
#include <glib.h>
#include <platform/ThreadStackManager.h>

using namespace chip::Dnssd;
using namespace chip::DeviceLayer::Internal;

namespace {

constexpr uint8_t kDnssdKeyMaxSize = 32;

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

void RemoveContext(GenericContext * context)
{
    if (DnssdTizen::GetInstance().Remove(context) == CHIP_ERROR_KEY_NOT_FOUND)
    {
        chip::Platform::Delete(context);
    }
}

bool CheckForSuccess(GenericContext * context, int err, const char * func, bool useCallback = false)
{
    if (context == nullptr)
    {
        ChipLogError(DeviceLayer, "DNSsd %s (context is null)", func);
        return false;
    }

    if (err != DNSSD_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "DNSsd %s Err(%d)", func, err);
        if (useCallback)
        {
            switch (context->contextType)
            {
            case ContextType::Browse: {
                BrowseContext * bCtx = reinterpret_cast<BrowseContext *>(context);
                bCtx->callback(bCtx->context, nullptr, 0, CHIP_ERROR_INTERNAL);
                break;
            }
            case ContextType::Resolve: {
                ResolveContext * rCtx = reinterpret_cast<ResolveContext *>(context);
                rCtx->callback(rCtx->context, nullptr, chip::Span<chip::Inet::IPAddress>(), CHIP_ERROR_INTERNAL);
                break;
            }
            }
        }
        RemoveContext(context);
        return false;
    }
    return true;
}

void OnRegister(dnssd_error_e result, dnssd_service_h service, void * data)
{
    auto rCtx = reinterpret_cast<RegisterContext *>(data);
    auto loop = reinterpret_cast<GMainLoop *>(rCtx->mContext);

    ChipLogDetail(DeviceLayer, "DNSsd %s: name: %s, type: %s, port: %u, interfaceId: %u", __func__, rCtx->mName, rCtx->mType,
                  rCtx->mPort, rCtx->mInterfaceId);

    g_main_loop_quit(loop);

    if (result != DNSSD_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "DNSsd %s: Error: %d", __func__, result);
        rCtx->mCallback(rCtx->mCallbackContext, nullptr, GetChipError(result));
        // After this point, the context might be no longer valid
        rCtx->mInstance->UnregisterService(rCtx->mServiceHandle);
        return;
    }

    rCtx->mIsRegistered = true;
    rCtx->mCallback(rCtx->mCallbackContext, rCtx->mType, CHIP_NO_ERROR);
}

gboolean RegisterAsync(GMainLoop * mainLoop, gpointer userData)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s", __func__);

    auto rCtx      = reinterpret_cast<RegisterContext *>(userData);
    rCtx->mContext = mainLoop;

    int ret = dnssd_register_local_service(rCtx->mServiceHandle, OnRegister, rCtx);
    VerifyOrReturnError(ret == DNSSD_ERROR_NONE,
                        (ChipLogError(DeviceLayer, "dnssd_register_local_service() failed. ret: %d", ret), false));

    return true;
}

void OnBrowseAdd(BrowseContext * context, dnssd_service_h service, const char * type, const char * name, uint32_t interfaceId)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s type: %s, name: %s", __func__, type, name);

    char * tokens  = strdup(type);
    char * regtype = strtok(tokens, ".");
    chip::Inet::InterfaceId platformInterface(interfaceId);

    DnssdService mdnsService = {};
    g_strlcpy(mdnsService.mType, regtype, sizeof(mdnsService.mType));
    g_strlcpy(mdnsService.mName, name, sizeof(mdnsService.mName));
    mdnsService.mProtocol  = context->protocol;
    mdnsService.mInterface = platformInterface;

    context->services.push_back(mdnsService);
    free(tokens);

    dnssd_destroy_remote_service(service);
}

void OnBrowseRemove(BrowseContext * context, dnssd_service_h service, const char * type, const char * name, uint32_t interfaceId)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s type: %s, name: %s, interfaceId: %u", __func__, type, name, interfaceId);

    auto it = std::remove_if(
        context->services.begin(), context->services.end(), [name, type, interfaceId](const DnssdService & mdnsService) {
            return strcmp(name, mdnsService.mName) == 0 && type == GetFullType(mdnsService.mType, mdnsService.mProtocol) &&
                interfaceId == mdnsService.mInterface.GetPlatformInterface();
        });

    context->services.erase(it);

    dnssd_destroy_remote_service(service);
}

void StopBrowse(BrowseContext * context)
{
    DnssdTizen::GetInstance().Remove(context);
}

void OnBrowse(dnssd_service_state_e state, dnssd_service_h service, void * data)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s", __func__);
    auto bCtx = reinterpret_cast<BrowseContext *>(data);
    auto loop = reinterpret_cast<GMainLoop *>(bCtx->context);

    // Always stop browsing
    g_main_loop_quit(loop);

    char * type          = nullptr;
    char * name          = nullptr;
    char * ifaceName     = nullptr;
    uint32_t interfaceId = 0;

    int ret = dnssd_service_get_type(service, &type);
    VerifyOrExit(CheckForSuccess(bCtx, ret, __func__, true), );

    ret = dnssd_service_get_name(service, &name);
    VerifyOrExit(CheckForSuccess(bCtx, ret, __func__, true), );

    ret = dnssd_service_get_interface(service, &ifaceName);
    VerifyOrExit(CheckForSuccess(bCtx, ret, __func__, true), );
    interfaceId = if_nametoindex(ifaceName);
    VerifyOrExit(interfaceId > 0, );

    ChipLogDetail(DeviceLayer, "DNSsd %s type: %s, name: %s, interfaceId: %u", __func__, type, name, interfaceId);

    if (state == DNSSD_SERVICE_STATE_AVAILABLE)
    {
        OnBrowseAdd(bCtx, service, type, name, interfaceId);
    }
    else
    {
        OnBrowseRemove(bCtx, service, type, name, interfaceId);
    }

    // For now, there is no way to wait for multiple services to be found.
    // Darwin implementation just checks if kDNSServiceFlagsMoreComing is set or not,
    // but it doesn't ensure that multiple services can be found.
    // (In many cases, kDNSServiceFlagsMoreComing is not set)
    if (bCtx->callback)
    {
        bCtx->callback(bCtx->context, bCtx->services.data(), bCtx->services.size(), CHIP_NO_ERROR);
    }

    // Darwin and Linux implementations stop browsing when a Browse Callback is invoked.
    // I'm not sure if it is a proper operation.
    StopBrowse(bCtx);

exit:
    g_free(type);
    g_free(name);
    g_free(ifaceName);
}

gboolean BrowseAsync(GMainLoop * mainLoop, gpointer userData)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s", __func__);

    BrowseContext * bCtx = reinterpret_cast<BrowseContext *>(userData);
    bCtx->context        = mainLoop;

    uint32_t interfaceId = bCtx->interfaceId;
    int ret;
    dnssd_browser_h browser;
    if (interfaceId == 0)
    {
        ret = dnssd_browse_service(bCtx->type, nullptr, &browser, OnBrowse, bCtx);
    }
    else
    {
        char iface[IF_NAMESIZE + 1] = {
            0,
        };
        if (interfaceId > 0 && if_indextoname(interfaceId, iface) == nullptr)
        {
            ChipLogError(DeviceLayer, "if_indextoname() fails. errno: %d", errno);
            return false;
        }
        ret = dnssd_browse_service(bCtx->type, iface, &browser, OnBrowse, bCtx);
    }

    VerifyOrReturnError(CheckForSuccess(bCtx, ret, __func__, true), false);
    bCtx->isBrowsing = true;
    DnssdTizen::GetInstance().Add(bCtx, browser);

    return true;
}

CHIP_ERROR Browse(uint32_t interfaceId, const char * type, DnssdServiceProtocol protocol, DnssdBrowseCallback callback,
                  void * context)
{
    BrowseContext * bCtx = chip::Platform::New<BrowseContext>(protocol, type, interfaceId, callback, context);
    if (MainLoop::Instance().AsyncRequest(BrowseAsync, bCtx) == false)
    {
        chip::Platform::Delete(bCtx);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

void StopResolve(ResolveContext * context)
{
    DnssdTizen::GetInstance().Remove(context);
}

void ConvertTxtRecords(unsigned short txtLen, uint8_t * txtRecord, std::vector<TextEntry> & textEntries)
{
    if (txtLen <= 1)
    {
        ChipLogDetail(DeviceLayer, "DNSsd %s: No TXT records", __func__);
        return;
    }

    const uint8_t * ptr = txtRecord;
    const uint8_t * max = txtRecord + txtLen;
    char key[kDnssdKeyMaxSize + 1];
    char value[kDnssdTextMaxSize + 1];

    while (ptr < max)
    {
        const uint8_t * const end = ptr + 1 + ptr[0];
        if (end > max)
        {
            ChipLogError(DeviceLayer, "DNSsd %s: Invalid TXT data", __func__);
            return;
        }

        char * buf = &key[0];
        while (++ptr < end)
        {
            if (*ptr == '=')
            {
                *buf = 0;
                buf  = &value[0];
            }
            else
            {
                *buf = *ptr;
                ++buf;
            }
        }
        *buf = 0;

        auto valueLen = strlen(value);
        auto valuePtr = reinterpret_cast<const uint8_t *>(strdup(value));
        textEntries.push_back(TextEntry{ strdup(key), valuePtr, valueLen });
    }
}

void OnResolve(dnssd_error_e result, dnssd_service_h service, void * data)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s", __func__);
    auto rCtx = reinterpret_cast<ResolveContext *>(data);
    auto loop = reinterpret_cast<GMainLoop *>(rCtx->context);

    g_main_loop_quit(loop);

    dnssd_cancel_resolve_service(service);
    VerifyOrReturn(CheckForSuccess(rCtx, result, __func__, true));

    char * name           = nullptr;
    char * ipv4           = nullptr;
    char * ipv6           = nullptr;
    int port              = 0;
    unsigned short txtLen = 0;
    uint8_t * txtRecord   = nullptr;
    std::vector<TextEntry> textEntries;
    DnssdService mdnsService = {};
    chip::Inet::IPAddress ipStr;
    bool validIP = false;

    int ret = dnssd_service_get_name(service, &name);
    VerifyOrExit(CheckForSuccess(rCtx, ret, __func__, true), );

    ret = dnssd_service_get_ip(service, &ipv4, &ipv6);
    VerifyOrExit(CheckForSuccess(rCtx, ret, __func__, true), );

    ret = dnssd_service_get_port(service, &port);
    VerifyOrExit(CheckForSuccess(rCtx, ret, __func__, true), );

    ret = dnssd_service_get_all_txt_record(service, &txtLen, reinterpret_cast<void **>(&txtRecord));
    ConvertTxtRecords(txtLen, txtRecord, textEntries);
    g_free(txtRecord);

    mdnsService.mPort          = (uint16_t) port;
    mdnsService.mTextEntries   = textEntries.empty() ? nullptr : textEntries.data();
    mdnsService.mTextEntrySize = textEntries.empty() ? 0 : textEntries.size();
    g_strlcpy(mdnsService.mName, name, sizeof(mdnsService.mName));

    // If both IPv4 and IPv6 are set, IPv6 address has higher priority.
    if (ipv6 != nullptr)
    {
        validIP = (chip::Inet::IPAddress::FromString(ipv6, ipStr) && ipStr.Type() == chip::Inet::IPAddressType::kIPv6);
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (ipv4 != nullptr)
    {
        validIP = (chip::Inet::IPAddress::FromString(ipv4, ipStr) && ipStr.Type() == chip::Inet::IPAddressType::kIPv4);
    }
#endif

    ChipLogDetail(DeviceLayer, "DNSsd %s ipv4: %s, ipv6: %s, valid: %d", __func__, ipv4, ipv6, validIP);

    if (validIP)
    {
        rCtx->callback(rCtx->cbContext, &mdnsService, chip::Span<chip::Inet::IPAddress>(&ipStr, 1), CHIP_NO_ERROR);
        StopResolve(rCtx);
    }
    else
    {
        rCtx->callback(rCtx->cbContext, nullptr, chip::Span<chip::Inet::IPAddress>(), CHIP_ERROR_INTERNAL);
        RemoveContext(rCtx);
    }

exit:
    g_free(name);
    g_free(ipv4);
    g_free(ipv6);
}

gboolean ResolveAsync(GMainLoop * mainLoop, gpointer userData)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s", __func__);

    ResolveContext * rCtx = reinterpret_cast<ResolveContext *>(userData);
    rCtx->context         = mainLoop;

    int ret = dnssd_resolve_service(rCtx->service, OnResolve, rCtx);
    VerifyOrReturnError(CheckForSuccess(rCtx, ret, __func__), false);

    rCtx->isResolving = true;

    return true;
}

CHIP_ERROR Resolve(uint32_t interfaceId, const char * type, const char * name, DnssdResolveCallback callback, void * context)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s type: %s, name: %s, interfaceId: %u", __func__, type, name, interfaceId);

    ResolveContext * rCtx = chip::Platform::New<ResolveContext>(type, name, interfaceId, callback, context);

    dnssd_service_h service;
    int ret;

    if (interfaceId == 0)
    {
        ret = dnssd_create_remote_service(type, name, nullptr, &service);
    }
    else
    {
        char iface[IF_NAMESIZE + 1] = {
            0,
        };
        if (interfaceId > 0 && if_indextoname(interfaceId, iface) == nullptr)
        {
            ChipLogError(DeviceLayer, "if_indextoname() fails. errno: %d", errno);
            return CHIP_ERROR_INTERNAL;
        }
        ret = dnssd_create_remote_service(type, name, iface, &service);
    }

    VerifyOrReturnError(CheckForSuccess(rCtx, ret, __func__, true), CHIP_ERROR_INTERNAL);

    DnssdTizen::GetInstance().Add(rCtx, service);
    if (MainLoop::Instance().AsyncRequest(ResolveAsync, rCtx) == false)
    {
        chip::Platform::Delete(rCtx);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

} // namespace

namespace chip {
namespace Dnssd {

DnssdTizen DnssdTizen::sInstance;

RegisterContext::RegisterContext(DnssdTizen * instance, const char * type, const DnssdService & service,
                                 DnssdPublishCallback callback, void * context)
{
    mInstance = instance;
    Platform::CopyString(mName, service.mName);
    Platform::CopyString(mType, type);
    mInterfaceId = service.mInterface.GetPlatformInterface();
    mPort        = service.mPort;

    mCallback        = callback;
    mCallbackContext = context;
}

RegisterContext::~RegisterContext()
{
    if (mServiceHandle != 0)
    {
        if (mIsRegistered)
        {
            dnssd_deregister_local_service(mServiceHandle);
        }
        dnssd_destroy_local_service(mServiceHandle);
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

CHIP_ERROR DnssdTizen::Shutdown()
{
    int ret = dnssd_deinitialize();
    VerifyOrReturnError(ret == DNSSD_ERROR_NONE, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnssdTizen::RegisterService(const DnssdService & service, DnssdPublishCallback callback, void * context)
{
    std::string type = GetFullType(service.mType, service.mProtocol);
    auto interfaceId = service.mInterface.GetPlatformInterface();
    CHIP_ERROR err   = CHIP_NO_ERROR;
    bool ok          = false;

    ChipLogProgress(DeviceLayer, "DNSsd %s: name: %s, type: %s, interfaceId: %u, port: %u", __func__, service.mName, type.c_str(),
                    interfaceId, service.mPort);

    { // If the service was already registered, update it
        std::lock_guard<std::mutex> lock(mMutex);

        auto iServiceCtx =
            std::find_if(mRegisteredServices.begin(), mRegisteredServices.end(), [type, service, interfaceId](const auto & ctx) {
                return (strcmp(ctx->mName, service.mName) == 0 && strcmp(ctx->mType, type.c_str()) == 0 &&
                        ctx->mPort == service.mPort && ctx->mInterfaceId == interfaceId);
            });
        if (iServiceCtx != mRegisteredServices.end())
        {
            ChipLogDetail(DeviceLayer, "DNSsd %s: Updating TXT records", __func__);
            auto serviceHandle = iServiceCtx->get()->mServiceHandle;
            for (size_t i = 0; i < service.mTextEntrySize; ++i)
            {
                TextEntry entry = service.mTextEntries[i];
                VerifyOrReturnError(chip::CanCastTo<unsigned short>(entry.mDataSize), CHIP_ERROR_INVALID_ARGUMENT);
                auto dataSize = static_cast<unsigned short>(entry.mDataSize);
                int ret       = dnssd_service_add_txt_record(serviceHandle, entry.mKey, dataSize, entry.mData);
                if (ret != DNSSD_ERROR_NONE)
                {
                    ChipLogError(DeviceLayer, "dnssd_service_add_txt_record() failed. ret: %d", ret);
                    callback(context, nullptr, err = GetChipError(ret));
                }
            }
            return err;
        }
    }

    auto serviceCtx = std::make_shared<RegisterContext>(this, type.c_str(), service, callback, context);

    { // Add context to registered services list
        std::lock_guard<std::mutex> lock(mMutex);
        mRegisteredServices.emplace(serviceCtx);
    }

    // Local service will be freed by the RegisterContext destructor
    int ret            = dnssd_create_local_service(type.c_str(), &serviceCtx->mServiceHandle);
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

    ok = MainLoop::Instance().AsyncRequest(RegisterAsync, serviceCtx.get());
    VerifyOrExit(ok, err = CHIP_ERROR_INTERNAL);

exit:
    if (err != CHIP_NO_ERROR)
    {
        callback(context, nullptr, err);
        UnregisterService(serviceHandle);
    }
    return err;
}

CHIP_ERROR DnssdTizen::UnregisterService(dnssd_service_h serviceHandle)
{
    std::lock_guard<std::mutex> lock(mMutex);

    auto iServiceCtx = std::find_if(mRegisteredServices.begin(), mRegisteredServices.end(),
                                    [serviceHandle](const auto & ctx) { return ctx->mServiceHandle == serviceHandle; });
    VerifyOrReturnError(iServiceCtx != mRegisteredServices.end(), CHIP_ERROR_NOT_FOUND);

    mRegisteredServices.erase(iServiceCtx);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnssdTizen::UnregisterAllServices()
{
    ChipLogDetail(DeviceLayer, "DNSsd %s: %zu", __func__, mRegisteredServices.size());
    mRegisteredServices.clear();
    return CHIP_NO_ERROR;
}

void DnssdTizen::Delete(GenericContext * context)
{
    switch (context->contextType)
    {
    case ContextType::Browse: {
        BrowseContext * bCtx = reinterpret_cast<BrowseContext *>(context);
        if (bCtx->isBrowsing)
        {
            dnssd_cancel_browse_service(bCtx->browser);
            bCtx->isBrowsing = false;
        }
        break;
    }
    case ContextType::Resolve: {
        ResolveContext * rCtx = reinterpret_cast<ResolveContext *>(context);
        if (rCtx->isResolving)
        {
            dnssd_cancel_resolve_service(rCtx->service);
            rCtx->isResolving = false;
        }
        break;
    }
    }

    chip::Platform::Delete(context);
}

DnssdTizen::~DnssdTizen()
{
    auto iter = mContexts.cbegin();
    while (iter != mContexts.cend())
    {
        Delete(*iter);
        mContexts.erase(iter);
    }
}

CHIP_ERROR DnssdTizen::Add(BrowseContext * context, dnssd_browser_h browser)
{
    VerifyOrReturnError(context != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(browser != 0, CHIP_ERROR_INVALID_ARGUMENT);
    context->browser = browser;
    mContexts.push_back(context);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnssdTizen::Add(ResolveContext * context, dnssd_service_h service)
{
    VerifyOrReturnError(context != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    context->service = service;
    mContexts.push_back(context);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnssdTizen::Remove(GenericContext * context)
{
    ChipLogDetail(DeviceLayer, "DNSsd %s", __func__);
    auto iter = mContexts.cbegin();
    while (iter != mContexts.end())
    {
        if (*iter == context)
        {
            Delete(*iter);
            mContexts.erase(iter);
            return CHIP_NO_ERROR;
        }
        ++iter;
    }
    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    VerifyOrReturnError(initCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(errorCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return DnssdTizen::GetInstance().Init(initCallback, errorCallback, context);
}

CHIP_ERROR ChipDnssdShutdown()
{
    return DnssdTizen::GetInstance().Shutdown();
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(service->mProtocol), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    if (chip::DeviceLayer::ThreadStackMgr().IsThreadEnabled())
    {
        if (strcmp(service->mHostName, "") != 0)
        {
            chip::DeviceLayer::ThreadStackMgr().SetupSrpHost(service->mHostName);
        }

        std::string regtype = GetFullType(service->mType, service->mProtocol);
        Span<const char * const> subTypes(service->mSubTypes, service->mSubTypeSize);
        Span<const TextEntry> textEntries(service->mTextEntries, service->mTextEntrySize);
        return chip::DeviceLayer::ThreadStackMgr().AddSrpService(service->mName, regtype.c_str(), service->mPort, subTypes,
                                                                 textEntries);
    }

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    return DnssdTizen::GetInstance().RegisterService(*service, callback, context);
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    return DnssdTizen::GetInstance().UnregisterAllServices();
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interfaceId, DnssdBrowseCallback callback, void * context)
{
    VerifyOrReturnError(type != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(protocol), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    std::string regtype = GetFullType(type, protocol);

    return Browse(interfaceId.GetPlatformInterface(), regtype.c_str(), protocol, callback, context);
}

CHIP_ERROR ChipDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interfaceId, DnssdResolveCallback callback,
                            void * context)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(service->mProtocol), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    std::string regtype = GetFullType(service->mType, service->mProtocol);

    return Resolve(interfaceId.GetPlatformInterface(), regtype.c_str(), service->mName, callback, context);
}

} // namespace Dnssd
} // namespace chip
