#include <cstring>

#include <inet/InetInterface.h>
#include <lib/mdns/platform/Mdns.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ReturnMacros.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::Mdns;
using namespace chip::Inet;

using PyObjectPtr                               = void *;
using PythonChipMdnsBrowseFinishedCallback      = void (*)(PyObjectPtr context, CHIP_ERROR error);
using PythonChipMdnsBrowseServiceFoundCallback  = void (*)(PyObjectPtr context, const char * name, const char * type, int protocol,
                                                          int addressType);
using PythonChipMdnsResolveServiceFoundCallback = void (*)(PyObjectPtr context, const char * name, const char * type, int protocol,
                                                           int addressType);

using PythonChipMdnsResolveFinishCallback    = void (*)(PyObjectPtr context, CHIP_ERROR err);
using PythonChipMdnsResolveTextEntryCallback = void (*)(PyObjectPtr context, const char * key, const uint8_t * data,
                                                        size_t dataSize);
using PythonChipMdnsResolveAddressCallback   = void (*)(PyObjectPtr context, const char * address, uint16_t port);

namespace {
PythonChipMdnsBrowseFinishedCallback sOnBrowseFinishedCallback             = nullptr;
PythonChipMdnsBrowseServiceFoundCallback sOnMdnsBrowseServiceFoundCallback = nullptr;

PythonChipMdnsResolveFinishCallback sOnResolveFinishedCallback     = nullptr;
PythonChipMdnsResolveTextEntryCallback sOnResolveTextEntryCallback = nullptr;
PythonChipMdnsResolveAddressCallback sOnResolveAddressCallback     = nullptr;
} // namespace

void PythonChipMdnsBrowseCallbackWrapper(void * context, MdnsService * services, size_t servicesSize, CHIP_ERROR error)
{
    ChipLogDetail(Controller, "PythonChipMdnsBrowseCallbackWarpper: services: %zu services", servicesSize);
    for (size_t i = 0; i < servicesSize; i++)
    {
        if (sOnMdnsBrowseServiceFoundCallback != nullptr)
        {
            sOnMdnsBrowseServiceFoundCallback(context, services[i].mName, services[i].mType,
                                              static_cast<int>(services[i].mProtocol), static_cast<int>(services[i].mAddressType));
        }
    }
    if (sOnBrowseFinishedCallback != nullptr)
    {
        sOnBrowseFinishedCallback(context, error);
    }
}

void PythonChipMdnsResolveCallbackWrapper(void * context, MdnsService * result, CHIP_ERROR error)
{
    char address[kMaxIPAddressStringLength];
    VerifyOrExit(result != nullptr, /* no action */);
    if (result->mAddress.HasValue())
    {
        result->mAddress.Value().ToString(address, sizeof(address));
        if (sOnResolveAddressCallback != nullptr)
        {
            sOnResolveAddressCallback(context, address, result->mPort);
        }
    }
    for (size_t i = 0; i < result->mTextEntrySize; i++)
    {
        if (sOnResolveTextEntryCallback != nullptr)
        {
            sOnResolveTextEntryCallback(context, result->mTextEntries[i].mKey, result->mTextEntries[i].mData,
                                        result->mTextEntries[i].mDataSize);
        }
    }
exit:
    if (sOnResolveFinishedCallback != nullptr)
    {
        sOnResolveFinishedCallback(context, error);
    }
}

extern "C" {
CHIP_ERROR pychip_ChipMdnsBrowse(const char * type, uint8_t protocol, int addressType, PyObjectPtr context);
CHIP_ERROR pychip_ChipMdnsResolve(const char * name, const char * type, uint8_t protocol, int addressType, PyObjectPtr context);

void pychip_SetMdnsBrowseCallbacks(PythonChipMdnsBrowseFinishedCallback onBrowseFinishedCallback,
                                   PythonChipMdnsBrowseServiceFoundCallback onMdnsBrowseServiceFoundCallback)
{
    sOnBrowseFinishedCallback         = onBrowseFinishedCallback;
    sOnMdnsBrowseServiceFoundCallback = onMdnsBrowseServiceFoundCallback;
}

void pychip_SetMdnsResolveCallbacks(PythonChipMdnsResolveFinishCallback onResolveFinishedCallback,
                                    PythonChipMdnsResolveTextEntryCallback onResolveTextEntryCallback,
                                    PythonChipMdnsResolveAddressCallback onResolveAddressCallback)
{
    sOnResolveFinishedCallback  = onResolveFinishedCallback;
    sOnResolveTextEntryCallback = onResolveTextEntryCallback;
    sOnResolveAddressCallback   = onResolveAddressCallback;
}
}

CHIP_ERROR Uint8ToMdnsServiceProtocol(uint8_t protocol, MdnsServiceProtocol * outMdnsProtocol)
{
    VerifyOrReturnError(outMdnsProtocol != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    switch (protocol)
    {
    case 0:
        *outMdnsProtocol = MdnsServiceProtocol::kMdnsProtocolUdp;
        break;
    case 1:
        *outMdnsProtocol = MdnsServiceProtocol::kMdnsProtocolTcp;
        break;
    case 255:
        *outMdnsProtocol = MdnsServiceProtocol::kMdnsProtocolUnknown;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR IntToIPAddressTypeProtocol(int addressType, IPAddressType * outIPAddressType)
{
    VerifyOrReturnError(outIPAddressType != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    switch (addressType)
    {
    case 0:
        *outIPAddressType = kIPAddressType_Unknown;
        break;
#if INET_CONFIG_ENABLE_IPV4
    case 1:
        *outIPAddressType = kIPAddressType_IPv4;
        break;
#endif
    case 2:
        *outIPAddressType = kIPAddressType_IPv6;
        break;
    case 3:
        (*outIPAddressType) = kIPAddressType_Any;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR pychip_ChipMdnsBrowse(const char * type, uint8_t protocol, int addressType, PyObjectPtr context)
{
    MdnsServiceProtocol mdnsProtocol;
    IPAddressType mdnsAddressType;

    ReturnErrorOnFailure(Uint8ToMdnsServiceProtocol(protocol, &mdnsProtocol));
    ReturnErrorOnFailure(IntToIPAddressTypeProtocol(addressType, &mdnsAddressType));

    return ChipMdnsBrowse(type, mdnsProtocol, mdnsAddressType, INET_NULL_INTERFACEID, PythonChipMdnsBrowseCallbackWrapper, context);
}

CHIP_ERROR pychip_ChipMdnsResolve(const char * name, const char * type, uint8_t protocol, int addressType, PyObjectPtr context)
{
    MdnsService serviceName;

    strncpy(serviceName.mName, name, sizeof(serviceName.mName));
    strncpy(serviceName.mType, type, sizeof(serviceName.mType));
    ReturnErrorOnFailure(Uint8ToMdnsServiceProtocol(protocol, &serviceName.mProtocol));
    ReturnErrorOnFailure(IntToIPAddressTypeProtocol(addressType, &serviceName.mAddressType));

    return ChipMdnsResolve(&serviceName, INET_NULL_INTERFACEID, PythonChipMdnsResolveCallbackWrapper, context);
}
