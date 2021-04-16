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
#include "MdnsImpl.h"

#include "MdnsError.h"

#include <cstdio>
#include <sstream>
#include <string.h>

#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/SafeInt.h>
#include <support/logging/CHIPLogging.h>

using namespace chip::Mdns;

namespace {

constexpr const char * kLocalDomain = "local.";
constexpr const char * kProtocolTcp = "._tcp";
constexpr const char * kProtocolUdp = "._udp";
constexpr uint8_t kMdnsKeyMaxSize   = 32;

bool IsSupportedProtocol(MdnsServiceProtocol protocol)
{
    return (protocol == MdnsServiceProtocol::kMdnsProtocolUdp) || (protocol == MdnsServiceProtocol::kMdnsProtocolTcp);
}

uint32_t GetInterfaceId(chip::Inet::InterfaceId interfaceId)
{
    return (interfaceId == INET_NULL_INTERFACEID) ? kDNSServiceInterfaceIndexAny : interfaceId;
}

std::string GetFullType(const char * type, MdnsServiceProtocol protocol)
{
    std::ostringstream typeBuilder;
    typeBuilder << type;
    typeBuilder << (protocol == MdnsServiceProtocol::kMdnsProtocolUdp ? kProtocolUdp : kProtocolTcp);
    return typeBuilder.str();
}

} // namespace

namespace chip {
namespace Mdns {

MdnsContexts MdnsContexts::sInstance;

void MdnsContexts::Delete(GenericContext * context)
{
    if (context->type == ContextType::GetAddrInfo)
    {
        GetAddrInfoContext * addrInfoContext = reinterpret_cast<GetAddrInfoContext *>(context);
        std::vector<TextEntry>::iterator textEntry;
        for (textEntry = addrInfoContext->textEntries.begin(); textEntry != addrInfoContext->textEntries.end(); textEntry++)
        {
            free(const_cast<char *>(textEntry->mKey));
            free(const_cast<uint8_t *>(textEntry->mData));
        }
    }

    DNSServiceRefDeallocate(context->serviceRef);
    chip::Platform::Delete(context);
}

MdnsContexts::~MdnsContexts()
{
    std::vector<GenericContext *>::const_iterator iter = mContexts.cbegin();
    while (iter != mContexts.cend())
    {
        Delete(*iter);
        mContexts.erase(iter);
    }
}

CHIP_ERROR MdnsContexts::Add(GenericContext * context, DNSServiceRef sdRef)
{
    VerifyOrReturnError(context != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(sdRef != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    context->serviceRef = sdRef;
    mContexts.push_back(context);

    return CHIP_NO_ERROR;
}

CHIP_ERROR MdnsContexts::Remove(GenericContext * context)
{
    bool found = false;

    std::vector<GenericContext *>::const_iterator iter = mContexts.cbegin();
    while (iter != mContexts.cend())
    {
        if (*iter != context)
        {
            iter++;
            continue;
        }

        Delete(*iter);
        mContexts.erase(iter);
        found = true;
        break;
    }

    return found ? CHIP_NO_ERROR : CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR MdnsContexts::Removes(ContextType type)
{
    bool found = false;

    std::vector<GenericContext *>::const_iterator iter = mContexts.cbegin();
    while (iter != mContexts.cend())
    {
        if ((*iter)->type != type)
        {
            iter++;
            continue;
        }

        Delete(*iter);
        mContexts.erase(iter);
        found = true;
    }

    return found ? CHIP_NO_ERROR : CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR MdnsContexts::Get(ContextType type, GenericContext * context)
{
    bool found = false;
    std::vector<GenericContext *>::iterator iter;

    for (iter = mContexts.begin(); iter != mContexts.end(); iter++)
    {
        if ((*iter)->type == type)
        {
            context = *iter;
            found   = true;
            break;
        }
    }

    return found ? CHIP_NO_ERROR : CHIP_ERROR_KEY_NOT_FOUND;
}

void MdnsContexts::PrepareSelect(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet, int & maxFd, timeval & timeout)
{
    std::vector<DNSServiceRef> serviceRefs(mContexts.size());
    std::transform(mContexts.begin(), mContexts.end(), serviceRefs.begin(),
                   [](GenericContext * context) { return context->serviceRef; });

    std::vector<DNSServiceRef>::iterator iter;
    for (iter = serviceRefs.begin(); iter != serviceRefs.end(); iter++)
    {
        int fd = DNSServiceRefSockFD(*iter);
        FD_SET(fd, &readFdSet);
        if (maxFd < fd)
        {
            maxFd = fd;
        }
    }
}

void MdnsContexts::HandleSelectResult(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet)
{
    std::vector<DNSServiceRef> serviceRefs(mContexts.size());
    std::transform(mContexts.begin(), mContexts.end(), serviceRefs.begin(),
                   [](GenericContext * context) { return context->serviceRef; });

    std::vector<DNSServiceRef>::iterator iter;
    for (iter = serviceRefs.begin(); iter != serviceRefs.end(); iter++)
    {
        int fd = DNSServiceRefSockFD(*iter);
        if (FD_ISSET(fd, &readFdSet))
        {
            DNSServiceProcessResult(*iter);
        }
    }
}

CHIP_ERROR PopulateTextRecord(TXTRecordRef * record, char * buffer, uint16_t bufferLen, TextEntry * textEntries,
                              size_t textEntrySize)
{
    VerifyOrReturnError(textEntrySize <= kMdnsTextMaxSize, CHIP_ERROR_INVALID_ARGUMENT);

    DNSServiceErrorType err;
    TXTRecordCreate(record, bufferLen, buffer);

    for (size_t i = 0; i < textEntrySize; i++)
    {
        TextEntry entry = textEntries[i];
        VerifyOrReturnError(chip::CanCastTo<uint8_t>(entry.mDataSize), CHIP_ERROR_INVALID_ARGUMENT);

        err = TXTRecordSetValue(record, entry.mKey, static_cast<uint8_t>(entry.mDataSize), entry.mData);
        VerifyOrReturnError(err == kDNSServiceErr_NoError, CHIP_ERROR_INVALID_ARGUMENT);
    }

    return CHIP_NO_ERROR;
}

bool CheckForSuccess(GenericContext * context, const char * name, DNSServiceErrorType err, bool useCallback = false)
{
    if (context == nullptr)
    {
        ChipLogError(DeviceLayer, "%s (%s)", name, "Mdns context is null.");
        return false;
    }

    if (kDNSServiceErr_NoError != err)
    {
        ChipLogError(DeviceLayer, "%s (%s)", name, Error::ToString(err));

        if (useCallback)
        {
            switch (context->type)
            {
            case ContextType::Register:
                // Nothing special to do. Maybe ChipMdnsPublishService should take a callback ?
                break;
            case ContextType::Browse: {
                BrowseContext * browseContext = reinterpret_cast<BrowseContext *>(context);
                browseContext->callback(browseContext->context, nullptr, 0, CHIP_ERROR_INTERNAL);
                break;
            }
            case ContextType::Resolve: {
                ResolveContext * resolveContext = reinterpret_cast<ResolveContext *>(context);
                resolveContext->callback(resolveContext->context, nullptr, CHIP_ERROR_INTERNAL);
                break;
            }
            case ContextType::GetAddrInfo: {
                GetAddrInfoContext * resolveContext = reinterpret_cast<GetAddrInfoContext *>(context);
                resolveContext->callback(resolveContext->context, nullptr, CHIP_ERROR_INTERNAL);
                break;
            }
            }
        }

        if (CHIP_ERROR_KEY_NOT_FOUND == MdnsContexts::GetInstance().Remove(context))
        {
            chip::Platform::Delete(context);
        }

        return false;
    }

    return true;
}

static void OnRegister(DNSServiceRef sdRef, DNSServiceFlags flags, DNSServiceErrorType err, const char * name, const char * type,
                       const char * domain, void * context)
{
    RegisterContext * sdCtx = reinterpret_cast<RegisterContext *>(context);
    VerifyOrReturn(CheckForSuccess(sdCtx, __func__, err));

    ChipLogDetail(DeviceLayer, "Mdns: %s name: %s, type: %s, domain: %s, flags: %d", __func__, name, type, domain, flags);
};

CHIP_ERROR Register(uint32_t interfaceId, const char * type, const char * name, uint16_t port, TXTRecordRef * recordRef)
{
    DNSServiceErrorType err;
    DNSServiceRef sdRef;
    RegisterContext * sdCtx = nullptr;

    uint16_t recordLen          = TXTRecordGetLength(recordRef);
    const void * recordBytesPtr = TXTRecordGetBytesPtr(recordRef);

    if (CHIP_NO_ERROR == MdnsContexts::GetInstance().Get(ContextType::Register, sdCtx))
    {
        err = DNSServiceUpdateRecord(sdCtx->serviceRef, NULL, 0 /* flags */, recordLen, recordBytesPtr, 0 /* ttl */);
        TXTRecordDeallocate(recordRef);
        VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err), CHIP_ERROR_INTERNAL);
        return CHIP_NO_ERROR;
    }

    sdCtx = chip::Platform::New<RegisterContext>(nullptr);
    err   = DNSServiceRegister(&sdRef, 0 /* flags */, interfaceId, name, type, kLocalDomain, NULL, port, recordLen, recordBytesPtr,
                             OnRegister, sdCtx);
    TXTRecordDeallocate(recordRef);

    VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err), CHIP_ERROR_INTERNAL);

    return MdnsContexts::GetInstance().Add(sdCtx, sdRef);
}

void OnBrowseAdd(BrowseContext * context, const char * name, const char * type, const char * domain, uint32_t interfaceId)
{
    ChipLogDetail(DeviceLayer, "Mdns: %s  name: %s, type: %s, domain: %s, interface: %d", __func__, name, type, domain,
                  interfaceId);

    VerifyOrReturn(strcmp(kLocalDomain, domain) == 0);

    char * tokens  = strdup(type);
    char * regtype = strtok(tokens, ".");
    free(tokens);

    MdnsService service = {};
    service.mInterface  = interfaceId;
    service.mProtocol   = context->protocol;

    strncpy(service.mName, name, sizeof(service.mName));
    service.mName[kMdnsNameMaxSize] = 0;

    strncpy(service.mType, regtype, sizeof(service.mType));
    service.mType[kMdnsTypeMaxSize] = 0;

    context->services.push_back(service);
}

void OnBrowseRemove(BrowseContext * context, const char * name, const char * type, const char * domain, uint32_t interfaceId)
{
    ChipLogDetail(DeviceLayer, "Mdns: %s  name: %s, type: %s, domain: %s, interface: %d", __func__, name, type, domain,
                  interfaceId);

    VerifyOrReturn(strcmp(kLocalDomain, domain) == 0);

    std::remove_if(context->services.begin(), context->services.end(), [name, type, interfaceId](const MdnsService & service) {
        return strcmp(name, service.mName) == 0 && type == GetFullType(service.mType, service.mProtocol) &&
            service.mInterface == interfaceId;
    });
}

static void OnBrowse(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err, const char * name,
                     const char * type, const char * domain, void * context)
{
    BrowseContext * sdCtx = reinterpret_cast<BrowseContext *>(context);
    VerifyOrReturn(CheckForSuccess(sdCtx, __func__, err, true));

    (flags & kDNSServiceFlagsAdd) ? OnBrowseAdd(sdCtx, name, type, domain, interfaceId)
                                  : OnBrowseRemove(sdCtx, name, type, domain, interfaceId);

    if (!(flags & kDNSServiceFlagsMoreComing))
    {
        sdCtx->callback(sdCtx->context, sdCtx->services.data(), sdCtx->services.size(), CHIP_NO_ERROR);
        MdnsContexts::GetInstance().Remove(sdCtx);
    }
}

CHIP_ERROR Browse(void * context, MdnsBrowseCallback callback, uint32_t interfaceId, const char * type,
                  MdnsServiceProtocol protocol)
{
    DNSServiceErrorType err;
    DNSServiceRef sdRef;
    BrowseContext * sdCtx;

    sdCtx = chip::Platform::New<BrowseContext>(context, callback, protocol);
    err   = DNSServiceBrowse(&sdRef, 0 /* flags */, interfaceId, type, kLocalDomain, OnBrowse, sdCtx);
    VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err), CHIP_ERROR_INTERNAL);

    return MdnsContexts::GetInstance().Add(sdCtx, sdRef);
}

static void OnGetAddrInfo(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err,
                          const char * hostname, const struct sockaddr * address, uint32_t ttl, void * context)
{
    GetAddrInfoContext * sdCtx = reinterpret_cast<GetAddrInfoContext *>(context);
    VerifyOrReturn(CheckForSuccess(sdCtx, __func__, err, true));

    ChipLogDetail(DeviceLayer, "Mdns: %s hostname:%s", __func__, hostname);

    MdnsService service    = {};
    service.mPort          = sdCtx->port;
    service.mTextEntries   = sdCtx->textEntries.empty() ? nullptr : sdCtx->textEntries.data();
    service.mTextEntrySize = sdCtx->textEntries.empty() ? 0 : sdCtx->textEntries.size();
    service.mAddress.SetValue(chip::Inet::IPAddress::FromSockAddr(*address));
    strncpy(service.mName, sdCtx->name, sizeof(service.mName));

    sdCtx->callback(sdCtx->context, &service, CHIP_NO_ERROR);
    MdnsContexts::GetInstance().Remove(sdCtx);
}

CHIP_ERROR GetAddrInfo(void * context, MdnsResolveCallback callback, uint32_t interfaceId, const char * name, const char * hostname,
                       uint16_t port, uint16_t txtLen, const unsigned char * txtRecord)
{
    DNSServiceErrorType err;
    DNSServiceRef sdRef;
    GetAddrInfoContext * sdCtx;

    sdCtx = chip::Platform::New<GetAddrInfoContext>(context, callback, name, port);

    char key[kMdnsKeyMaxSize];
    char value[kMdnsTextMaxSize];
    uint8_t valueLen;
    const void * valuePtr;

    uint16_t recordCount = TXTRecordGetCount(txtLen, txtRecord);
    for (uint16_t i = 0; i < recordCount; i++)
    {
        err = TXTRecordGetItemAtIndex(txtLen, txtRecord, i, kMdnsKeyMaxSize, key, &valueLen, &valuePtr);
        VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err, true), CHIP_ERROR_INTERNAL);

        strncpy(value, reinterpret_cast<const char *>(valuePtr), valueLen);
        value[valueLen] = 0;

        sdCtx->textEntries.push_back(TextEntry{ strdup(key), reinterpret_cast<const uint8_t *>(strdup(value)), valueLen });
    }

    err = DNSServiceGetAddrInfo(&sdRef, 0 /* flags */, interfaceId, kDNSServiceProtocol_IPv4, hostname, OnGetAddrInfo, sdCtx);
    VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err, true), CHIP_ERROR_INTERNAL);

    return MdnsContexts::GetInstance().Add(sdCtx, sdRef);
}

static void OnResolve(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceId, DNSServiceErrorType err,
                      const char * fullname, const char * hostname, uint16_t port, uint16_t txtLen, const unsigned char * txtRecord,
                      void * context)
{
    ResolveContext * sdCtx = reinterpret_cast<ResolveContext *>(context);
    VerifyOrReturn(CheckForSuccess(sdCtx, __func__, err, true));

    GetAddrInfo(sdCtx->context, sdCtx->callback, interfaceId, sdCtx->name, hostname, ntohs(port), txtLen, txtRecord);
    MdnsContexts::GetInstance().Remove(sdCtx);
}

CHIP_ERROR Resolve(void * context, MdnsResolveCallback callback, uint32_t interfaceId, const char * type, const char * name)
{
    DNSServiceErrorType err;
    DNSServiceRef sdRef;
    ResolveContext * sdCtx;

    sdCtx = chip::Platform::New<ResolveContext>(context, callback, name);
    err   = DNSServiceResolve(&sdRef, 0 /* flags */, interfaceId, name, type, kLocalDomain, OnResolve, sdCtx);
    VerifyOrReturnError(CheckForSuccess(sdCtx, __func__, err), CHIP_ERROR_INTERNAL);

    return MdnsContexts::GetInstance().Add(sdCtx, sdRef);
}

CHIP_ERROR ChipMdnsInit(MdnsAsyncReturnCallback successCallback, MdnsAsyncReturnCallback errorCallback, void * context)
{
    VerifyOrReturnError(successCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(errorCallback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    successCallback(context, CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipMdnsSetHostname(const char * hostname)
{
    MdnsContexts::GetInstance().SetHostname(hostname);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipMdnsPublishService(const MdnsService * service)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(service->mProtocol), CHIP_ERROR_INVALID_ARGUMENT);

    std::string regtype  = GetFullType(service->mType, service->mProtocol);
    uint32_t interfaceId = GetInterfaceId(service->mInterface);

    TXTRecordRef record;
    char buffer[kMdnsTextMaxSize];
    ReturnErrorOnFailure(PopulateTextRecord(&record, buffer, sizeof(buffer), service->mTextEntries, service->mTextEntrySize));

    return Register(interfaceId, regtype.c_str(), service->mName, service->mPort, &record);
}

CHIP_ERROR ChipMdnsStopPublish()
{
    RegisterContext * sdCtx = nullptr;
    if (CHIP_ERROR_KEY_NOT_FOUND == MdnsContexts::GetInstance().Get(ContextType::Register, sdCtx))
    {
        return CHIP_NO_ERROR;
    }

    return MdnsContexts::GetInstance().Removes(ContextType::Register);
}

CHIP_ERROR ChipMdnsStopPublishService(const MdnsService * service)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipMdnsBrowse(const char * type, MdnsServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                          chip::Inet::InterfaceId interface, MdnsBrowseCallback callback, void * context)
{
    VerifyOrReturnError(type != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(protocol), CHIP_ERROR_INVALID_ARGUMENT);

    std::string regtype  = GetFullType(type, protocol);
    uint32_t interfaceId = GetInterfaceId(interface);

    return Browse(context, callback, interfaceId, regtype.c_str(), protocol);
}

CHIP_ERROR ChipMdnsResolve(MdnsService * service, chip::Inet::InterfaceId interface, MdnsResolveCallback callback, void * context)
{
    VerifyOrReturnError(service != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSupportedProtocol(service->mProtocol), CHIP_ERROR_INVALID_ARGUMENT);

    std::string regtype  = GetFullType(service->mType, service->mProtocol);
    uint32_t interfaceId = GetInterfaceId(interface);

    return Resolve(context, callback, interfaceId, regtype.c_str(), service->mName);
}

void UpdateMdnsDataset(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet, int & maxFd, timeval & timeout)
{
    MdnsContexts::GetInstance().PrepareSelect(readFdSet, writeFdSet, errorFdSet, maxFd, timeout);
}

void ProcessMdns(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet)
{
    MdnsContexts::GetInstance().HandleSelectResult(readFdSet, writeFdSet, errorFdSet);
}

} // namespace Mdns
} // namespace chip
