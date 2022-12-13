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

#import "MTRDeviceDiscoveryDelegateBridge.h"
#import "MTRDeviceController.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"

MTRDeviceDiscoveryDelegateBridge::MTRDeviceDiscoveryDelegateBridge(void)
    : mDelegate(nil)
{
}

MTRDeviceDiscoveryDelegateBridge::~MTRDeviceDiscoveryDelegateBridge(void) {}

void MTRDeviceDiscoveryDelegateBridge::setDelegate(
    MTRDeviceController * controller, id<MTRDeviceDiscoveryDelegate> delegate, dispatch_queue_t queue)
{
    if (delegate && queue) {
        mController = controller;
        mDelegate = delegate;
        mQueue = queue;
    } else {
        mController = nil;
        mDelegate = nil;
        mQueue = nil;
    }
}

DiscoveredNodeData changeNodeDataType(chip::Dnssd::DiscoveredNodeData nodeData)
{
    MTR_LOG_DEBUG("Change Node Data Type");
    DiscoveredNodeData nodeDataForDelegate;
    nodeDataForDelegate.hostName = [NSString stringWithUTF8String:nodeData.resolutionData.hostName];
    MTR_LOG_DEBUG("hostName: %@", nodeDataForDelegate.hostName);
    nodeDataForDelegate.ipAddress = [[NSMutableArray alloc] init];
    for (int i = 0; i < nodeData.resolutionData.numIPs; i++){
        char buf[chip::Transport::PeerAddress::kMaxToStringSize];
        nodeData.resolutionData.ipAddress[i].ToString(buf);
        [nodeDataForDelegate.ipAddress addObject:[NSString stringWithUTF8String:buf]];
        MTR_LOG_DEBUG("ip address: %@", nodeDataForDelegate.ipAddress[i]);
    }
    nodeDataForDelegate.port = nodeData.resolutionData.port;
    nodeDataForDelegate.supportsTcp = nodeData.resolutionData.supportsTcp;
    nodeDataForDelegate.vendorId = nodeData.commissionData.vendorId;
    nodeDataForDelegate.productId = nodeData.commissionData.productId;
    nodeDataForDelegate.longDiscriminatior = nodeData.commissionData.longDiscriminator;
    nodeDataForDelegate.pairingHint = nodeData.commissionData.pairingHint;
    nodeDataForDelegate.instanceName = [NSString stringWithUTF8String:nodeData.commissionData.instanceName];
    nodeDataForDelegate.commissioningMode = nodeData.commissionData.commissioningMode;
    return nodeDataForDelegate;
}


void MTRDeviceDiscoveryDelegateBridge::OnNodeDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    MTR_LOG_DEBUG("DeviceControllerDelegate Discovered Device.");
    id<MTRDeviceDiscoveryDelegate> strongDelegate = mDelegate;
    MTRDeviceController * strongController = mController;
    DiscoveredNodeData node = changeNodeDataType(nodeData);
    if (strongDelegate && mQueue && strongController) {
        if ([strongDelegate respondsToSelector:@selector(onDiscoveredDevice:)]) {
            dispatch_async(mQueue, ^{
                [strongDelegate onDiscoveredDevice:node];
            });
        }
    }
}
