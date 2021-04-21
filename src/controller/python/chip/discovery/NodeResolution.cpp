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
#include <mdns/Resolver.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>

using namespace chip;
using namespace chip::Mdns;

namespace {

// callback types shared with python code (see ptyhon code in chip.discovery.types)
using DiscoverSuccessCallback = void (*)(uint64_t fabricId, uint64_t nodeId, uint32_t interfaceId, const char * ip, uint16_t port);
using DiscoverFailureCallback = void (*)(uint64_t fabricId, uint64_t nodeId, uint32_t error_code);

constexpr uint16_t kMdnsPort = 5353;

class PythonResolverDelegate : public ResolverDelegate
{
public:
    void OnNodeIdResolved(const ResolvedNodeData & nodeData) override
    {
        if (mSuccessCallback != nullptr)
        {
            char ipAddressBuffer[128];

            mSuccessCallback(                                                         //
                nodeData.mPeerId.GetFabricId(),                                       //
                nodeData.mPeerId.GetNodeId(),                                         //
                nodeData.mInterfaceId,                                                //
                nodeData.mAddress.ToString(ipAddressBuffer, sizeof(ipAddressBuffer)), //
                nodeData.mPort                                                        //
            );
        }
        else
        {
            ChipLogError(Controller, "Discovery success without any python callback set.");
        }
    }

    void OnNodeIdResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override
    {
        if (mFailureCallback != nullptr)
        {
            mFailureCallback(peerId.GetFabricId(), peerId.GetNodeId(), error);
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

extern "C" int32_t pychip_discovery_resolve(uint64_t fabricId, uint64_t nodeId)
{
    int32_t result = CHIP_NO_ERROR;

    chip::python::ChipMainThreadScheduleAndWait([&] {
        result = Resolver::Instance().StartResolver(&chip::DeviceLayer::InetLayer, kMdnsPort);
        ReturnOnFailure(result);

        result = Resolver::Instance().SetResolverDelegate(&gPythonResolverDelegate);
        ReturnOnFailure(result);

        result = Resolver::Instance().ResolveNodeId(chip::PeerId().SetFabricId(fabricId).SetNodeId(nodeId),
                                                    chip::Inet::IPAddressType::kIPAddressType_Any);
    });

    return result;
}
