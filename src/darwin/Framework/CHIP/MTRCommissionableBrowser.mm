/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "MTRCommissionableBrowser.h"
#import "MTRCommissionableBrowserDelegate.h"
#import "MTRCommissionableBrowserResult_Internal.h"
#import "MTRDeviceController.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#include <tracing/metric_macros.h>

#include <controller/CHIPDeviceController.h>
#include <lib/dnssd/platform/Dnssd.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::Dnssd;
using namespace chip::DeviceLayer;

#if CONFIG_NETWORK_LAYER_BLE
#include <platform/Darwin/BleScannerDelegate.h>
using namespace chip::Ble;
#endif // CONFIG_NETWORK_LAYER_BLE

constexpr char kBleKey[] = "BLE";

using namespace chip::Tracing::DarwinFramework;

@implementation MTRCommissionableBrowserResultInterfaces
@end

@interface MTRCommissionableBrowserResult ()
@property (nonatomic) NSString * instanceName;
@property (nonatomic) NSNumber * vendorID;
@property (nonatomic) NSNumber * productID;
@property (nonatomic) NSNumber * discriminator;
@property (nonatomic) BOOL commissioningMode;
@end

@implementation MTRCommissionableBrowserResult
@end

class CommissionableBrowserInternal : public DiscoverNodeDelegate,
                                      public DnssdBrowseDelegate
#if CONFIG_NETWORK_LAYER_BLE
    ,
                                      public BleScannerDelegate
#endif // CONFIG_NETWORK_LAYER_BLE
{
public:
    CHIP_ERROR Start(id<MTRCommissionableBrowserDelegate> delegate, MTRDeviceController * controller, dispatch_queue_t queue)
    {
        assertChipStackLockedByCurrentThread();

        VerifyOrReturnError(mDelegate == nil, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mController == nil, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mDispatchQueue == nil, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mDiscoveredResults == nil, CHIP_ERROR_INCORRECT_STATE);

        mDelegate = delegate;
        mController = controller;
        mDispatchQueue = queue;
        mDiscoveredResults = [[NSMutableDictionary alloc] init];
        ResetCounters();

#if CONFIG_NETWORK_LAYER_BLE
        ReturnErrorOnFailure(PlatformMgrImpl().StartBleScan(this));
#endif // CONFIG_NETWORK_LAYER_BLE

        ReturnErrorOnFailure(Resolver::Instance().Init(chip::DeviceLayer::UDPEndPointManager()));

        char serviceName[kMaxCommissionableServiceNameSize];
        auto filter = DiscoveryFilterType::kNone;
        ReturnErrorOnFailure(MakeServiceTypeName(serviceName, sizeof(serviceName), filter, DiscoveryType::kCommissionableNode));

        return ChipDnssdBrowse(serviceName, DnssdServiceProtocol::kDnssdProtocolUdp, chip::Inet::IPAddressType::kAny,
            chip::Inet::InterfaceId::Null(), this);
    }

    CHIP_ERROR Stop()
    {
        assertChipStackLockedByCurrentThread();

        VerifyOrReturnError(mDelegate != nil, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mController != nil, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mDispatchQueue != nil, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mDiscoveredResults != nil, CHIP_ERROR_INCORRECT_STATE);

        mDelegate = nil;
        mController = nil;
        mDispatchQueue = nil;

        ClearBleDiscoveredDevices();
        ClearDnssdDiscoveredDevices();
        ResetCounters();
        mDiscoveredResults = nil;

#if CONFIG_NETWORK_LAYER_BLE
        ReturnErrorOnFailure(PlatformMgrImpl().StopBleScan());
#endif // CONFIG_NETWORK_LAYER_BLE

        return ChipDnssdStopBrowse(this);
    }

    void ResetCounters()
    {
        mOnNetworkDevicesAdded = mOnNetworkDevicesRemoved = 0;
        mBLEDevicesAdded = mBLEDevicesRemoved = 0;
    }

    void ClearBleDiscoveredDevices()
    {
        NSMutableDictionary<NSString *, MTRCommissionableBrowserResult *> * discoveredResultsCopy = @ {}.mutableCopy;

        for (NSString * key in mDiscoveredResults) {
            if (![mDiscoveredResults[key].instanceName isEqual:[NSString stringWithUTF8String:kBleKey]]) {
                discoveredResultsCopy[key] = mDiscoveredResults[key];
            }
        }

        mDiscoveredResults = discoveredResultsCopy;
    }

    void ClearDnssdDiscoveredDevices()
    {
        NSMutableDictionary<NSString *, MTRCommissionableBrowserResult *> * discoveredResultsCopy = @ {}.mutableCopy;

        for (NSString * key in mDiscoveredResults) {
            auto * interfaces = mDiscoveredResults[key].interfaces;
            for (id interfaceKey in interfaces) {
                // Check if the interface data has been resolved already, otherwise, just inform the
                // back end that we may not need it anymore.
                if (!interfaces[interfaceKey].resolutionData.HasValue()) {
                    ChipDnssdResolveNoLongerNeeded(key.UTF8String);
                }
            }

            if ([mDiscoveredResults[key].instanceName isEqual:[NSString stringWithUTF8String:kBleKey]]) {
                discoveredResultsCopy[key] = mDiscoveredResults[key];
            }
        }

        mDiscoveredResults = discoveredResultsCopy;
    }

    /////////// DiscoverNodeDelegate Interface /////////
    void OnNodeDiscovered(const DiscoveredNodeData & nodeData) override
    {
        assertChipStackLockedByCurrentThread();

        if (!nodeData.Is<CommissionNodeData>()) {
            // not commissionable/commissioners node
            return;
        }

        auto & commissionData = nodeData.Get<CommissionNodeData>();
        auto key = [NSString stringWithUTF8String:commissionData.instanceName];
        if ([mDiscoveredResults objectForKey:key] == nil) {
            // It should not happens.
            return;
        }

        auto result = mDiscoveredResults[key];
        result.instanceName = key;
        result.vendorID = @(commissionData.vendorId);
        result.productID = @(commissionData.productId);
        result.discriminator = @(commissionData.longDiscriminator);
        result.commissioningMode = commissionData.commissioningMode != 0;

        const CommonResolutionData & resolutionData = commissionData;
        auto * interfaces = result.interfaces;
        interfaces[@(resolutionData.interfaceId.GetPlatformInterface())].resolutionData = chip::MakeOptional(resolutionData);

        // Check if any interface for the advertised service has been resolved already. If so,
        // we don't need to inform the delegate about it since it already knows that something
        // is available.
        auto shouldDispatchToDelegate = YES;
        for (id interfaceKey in interfaces) {
            if (![interfaceKey isEqual:@(resolutionData.interfaceId.GetPlatformInterface())]
                && interfaces[interfaceKey].resolutionData.HasValue()) {
                shouldDispatchToDelegate = NO;
                break;
            }
        }

        if (!shouldDispatchToDelegate) {
            return;
        }

        dispatch_async(mDispatchQueue, ^{
            [mDelegate controller:mController didFindCommissionableDevice:result];
        });
    }

    /////////// DnssdBrowseDelegate Interface /////////
    void OnBrowseAdd(DnssdService service) override
    {
        assertChipStackLockedByCurrentThread();

        VerifyOrReturn(mDelegate != nil);
        VerifyOrReturn(mController != nil);
        VerifyOrReturn(mDispatchQueue != nil);

        MATTER_LOG_METRIC(kMetricOnNetworkDevicesAdded, ++mOnNetworkDevicesAdded);

        auto key = [NSString stringWithUTF8String:service.mName];
        if ([mDiscoveredResults objectForKey:key] == nil) {
            mDiscoveredResults[key] = [[MTRCommissionableBrowserResult alloc] init];
            mDiscoveredResults[key].interfaces = [[NSMutableDictionary alloc] init];
        }

        auto * interfaces = mDiscoveredResults[key].interfaces;
        auto interfaceKey = @(service.mInterface.GetPlatformInterface());
        interfaces[interfaceKey] = [[MTRCommissionableBrowserResultInterfaces alloc] init];

        LogErrorOnFailure(ChipDnssdResolve(&service, service.mInterface, this));
    }

    void OnBrowseRemove(DnssdService service) override
    {
        assertChipStackLockedByCurrentThread();

        VerifyOrReturn(mDelegate != nil);
        VerifyOrReturn(mController != nil);
        VerifyOrReturn(mDispatchQueue != nil);

        MATTER_LOG_METRIC(kMetricOnNetworkDevicesRemoved, ++mOnNetworkDevicesRemoved);

        auto key = [NSString stringWithUTF8String:service.mName];
        if ([mDiscoveredResults objectForKey:key] == nil) {
            // It should not happens.
            return;
        }

        auto result = mDiscoveredResults[key];
        auto * interfaces = result.interfaces;
        auto interfaceKey = @(service.mInterface.GetPlatformInterface());

        // Check if the interface data has been resolved already, otherwise, just inform the
        // back end that we may not need it anymore.
        if (!interfaces[interfaceKey].resolutionData.HasValue()) {
            ChipDnssdResolveNoLongerNeeded(service.mName);
        }

        // Delete the interface placeholder.
        interfaces[interfaceKey] = nil;

        // If there is nothing else to resolve for the given instance name, just remove it
        // too and informs the delegate that it is gone.
        if ([interfaces count] == 0) {
            dispatch_async(mDispatchQueue, ^{
                [mDelegate controller:mController didRemoveCommissionableDevice:result];
            });

            mDiscoveredResults[key] = nil;
        }
    }

    void OnBrowseStop(CHIP_ERROR error) override
    {
        assertChipStackLockedByCurrentThread();

        ClearDnssdDiscoveredDevices();
    }

#if CONFIG_NETWORK_LAYER_BLE
    /////////// BleScannerDelegate Interface /////////
    void OnBleScanAdd(BLE_CONNECTION_OBJECT connObj, const ChipBLEDeviceIdentificationInfo & info) override
    {
        assertChipStackLockedByCurrentThread();

        auto result = [[MTRCommissionableBrowserResult alloc] init];
        result.instanceName = [NSString stringWithUTF8String:kBleKey];
        result.vendorID = @(info.GetVendorId());
        result.productID = @(info.GetProductId());
        result.discriminator = @(info.GetDeviceDiscriminator());
        result.commissioningMode = YES;
        result.params = chip::MakeOptional(chip::Controller::SetUpCodePairerParameters(connObj, false /* connected */));

        MATTER_LOG_METRIC(kMetricBLEDevicesAdded, ++mBLEDevicesAdded);

        auto key = [NSString stringWithFormat:@"%@", connObj];
        mDiscoveredResults[key] = result;

        dispatch_async(mDispatchQueue, ^{
            [mDelegate controller:mController didFindCommissionableDevice:result];
        });
    }

    void OnBleScanRemove(BLE_CONNECTION_OBJECT connObj) override
    {
        assertChipStackLockedByCurrentThread();

        auto key = [NSString stringWithFormat:@"%@", connObj];
        if ([mDiscoveredResults objectForKey:key] == nil) {
            // It should not happens.
            return;
        }

        auto result = mDiscoveredResults[key];
        mDiscoveredResults[key] = nil;

        MATTER_LOG_METRIC(kMetricBLEDevicesRemoved, ++mBLEDevicesRemoved);

        dispatch_async(mDispatchQueue, ^{
            [mDelegate controller:mController didFindCommissionableDevice:result];
        });
    }
#endif // CONFIG_NETWORK_LAYER_BLE

private:
    dispatch_queue_t mDispatchQueue;
    id<MTRCommissionableBrowserDelegate> mDelegate;
    MTRDeviceController * mController;
    NSMutableDictionary<NSString *, MTRCommissionableBrowserResult *> * mDiscoveredResults;
    int32_t mOnNetworkDevicesAdded;
    int32_t mOnNetworkDevicesRemoved;
    int32_t mBLEDevicesAdded;
    int32_t mBLEDevicesRemoved;
};

@interface MTRCommissionableBrowser ()
@property (strong, nonatomic) dispatch_queue_t queue;
@property (nonatomic, readonly) id<MTRCommissionableBrowserDelegate> delegate;
@property (nonatomic, readonly) MTRDeviceController * controller;
@property (unsafe_unretained, nonatomic) CommissionableBrowserInternal browser;
@end

@implementation MTRCommissionableBrowser
- (instancetype)initWithDelegate:(id<MTRCommissionableBrowserDelegate>)delegate
                      controller:(MTRDeviceController *)controller
                           queue:(dispatch_queue_t)queue
{
    if (self = [super init]) {
        _delegate = delegate;
        _controller = controller;
        _queue = queue;
    }
    return self;
}

- (BOOL)start
{
    VerifyOrReturnValue(CHIP_NO_ERROR == _browser.Start(_delegate, _controller, _queue), NO);
    return YES;
}

- (BOOL)stop
{
    VerifyOrReturnValue(CHIP_NO_ERROR == _browser.Stop(), NO);
    _delegate = nil;
    _controller = nil;
    _queue = nil;
    return YES;
}

@end
