#include "RendezvousServer.h"
#if CHIP_ENABLE_OPENTHREAD
#include <platform/ThreadStackManager.h>
#include <platform/internal/DeviceNetworkInfo.h>
#endif

using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::DeviceLayer;

namespace chip {

RendezvousServer::RendezvousServer() : mRendezvousSession(this) {}

CHIP_ERROR RendezvousServer::Init(const RendezvousParameters & params)
{
    return mRendezvousSession.Init(params);
}

void RendezvousServer::OnRendezvousError(CHIP_ERROR err)
{
    ChipLogProgress(AppServer, "OnRendezvousError: %s", ErrorStr(err));
}

void RendezvousServer::OnRendezvousConnectionOpened()
{
    ChipLogProgress(AppServer, "OnRendezvousConnectionOpened");
}

void RendezvousServer::OnRendezvousConnectionClosed()
{
    ChipLogProgress(AppServer, "OnRendezvousConnectionClosed");
}

void RendezvousServer::OnRendezvousMessageReceived(PacketBuffer * buffer)
{
#if CHIP_ENABLE_OPENTHREAD
    uint16_t bufferLen = buffer->DataLength();
    uint8_t * data     = buffer->Start();
    chip::DeviceLayer::Internal::DeviceNetworkInfo networkInfo;
    ChipLogProgress(AppServer, "Receive BLE message size=%u", bufferLen);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadNetworkName),
                 ChipLogProgress(AppServer, "Invalid network provision message"));
    memcpy(networkInfo.ThreadNetworkName, data, sizeof(networkInfo.ThreadNetworkName));
    data += sizeof(networkInfo.ThreadNetworkName);
    bufferLen -= sizeof(networkInfo.ThreadNetworkName);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadExtendedPANId),
                 ChipLogProgress(AppServer, "Invalid network provision message"));
    memcpy(networkInfo.ThreadExtendedPANId, data, sizeof(networkInfo.ThreadExtendedPANId));
    data += sizeof(networkInfo.ThreadExtendedPANId);
    bufferLen -= sizeof(networkInfo.ThreadExtendedPANId);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadMeshPrefix),
                 ChipLogProgress(AppServer, "Invalid network provision message"));
    memcpy(networkInfo.ThreadMeshPrefix, data, sizeof(networkInfo.ThreadMeshPrefix));
    data += sizeof(networkInfo.ThreadMeshPrefix);
    bufferLen -= sizeof(networkInfo.ThreadMeshPrefix);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadNetworkKey),
                 ChipLogProgress(AppServer, "Invalid network provision message"));
    memcpy(networkInfo.ThreadNetworkKey, data, sizeof(networkInfo.ThreadNetworkKey));
    data += sizeof(networkInfo.ThreadNetworkKey);
    bufferLen -= sizeof(networkInfo.ThreadNetworkKey);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadPSKc), ChipLogProgress(AppServer, "Invalid network provision message"));
    memcpy(networkInfo.ThreadPSKc, data, sizeof(networkInfo.ThreadPSKc));
    data += sizeof(networkInfo.ThreadPSKc);
    bufferLen -= sizeof(networkInfo.ThreadPSKc);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadPANId), ChipLogProgress(AppServer, "Invalid network provision message"));
    networkInfo.ThreadPANId = data[0] | (data[1] << 8);
    data += sizeof(networkInfo.ThreadPANId);
    bufferLen -= sizeof(networkInfo.ThreadPANId);

    VerifyOrExit(bufferLen >= sizeof(networkInfo.ThreadChannel), ChipLogProgress(AppServer, "Invalid network provision message"));
    networkInfo.ThreadChannel = data[0];
    data += sizeof(networkInfo.ThreadChannel);
    bufferLen -= sizeof(networkInfo.ThreadChannel);

    VerifyOrExit(bufferLen >= 3, ChipLogProgress(AppServer, "Invalid network provision message"));
    networkInfo.FieldPresent.ThreadExtendedPANId = *data;
    data++;
    networkInfo.FieldPresent.ThreadMeshPrefix = *data;
    data++;
    networkInfo.FieldPresent.ThreadPSKc = *data;
    data++;
    networkInfo.NetworkId              = 0;
    networkInfo.FieldPresent.NetworkId = true;

    ThreadStackMgr().SetThreadEnabled(false);
    ThreadStackMgr().SetThreadProvision(networkInfo);
    ThreadStackMgr().SetThreadEnabled(true);

#endif
exit:
    chip::System::PacketBuffer::Free(buffer);
}

} // namespace chip
