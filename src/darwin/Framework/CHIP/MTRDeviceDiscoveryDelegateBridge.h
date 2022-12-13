/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#import "MTRDeviceDiscoveryDelegate.h"
#include <controller/CHIPDeviceController.h>
#include <platform/CHIPDeviceBuildConfig.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRDeviceController;

class MTRDeviceDiscoveryDelegateBridge : public chip::Controller::AbstractDnssdDiscoveryController {
public:
    MTRDeviceDiscoveryDelegateBridge();
    ~MTRDeviceDiscoveryDelegateBridge();
    void setDelegate(MTRDeviceController * controller, id<MTRDeviceDiscoveryDelegate> delegate, dispatch_queue_t queue);
    void OnNodeDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData) override;

protected:
    DiscoveredNodeList GetDiscoveredNodes() override { return DiscoveredNodeList(mDiscoveredCommissioners); }

private:
    MTRDeviceController * __weak mController;
    _Nullable id<MTRDeviceDiscoveryDelegate> mDelegate;
    _Nullable dispatch_queue_t mQueue;
    chip::Dnssd::DiscoveredNodeData mDiscoveredCommissioners[CHIP_DEVICE_CONFIG_MAX_DISCOVERED_NODES];
};

NS_ASSUME_NONNULL_END
