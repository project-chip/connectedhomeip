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
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#include <dns-sd-internal.h>
#include <glib.h>

using namespace chip::Dnssd;
using namespace chip::DeviceLayer::Internal;

namespace {

constexpr uint8_t kDnssdKeyMaxSize = 32;

bool IsSupportedProtocol(DnssdServiceProtocol protocol)
{
    return (protocol == DnssdServiceProtocol::kDnssdProtocolUdp) || (protocol == DnssdServiceProtocol::kDnssdProtocolTcp);
}

std::string GetFullType(const char * type, DnssdServiceProtocol protocol)
{
    std::ostringstream typeBuilder;
    typeBuilder << type;
    typeBuilder << "." << (protocol == DnssdServiceProtocol::kDnssdProtocolUdp ? kCommissionProtocol : kOperationalProtocol);
    return typeBuilder.str();
}

void RemoveContext(GenericContext * context)
{
    if (DnssdContexts::GetInstance().Remove(context) == CHIP_ERROR_KEY_NOT_FOUND)
    {
        chip::Platform::Delete(context);
    }
}

bool CheckForSuccess(GenericContext * context, int err, const char * func, bool useCallback = false)
{
    if (context == nullptr)
    {
        ChipLogError(DeviceLayer, "%s (Dnssd context is null)", func);
        return false;
    }

    if (err != DNSSD_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "%s Err(%d)", func, err);
        if (useCallback)
        {
            switch (context->contextType)
            {
            case ContextType::Register: {
                break;
            }
            case ContextType::Browse: {
                BrowseContext * bCtx = reinterpret_cast<BrowseContext *>(context);
                bCtx->callback(bCtx->context, nullptr, 0, CHIP_ERROR_INTERNAL);
                break;
            }
            case ContextType::Resolve: {
                ResolveContext * rCtx = reinterpret_cast<ResolveContext *>(context);
                rCtx->callback(rCtx->context, nullptr, CHIP_ERROR_INTERNAL);
                break;
            }
            }
        }
        RemoveContext(context);
        return false;
    }
    return true;
}

CHIP_ERROR Initialize()
{
    int ret = dnssd_initialize();

    VerifyOrReturnError(ret == DNSSD_ERROR_NONE || ret == DNSSD_ERROR_INVALID_OPERATION /* Already initialized */,
                        CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR UpdateTXTRecord(dnssd_service_h service, TextEntry * textEntries, size_t textEntrySize)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    // TODO
    return CHIP_NO_ERROR;
}

void OnRegister(dnssd_error_e error, dnssd_service_h service, void * data)
{
    ChipLogDetail(DeviceLayer, "Dnssd: %s", __func__);
    RegisterContext * rCtx = reinterpret_cast<RegisterContext *>(data);
    GMainLoop * loop       = (GMainLoop *) rCtx->context;

    g_main_loop_quit(loop);

    VerifyOrReturn(CheckForSuccess(rCtx, (int) error, __func__));
    rCtx->isRegistered = true;
    ChipLogDetail(DeviceLayer, "Dnssd: %s name: %s, type: %s, port: %u, interfaceId: %u", __func__, rCtx->name, rCtx->type,
                  rCtx->port, rCtx->interfaceId);
}

gboolean RegisterAsync(GMainLoop * mainLoop, gpointer userData)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    RegisterContext * rCtx = reinterpret_cast<RegisterContext *>(userData);
    rCtx->context          = mainLoop;

    int ret = dnssd_register_local_service(rCtx->service, OnRegister, rCtx);
    VerifyOrReturnError(CheckForSuccess(rCtx, ret, __func__), false);

    return true;
}

CHIP_ERROR RegisterService(const char * type, const char * name, uint16_t port, uint32_t interfaceId, TextEntry * textEntries,
                           size_t textEntrySize)
{
    VerifyOrReturnError(type != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(textEntrySize <= kDnssdTextMaxSize, CHIP_ERROR_INVALID_ARGUMENT);

    char iface[IF_NAMESIZE + 1] = {
        0,
    };
    if (interfaceId > 0 && if_indextoname(interfaceId, iface) == nullptr)
    {
        ChipLogError(DeviceLayer, "if_indextoname() fails. errno: %d", errno);
        return CHIP_ERROR_INTERNAL;
    }

    auto context = DnssdContexts::GetInstance().Get(type, name, port, interfaceId);
    if (context != nullptr)
    {
        return UpdateTXTRecord(context->service, textEntries, textEntrySize);
    }

    ChipLogProgress(DeviceLayer, "%s type: %s, name: %s, port: %u, interfaceId: %u", __func__, type, name, port, interfaceId);

    context = chip::Platform::New<RegisterContext>();

    dnssd_service_h service;
    int ret = dnssd_create_local_service(type, &service);
    VerifyOrReturnError(CheckForSuccess(context, ret, __func__), CHIP_ERROR_INTERNAL);

    ret = dnssd_service_set_name(service, name);
    VerifyOrReturnError(CheckForSuccess(context, ret, __func__), CHIP_ERROR_INTERNAL);

    ret = dnssd_service_set_port(service, port);
    VerifyOrReturnError(CheckForSuccess(context, ret, __func__), CHIP_ERROR_INTERNAL);

    if (interfaceId > 0)
    {
        ret = dnssd_service_set_interface(service, iface);
        VerifyOrReturnError(CheckForSuccess(context, ret, __func__), CHIP_ERROR_INTERNAL);
    }

    for (size_t i = 0; i < textEntrySize; ++i)
    {
        TextEntry entry = textEntries[i];
        if (!chip::CanCastTo<uint8_t>(entry.mDataSize))
        {
            RemoveContext(context);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        ret = dnssd_service_add_txt_record(service, entry.mKey, static_cast<uint8_t>(entry.mDataSize), entry.mData);
        VerifyOrReturnError(CheckForSuccess(context, ret, __func__), CHIP_ERROR_INTERNAL);
    }

    DnssdContexts::GetInstance().Add(context, service, type, name, port, interfaceId);
    if (MainLoop::Instance().AsyncRequest(RegisterAsync, context) == false)
    {
        chip::Platform::Delete(context);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR UnregisterAllServices()
{
    CHIP_ERROR err = DnssdContexts::GetInstance().Remove(ContextType::Register);
    if (err == CHIP_ERROR_KEY_NOT_FOUND)
        err = CHIP_NO_ERROR;
    return err;
}

void OnBrowseAdd(BrowseContext * context, dnssd_service_h service, const char * type, const char * name, uint32_t interfaceId)
{
    ChipLogDetail(DeviceLayer, "Dnssd: %s  type: %s, name: %s", __func__, type, name);

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
    ChipLogDetail(DeviceLayer, "Dnssd: %s  type: %s, name: %s, interfaceId: %u", __func__, type, name, interfaceId);

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
    DnssdContexts::GetInstance().Remove(context);
}

void OnBrowse(dnssd_service_state_e state, dnssd_service_h service, void * data)
{
    ChipLogDetail(DeviceLayer, "Dnssd: %s", __func__);
    BrowseContext * bCtx = reinterpret_cast<BrowseContext *>(data);
    GMainLoop * loop     = (GMainLoop *) bCtx->context;

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

    ChipLogDetail(DeviceLayer, "Dnssd: %s type: %s, name: %s, interfaceId: %u", __func__, type, name, interfaceId);

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
    ChipLogProgress(DeviceLayer, "%s", __func__);

    BrowseContext * bCtx = reinterpret_cast<BrowseContext *>(userData);
    bCtx->context        = mainLoop;

    uint32_t interfaceId = bCtx->interfaceId;
    int ret;
    dnssd_browser_h browser;
    if (interfaceId == 0)
    {
        ret = dnssd_browse_service(bCtx->type, NULL, &browser, OnBrowse, bCtx);
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
    DnssdContexts::GetInstance().Add(bCtx, browser);

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
    DnssdContexts::GetInstance().Remove(context);
}

void ConvertTxtRecords(unsigned short txtLen, uint8_t * txtRecord, std::vector<TextEntry> & textEntries)
{
    if (txtLen <= 1)
    {
        ChipLogDetail(DeviceLayer, "Dnssd: %s No TXT records", __func__);
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
            ChipLogError(DeviceLayer, "Dnssd: %s Invalid TXT data", __func__);
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
    ChipLogDetail(DeviceLayer, "Dnssd: %s", __func__);
    ResolveContext * rCtx = reinterpret_cast<ResolveContext *>(data);
    GMainLoop * loop      = (GMainLoop *) rCtx->context;

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

    ret = dnssd_service_get_all_txt_record(service, &txtLen, (void **) &txtRecord);
    ConvertTxtRecords(txtLen, txtRecord, textEntries);
    g_free(txtRecord);

    mdnsService.mPort          = (uint16_t) port;
    mdnsService.mTextEntries   = textEntries.empty() ? nullptr : textEntries.data();
    mdnsService.mTextEntrySize = textEntries.empty() ? 0 : textEntries.size();
    g_strlcpy(mdnsService.mName, name, sizeof(mdnsService.mName));

    // If both IPv4 and IPv6 are set, IPv6 address has higher priority.
    if (ipv6 != nullptr)
    {
        validIP = chip::Inet::IPAddress::FromString(ipv6, ipStr);
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (ipv4 != nullptr)
    {
        validIP = chip::Inet::IPAddress::FromString(ipv4, ipStr);
    }
#endif

    if (validIP)
    {
        mdnsService.mAddress.SetValue(ipStr);
        rCtx->callback(rCtx->cbContext, &mdnsService, CHIP_NO_ERROR);
        StopResolve(rCtx);
    }
    else
    {
        rCtx->callback(rCtx->cbContext, nullptr, CHIP_ERROR_INTERNAL);
        RemoveContext(rCtx);
    }

exit:
    g_free(name);
    g_free(ipv4);
    g_free(ipv6);
}

gboolean ResolveAsync(GMainLoop * mainLoop, gpointer userData)
{
    ChipLogProgress(DeviceLayer, "Dnssd %s", __func__);

    ResolveContext * rCtx = reinterpret_cast<ResolveContext *>(userData);
    rCtx->context         = mainLoop;

    int ret = dnssd_resolve_service(rCtx->service, OnResolve, rCtx);
    VerifyOrReturnError(CheckForSuccess(rCtx, ret, __func__), false);

    rCtx->isResolving = true;

    return true;
}

CHIP_ERROR Resolve(uint32_t interfaceId, const char * type, const char * name, DnssdResolveCallback callback, void * context)
{
    ChipLogProgress(DeviceLayer, "Dnssd %s type: %s, name: %s, interfaceId: %u", __func__, type, name, interfaceId);

    ResolveContext * rCtx = chip::Platform::New<ResolveContext>(type, name, interfaceId, callback, context);

    dnssd_service_h service;
    int ret;

    if (interfaceId == 0)
    {
        ret = dnssd_create_remote_service(type, name, NULL, &service);
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

    DnssdContexts::GetInstance().Add(rCtx, service);
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

DnssdContexts DnssdContexts::sInstance;

void DnssdContexts::Delete(GenericContext * context)
{
    switch (context->contextType)
    {
    case ContextType::Register: {
        RegisterContext * rCtx = reinterpret_cast<RegisterContext *>(context);
        if (rCtx->isRegistered)
        {
            dnssd_deregister_local_service(rCtx->service);
            rCtx->isRegistered = false;
        }
        if (rCtx->service)
        {
            dnssd_destroy_local_service(rCtx->service);
            rCtx->service = 0;
        }
        break;
    }
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

DnssdContexts::~DnssdContexts()
{
    auto iter = mContexts.cbegin();
    while (iter != mContexts.cend())
    {
        Delete(*iter);
        mContexts.erase(iter);
    }
}

CHIP_ERROR DnssdContexts::Add(RegisterContext * context, dnssd_service_h service, const char * type, const char * name,
                              uint16_t port, uint32_t interfaceId)
{
    VerifyOrReturnError(context != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(service != 0, CHIP_ERROR_INVALID_ARGUMENT);

    context->service = service;
    g_strlcpy(context->type, type, sizeof(context->type));
    g_strlcpy(context->name, name, sizeof(context->name));
    context->port        = port;
    context->interfaceId = interfaceId;

    mContexts.push_back(context);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DnssdContexts::Add(BrowseContext * context, dnssd_browser_h browser)
{
    VerifyOrReturnError(context != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(browser != 0, CHIP_ERROR_INVALID_ARGUMENT);
    context->browser = browser;
    mContexts.push_back(context);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnssdContexts::Add(ResolveContext * context, dnssd_service_h service)
{
    VerifyOrReturnError(context != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    context->service = service;
    mContexts.push_back(context);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DnssdContexts::Remove(GenericContext * context)
{
    ChipLogProgress(DeviceLayer, "Dnssd %s", __func__);
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

CHIP_ERROR DnssdContexts::Remove(const char * type, const char * name, uint16_t port, uint32_t interfaceId)
{
    ChipLogProgress(DeviceLayer, "Dnssd %s type: %s, name %s, port: %u, interfaceId: %u", __func__, type, name, port, interfaceId);
    for (auto iter = mContexts.begin(); iter != mContexts.end(); ++iter)
    {
        if ((*iter)->contextType == ContextType::Register)
        {
            RegisterContext * ctx = reinterpret_cast<RegisterContext *>(*iter);
            if (strcmp(ctx->type, type) == 0 && strcmp(ctx->name, name) == 0 && ctx->port == port &&
                ctx->interfaceId == interfaceId)
            {
                Delete(*iter);
                mContexts.erase(iter);
                return CHIP_NO_ERROR;
            }
        }
    }
    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR DnssdContexts::Remove(ContextType type)
{
    ChipLogProgress(DeviceLayer, "Dnssd %s", __func__);
    bool found = false;

    auto iter = mContexts.cbegin();
    while (iter != mContexts.cend())
    {
        if ((*iter)->contextType != type)
        {
            ++iter;
            continue;
        }

        Delete(*iter);
        mContexts.erase(iter);
        found = true;
    }

    return found ? CHIP_NO_ERROR : CHIP_ERROR_KEY_NOT_FOUND;
}

RegisterContext * DnssdContexts::Get(const char * type, const char * name, uint16_t port, uint32_t interfaceId)
{
    for (auto iter = mContexts.begin(); iter != mContexts.end(); ++iter)
    {
        if ((*iter)->contextType == ContextType::Register)
        {
            RegisterContext * ctx = reinterpret_cast<RegisterContext *>(*iter);
            if (strcmp(ctx->type, type) == 0 && strcmp(ctx->name, name) == 0 && ctx->port == port &&
                ctx->interfaceId == interfaceId)
            {
                return ctx;
            }
        }
    }
    return nullptr;
}

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback successCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    VerifyOrReturnError(successCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(errorCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(DeviceLayer, "Dnssd: %s", __func__);
    CHIP_ERROR err = Initialize();
    if (err == CHIP_NO_ERROR)
    {
        successCallback(context, CHIP_NO_ERROR);
    }
    else
    {
        errorCallback(context, CHIP_ERROR_INTERNAL);
    }
    return err;
}

CHIP_ERROR ChipDnssdShutdown()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdSetHostname(const char * hostname)
{
    ChipLogProgress(DeviceLayer, "Dnssd: hostname %s", hostname);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(service->mProtocol), CHIP_ERROR_INVALID_ARGUMENT);

    std::string regtype = GetFullType(service->mType, service->mProtocol);

    return RegisterService(regtype.c_str(), service->mName, service->mPort, service->mInterface.GetPlatformInterface(),
                           service->mTextEntries, service->mTextEntrySize);
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    return UnregisterAllServices();
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interfaceId, DnssdBrowseCallback callback, void * context)
{
    VerifyOrReturnError(type != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(protocol), CHIP_ERROR_INVALID_ARGUMENT);

    std::string regtype = GetFullType(type, protocol);

    return Browse(interfaceId.GetPlatformInterface(), regtype.c_str(), protocol, callback, context);
}

CHIP_ERROR ChipDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interfaceId, DnssdResolveCallback callback,
                            void * context)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(service->mProtocol), CHIP_ERROR_INVALID_ARGUMENT);

    std::string regtype = GetFullType(service->mType, service->mProtocol);

    return Resolve(interfaceId.GetPlatformInterface(), regtype.c_str(), service->mName, callback, context);
}

void GetDnssdTimeout(timeval & timeout)
{
    // Do nothing
    // Tizen DNS-SD API adds I/O events to GMainContext, so MainLoop::AsyncRequest will be used for DNS-SD operations
}

void HandleDnssdTimeout()
{
    // Do nothing
    // Tizen DNS-SD API adds I/O events to GMainContext, so MainLoop::AsyncRequest will be used for DNS-SD operations
}

} // namespace Dnssd
} // namespace chip
