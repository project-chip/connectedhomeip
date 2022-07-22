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

#include "AddressResolve.h"

#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include <condition_variable>
#include <mutex>
#include <stdlib.h>
#include <strings.h>

namespace {

using namespace chip;
using chip::Inet::InterfaceId;
using chip::Transport::PeerAddress;

// clang-format off
const char * const sHelp =
    "Usage: address-resolve-tool <command> [ <args...> ]\n"
    "\n"
    "Commands:\n"
    "\n"
    "    node <nodeid> <compressed_fabricid> -- Find the node for the given node/fabric.\n"
    "\n";
// clang-format on

class PrintOutNodeListener : public chip::AddressResolve::NodeListener
{
public:
    PrintOutNodeListener() { mSelfHandle.SetListener(this); }

    void OnNodeAddressResolved(const PeerId & peerId, const AddressResolve::ResolveResult & result) override
    {
        char addr_string[PeerAddress::kMaxToStringSize];
        result.address.ToString(addr_string);

        ChipLogProgress(Discovery, "Resolve completed: %s", addr_string);
        ChipLogProgress(Discovery, "   Supports TCP:                  %s", result.supportsTcp ? "YES" : "NO");
        ChipLogProgress(Discovery, "   MRP IDLE retransmit timeout:   %u ms", result.mrpRemoteConfig.mIdleRetransTimeout.count());
        ChipLogProgress(Discovery, "   MRP ACTIVE retransmit timeout: %u ms", result.mrpRemoteConfig.mActiveRetransTimeout.count());
        NotifyDone();
    }

    void OnNodeAddressResolutionFailed(const PeerId & peerId, CHIP_ERROR reason) override
    {
        ChipLogError(Discovery, "Failed to resolve: %s", reason.AsString());
        NotifyDone();
    }

    AddressResolve::NodeLookupHandle & Handle() { return mSelfHandle; }

    void NotifyDone() { DeviceLayer::PlatformMgr().StopEventLoopTask(); }

private:
    AddressResolve::NodeLookupHandle mSelfHandle;
};

PrintOutNodeListener gListener;

CHIP_ERROR Initialize()
{
    ReturnErrorOnFailure(DeviceLayer::PlatformMgr().InitChipStack());
    ReturnErrorOnFailure(Dnssd::Resolver::Instance().Init(DeviceLayer::UDPEndPointManager()));
    ReturnErrorOnFailure(AddressResolve::Resolver::Instance().Init(&DeviceLayer::SystemLayer()));
    return CHIP_NO_ERROR;
}

void Shutdown()
{
    Dnssd::Resolver::Instance().Shutdown();
    DeviceLayer::PlatformMgr().Shutdown();
}

bool Cmd_Node(int argc, const char ** argv)
{
    if (argc != 2)
    {
        ChipLogError(NotSpecified, "Please provide nodeid and fabricid as arguments");
        return false;
    }

    uint64_t node = strtoull(argv[0], nullptr, 0 /* auto-detect base */);

    if (node == 0)
    {
        ChipLogError(NotSpecified, "Node ID %s not supported", argv[0]);
        return false;
    }

    uint64_t fabric = strtoull(argv[1], nullptr, 0 /* auto-detect base */);
    if (node == 0)
    {
        ChipLogError(NotSpecified, "Compressed fabric ID %s not supported", argv[1]);
        return false;
    }

    ChipLogProgress(NotSpecified, "Will search for node %" PRIx64 " on fabric %" PRIx64, node, fabric);

    AddressResolve::NodeLookupRequest request(PeerId().SetNodeId(node).SetCompressedFabricId(fabric));

    CHIP_ERROR err = AddressResolve::Resolver::Instance().LookupNode(request, gListener.Handle());

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Discovery, "Lookup request failed: %s", err.AsString());
        return false;
    }
    ChipLogProgress(NotSpecified, "Running chip event loop and waiting for finish...");

    DeviceLayer::PlatformMgr().RunEventLoop();
    Shutdown();
    return true;
}

} // namespace

extern "C" int main(int argc, const char ** argv)
{
    Platform::MemoryInit();

    if (argc < 2)
    {
        ChipLogError(NotSpecified, "Please specify a command, or 'help' for help.");
        return -1;
    }
    if (strcasecmp(argv[1], "help") == 0 || strcasecmp(argv[1], "--help") == 0 || strcasecmp(argv[1], "-h") == 0)
    {
        fputs(sHelp, stdout);
        return 0;
    }

    CHIP_ERROR err = Initialize();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Initialization failed: %s", err.AsString());
        return 1;
    }

    if (strcasecmp(argv[1], "node") == 0)
    {
        return Cmd_Node(argc - 2, argv + 2) ? 0 : 1;
    }

    ChipLogError(NotSpecified, "Unrecognized command: %s", argv[1]);
    return 1;
}
