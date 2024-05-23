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

#include <chip/internal/ChipThreadWork.h>
#include <controller/python/chip/native/PyChipError.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <type_traits>

using namespace chip;
using namespace chip::Dnssd;

namespace {

// callback types shared with python code (see ptyhon code in chip.discovery.types)
using DiscoverSuccessCallback = void (*)(uint64_t fabricId, uint64_t nodeId, uint32_t interfaceId, const char * ip, uint16_t port);
using DiscoverFailureCallback = void (*)(uint64_t fabricId, uint64_t nodeId, PyChipError error_code);

class PythonResolverDelegate : public OperationalResolveDelegate
{
public:
    void OnOperationalNodeResolved(const ResolvedNodeData & nodeData) override
    {
        Resolver::Instance().NodeIdResolutionNoLongerNeeded(nodeData.operationalData.peerId);
        if (mSuccessCallback != nullptr)
        {
            char ipAddressBuffer[128];

            // TODO: For now, just provide addr 0, but this should really provide all and
            // allow the caller to choose.
            mSuccessCallback(                                                                            //
                nodeData.operationalData.peerId.GetCompressedFabricId(),                                 //
                nodeData.operationalData.peerId.GetNodeId(),                                             //
                nodeData.resolutionData.interfaceId.GetPlatformInterface(),                              //
                nodeData.resolutionData.ipAddress[0].ToString(ipAddressBuffer, sizeof(ipAddressBuffer)), //
                nodeData.resolutionData.port                                                             //
            );
        }
        else
        {
            ChipLogError(Controller, "Discovery success without any python callback set.");
        }
    }

    void OnOperationalNodeResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override
    {
        Resolver::Instance().NodeIdResolutionNoLongerNeeded(peerId);
        if (mFailureCallback != nullptr)
        {
            mFailureCallback(peerId.GetCompressedFabricId(), peerId.GetNodeId(), ToPyChipError(error));
        }
        else
        {
            ChipLogError(Controller, "Discovery failure without any python callback set.");
        }
    }

    void SetSuccessCallback(DiscoverSuccessCallback cb) { mSuccessCallback = cb; }
    void SetFailureCallback(DiscoverFailureCallback cb) { mFailureCallback = cb; }

private:
    DiscoverSuccessCallback mSuccessCallback = nullptr;
    DiscoverFailureCallback mFailureCallback = nullptr;
}; // namespace

PythonResolverDelegate gPythonResolverDelegate;

} // namespace

extern "C" void pychip_discovery_set_callbacks(DiscoverSuccessCallback success, DiscoverFailureCallback failure)
{
    gPythonResolverDelegate.SetSuccessCallback(success);
    gPythonResolverDelegate.SetFailureCallback(failure);
}

extern "C" PyChipError pychip_discovery_resolve(uint64_t fabricId, uint64_t nodeId)
{
    CHIP_ERROR result = CHIP_NO_ERROR;

    chip::python::ChipMainThreadScheduleAndWait([&] {
        result = Resolver::Instance().Init(chip::DeviceLayer::UDPEndPointManager());
        ReturnOnFailure(result);
        Resolver::Instance().SetOperationalDelegate(&gPythonResolverDelegate);

        result = Resolver::Instance().ResolveNodeId(chip::PeerId().SetCompressedFabricId(fabricId).SetNodeId(nodeId));
    });

    return ToPyChipError(result);
}
