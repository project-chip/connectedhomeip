#include <assert.h>
#include <avahi-common/simple-watch.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/simple-watch.h>

#include "platform/CHIPDeviceLayer.h"
#include "platform/Mdns.h"

using chip::DeviceLayer::MdnsResolveResult;
using chip::DeviceLayer::MdnsService;
using chip::DeviceLayer::MdnsServiceProtocol;
using chip::DeviceLayer::TextEntry;

static void HandleResolve(void * context, MdnsResolveResult * result, CHIP_ERROR error)
{
    char addrBuf[100];

    assert(result != nullptr);
    assert(error == CHIP_NO_ERROR);
    result->mAddress.ToString(addrBuf, sizeof(addrBuf));
    printf("Service at [%s]:%u\n", addrBuf, result->mService.mPort);
    exit(0);
}

static void HandleBrowse(void * context, MdnsService * services, size_t servicesSize, CHIP_ERROR error)
{
    assert(error == CHIP_NO_ERROR);
    if (services)
    {
        printf("Mdns service size %zu\n", servicesSize);
        printf("Service name %s\n", services->mName);
        printf("Service type %s\n", services->mType);
        ChipMdnsResolve(services->mName, services->mType, services->mProtocol, INET_NULL_INTERFACEID, HandleResolve, nullptr);
    }
}

static void InitCallback(CHIP_ERROR error)
{
    MdnsService service;
    TextEntry entry;
    char buf[] = "key=val";

    assert(error == CHIP_NO_ERROR);

    service.interface = INET_NULL_INTERFACEID;
    service.mPort     = 80;
    strcpy(service.mName, "test");
    strcpy(service.mType, "_mock");
    service.mProtocol      = MdnsServiceProtocol::kMdnsProtocolTcp;
    entry.mData            = reinterpret_cast<const uint8_t *>(buf);
    entry.mSize            = strlen(reinterpret_cast<const char *>(entry.mData));
    service.mTextEntryies  = &entry;
    service.mTextEntrySize = 1;

    assert(ChipMdnsPublishService(&service) == CHIP_NO_ERROR);
    ChipMdnsBrowse("_mock", MdnsServiceProtocol::kMdnsProtocolTcp, INET_NULL_INTERFACEID, HandleBrowse, nullptr);
}

static void ErrorCallback(CHIP_ERROR error)
{
    if (error != CHIP_NO_ERROR)
    {
        fprintf(stderr, "Mdns error: %d\n", static_cast<int>(error));
        abort();
    }
}

int TestMdns()
{
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    chip::DeviceLayer::ChipMdnsInit(InitCallback, ErrorCallback);

    ChipLogProgress(DeviceLayer, "Start EventLoop");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}
