/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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
#import <Foundation/Foundation.h>

#import "ConversionUtils.hpp"

@implementation ConversionUtils

+ (CHIP_ERROR)convertToCppAppParamsInfoFrom:(AppParameters * _Nonnull)objCAppParameters outAppParams:(AppParams &)outAppParams
{
    VerifyOrReturnError(objCAppParameters != nil, CHIP_ERROR_INVALID_ARGUMENT);

    if (objCAppParameters.rotatingDeviceIdUniqueId != nil) {
        chip::ByteSpan rotatingDeviceIdUniqueId
            = chip::ByteSpan(static_cast<const uint8_t *>(objCAppParameters.rotatingDeviceIdUniqueId.bytes),
                objCAppParameters.rotatingDeviceIdUniqueId.length);
        outAppParams.SetRotatingDeviceIdUniqueId(MakeOptional(rotatingDeviceIdUniqueId));
    }
    return CHIP_NO_ERROR;
}

+ (CHIP_ERROR)convertToCppTargetEndpointInfoFrom:(ContentApp * _Nonnull)objCContentApp
                           outTargetEndpointInfo:(TargetEndpointInfo &)outTargetEndpointInfo
{
    VerifyOrReturnError(objCContentApp.isInitialized, CHIP_ERROR_INVALID_ARGUMENT);
    outTargetEndpointInfo.Initialize(objCContentApp.endpointId);
    for (NSNumber * clusterId in objCContentApp.clusterIds) {
        VerifyOrReturnError(outTargetEndpointInfo.AddCluster([clusterId unsignedIntValue]), CHIP_ERROR_INVALID_ARGUMENT);
    }
    return CHIP_NO_ERROR;
}

+ (CHIP_ERROR)convertToCppDiscoveredNodeDataFrom:(DiscoveredNodeData * _Nonnull)objCDiscoveredNodeData
                           outDiscoveredNodeData:(chip::Dnssd::CommissionNodeData &)outDiscoveredNodeData
{
    // setting CommissionNodeData
    outDiscoveredNodeData.deviceType = objCDiscoveredNodeData.deviceType;
    outDiscoveredNodeData.vendorId = objCDiscoveredNodeData.vendorId;
    outDiscoveredNodeData.productId = objCDiscoveredNodeData.productId;
    outDiscoveredNodeData.longDiscriminator = objCDiscoveredNodeData.longDiscriminator;
    outDiscoveredNodeData.commissioningMode = objCDiscoveredNodeData.commissioningMode;
    outDiscoveredNodeData.pairingHint = objCDiscoveredNodeData.pairingHint;
    memset(outDiscoveredNodeData.deviceName, '\0', sizeof(outDiscoveredNodeData.deviceName));
    if (objCDiscoveredNodeData.deviceName != nullptr) {
        chip::Platform::CopyString(outDiscoveredNodeData.deviceName, chip::Dnssd::kMaxDeviceNameLen + 1,
            [objCDiscoveredNodeData.deviceName UTF8String]);
    }
    outDiscoveredNodeData.rotatingIdLen = objCDiscoveredNodeData.rotatingIdLen;
    memcpy(
        outDiscoveredNodeData.rotatingId, objCDiscoveredNodeData.rotatingId, objCDiscoveredNodeData.rotatingIdLen);

    // setting CommonResolutionData
    outDiscoveredNodeData.port = objCDiscoveredNodeData.port;
    memset(outDiscoveredNodeData.hostName, '\0', sizeof(outDiscoveredNodeData.hostName));
    if (objCDiscoveredNodeData.hostName != nullptr) {
        chip::Platform::CopyString(outDiscoveredNodeData.hostName, chip::Dnssd::kHostNameMaxLength + 1,
            [objCDiscoveredNodeData.hostName UTF8String]);
    }
    outDiscoveredNodeData.interfaceId = chip::Inet::InterfaceId(objCDiscoveredNodeData.platformInterface);
    outDiscoveredNodeData.numIPs = objCDiscoveredNodeData.numIPs;
    for (size_t i = 0; i < objCDiscoveredNodeData.numIPs; i++) {
        chip::Inet::IPAddress::FromString(
            [objCDiscoveredNodeData.ipAddresses[i] UTF8String], outDiscoveredNodeData.ipAddress[i]);
    }
    return CHIP_NO_ERROR;
}

+ (CHIP_ERROR)convertToCppTargetVideoPlayerInfoFrom:(VideoPlayer * _Nonnull)objCVideoPlayer
                           outTargetVideoPlayerInfo:(TargetVideoPlayerInfo &)outTargetVideoPlayerInfo
{
    VerifyOrReturnError(objCVideoPlayer.isInitialized, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(outTargetVideoPlayerInfo.Initialize(objCVideoPlayer.nodeId, objCVideoPlayer.fabricIndex, nullptr, nullptr,
        objCVideoPlayer.vendorId, objCVideoPlayer.productId, objCVideoPlayer.deviceType, [objCVideoPlayer.deviceName UTF8String],
        [objCVideoPlayer.hostName UTF8String], 0, nullptr, objCVideoPlayer.port, [objCVideoPlayer.instanceName UTF8String],
        chip::System::Clock::Timestamp(objCVideoPlayer.lastDiscoveredMs)));
    if (objCVideoPlayer.MACAddress != nil) {
        outTargetVideoPlayerInfo.SetMACAddress(
            chip::CharSpan([objCVideoPlayer.MACAddress UTF8String], objCVideoPlayer.MACAddress.length));
    }
    outTargetVideoPlayerInfo.SetIsAsleep(objCVideoPlayer.isAsleep);

    for (ContentApp * contentApp in objCVideoPlayer.contentApps) {
        TargetEndpointInfo * endpoint = outTargetVideoPlayerInfo.GetOrAddEndpoint(contentApp.endpointId);
        VerifyOrReturnError(endpoint != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure([ConversionUtils convertToCppTargetEndpointInfoFrom:contentApp outTargetEndpointInfo:*endpoint]);
    }
    return CHIP_NO_ERROR;
}

+ (ContentApp *)convertToObjCContentAppFrom:(TargetEndpointInfo * _Nonnull)cppTargetEndpointInfo
{
    ContentApp * objCContentApp = [ContentApp new];
    if (cppTargetEndpointInfo->IsInitialized()) {
        objCContentApp.endpointId = cppTargetEndpointInfo->GetEndpointId();
        objCContentApp.clusterIds = [NSMutableArray new];
        chip::ClusterId * clusterIds = cppTargetEndpointInfo->GetClusters();
        for (size_t i = 0; i < kMaxNumberOfClustersPerEndpoint && clusterIds[i] != chip::kInvalidClusterId; i++) {
            objCContentApp.clusterIds[i] = @(clusterIds[i]);
        }
        objCContentApp.isInitialized = true;
    }
    return objCContentApp;
}

+ (DiscoveredNodeData *)convertToObjCDiscoveredNodeDataFrom:(const chip::Dnssd::CommissionNodeData * _Nonnull)cppDiscoveredNodedata
{
    DiscoveredNodeData * objCDiscoveredNodeData = [DiscoveredNodeData new];

    // from CommissionNodeData
    objCDiscoveredNodeData.deviceType = cppDiscoveredNodedata->deviceType;
    objCDiscoveredNodeData.vendorId = cppDiscoveredNodedata->vendorId;
    objCDiscoveredNodeData.productId = cppDiscoveredNodedata->productId;
    objCDiscoveredNodeData.longDiscriminator = cppDiscoveredNodedata->longDiscriminator;
    objCDiscoveredNodeData.commissioningMode = cppDiscoveredNodedata->commissioningMode;
    objCDiscoveredNodeData.pairingHint = cppDiscoveredNodedata->pairingHint;
    objCDiscoveredNodeData.deviceName = [NSString stringWithCString:cppDiscoveredNodedata->deviceName
                                                           encoding:NSUTF8StringEncoding];
    objCDiscoveredNodeData.rotatingIdLen = cppDiscoveredNodedata->rotatingIdLen;
    objCDiscoveredNodeData.rotatingId = cppDiscoveredNodedata->rotatingId;
    objCDiscoveredNodeData.instanceName = [NSString stringWithCString:cppDiscoveredNodedata->instanceName
                                                             encoding:NSUTF8StringEncoding];

    // from CommonResolutionData
    objCDiscoveredNodeData.port = cppDiscoveredNodedata->port;
    objCDiscoveredNodeData.hostName = [NSString stringWithCString:cppDiscoveredNodedata->hostName
                                                         encoding:NSUTF8StringEncoding];
    objCDiscoveredNodeData.platformInterface = cppDiscoveredNodedata->interfaceId.GetPlatformInterface();
    objCDiscoveredNodeData.numIPs = cppDiscoveredNodedata->numIPs;
    if (cppDiscoveredNodedata->numIPs > 0) {
        objCDiscoveredNodeData.ipAddresses = [NSMutableArray new];
    }
    for (size_t i = 0; i < cppDiscoveredNodedata->numIPs; i++) {
        char addrCString[chip::Inet::IPAddress::kMaxStringLength];
        cppDiscoveredNodedata->ipAddress[i].ToString(addrCString, chip::Inet::IPAddress::kMaxStringLength);
        objCDiscoveredNodeData.ipAddresses[i] = [NSString stringWithCString:addrCString encoding:NSASCIIStringEncoding];
    }
    return objCDiscoveredNodeData;
}

+ (DiscoveredNodeData *)convertToDiscoveredNodeDataFrom:(TargetVideoPlayerInfo * _Nonnull)cppTargetVideoPlayerInfo
{
    DiscoveredNodeData * objCDiscoveredNodeData = [DiscoveredNodeData new];

    objCDiscoveredNodeData.deviceType = cppTargetVideoPlayerInfo->GetDeviceType();
    objCDiscoveredNodeData.vendorId = cppTargetVideoPlayerInfo->GetVendorId();
    objCDiscoveredNodeData.productId = cppTargetVideoPlayerInfo->GetProductId();
    objCDiscoveredNodeData.deviceName = [NSString stringWithCString:cppTargetVideoPlayerInfo->GetDeviceName()
                                                           encoding:NSUTF8StringEncoding];
    objCDiscoveredNodeData.instanceName = [NSString stringWithCString:cppTargetVideoPlayerInfo->GetInstanceName()
                                                             encoding:NSUTF8StringEncoding];

    objCDiscoveredNodeData.port = cppTargetVideoPlayerInfo->GetPort();
    objCDiscoveredNodeData.hostName = [NSString stringWithCString:cppTargetVideoPlayerInfo->GetHostName()
                                                         encoding:NSUTF8StringEncoding];
    objCDiscoveredNodeData.numIPs = cppTargetVideoPlayerInfo->GetNumIPs();
    if (cppTargetVideoPlayerInfo->GetNumIPs() > 0) {
        objCDiscoveredNodeData.ipAddresses = [NSMutableArray new];
    }
    for (size_t i = 0; i < cppTargetVideoPlayerInfo->GetNumIPs(); i++) {
        char addrCString[chip::Inet::IPAddress::kMaxStringLength];
        cppTargetVideoPlayerInfo->GetIpAddresses()[i].ToString(addrCString, chip::Inet::IPAddress::kMaxStringLength);
        objCDiscoveredNodeData.ipAddresses[i] = [NSString stringWithCString:addrCString encoding:NSASCIIStringEncoding];
    }

    VideoPlayer * connectableVideoPlayer = [ConversionUtils convertToObjCVideoPlayerFrom:cppTargetVideoPlayerInfo];
    [objCDiscoveredNodeData setConnectableVideoPlayer:connectableVideoPlayer];
    return objCDiscoveredNodeData;
}

+ (VideoPlayer *)convertToObjCVideoPlayerFrom:(TargetVideoPlayerInfo * _Nonnull)cppTargetVideoPlayerInfo
{
    VideoPlayer * objCVideoPlayer = [VideoPlayer new];
    if (cppTargetVideoPlayerInfo->IsInitialized()) {
        objCVideoPlayer.nodeId = cppTargetVideoPlayerInfo->GetNodeId();
        objCVideoPlayer.fabricIndex = cppTargetVideoPlayerInfo->GetFabricIndex();
        objCVideoPlayer.vendorId = cppTargetVideoPlayerInfo->GetVendorId();
        objCVideoPlayer.productId = cppTargetVideoPlayerInfo->GetProductId();
        objCVideoPlayer.deviceType = cppTargetVideoPlayerInfo->GetDeviceType();
        objCVideoPlayer.isConnected = (cppTargetVideoPlayerInfo->GetOperationalDeviceProxy() != nil);
        objCVideoPlayer.deviceName = [NSString stringWithCString:cppTargetVideoPlayerInfo->GetDeviceName()
                                                        encoding:NSUTF8StringEncoding];
        objCVideoPlayer.port = cppTargetVideoPlayerInfo->GetPort();
        if (cppTargetVideoPlayerInfo->GetMACAddress() != nullptr && cppTargetVideoPlayerInfo->GetMACAddress()->size() > 0) {
            objCVideoPlayer.MACAddress = [NSString stringWithCString:cppTargetVideoPlayerInfo->GetMACAddress()->data()
                                                            encoding:NSUTF8StringEncoding];
        }
        objCVideoPlayer.isAsleep = cppTargetVideoPlayerInfo->IsAsleep();
        objCVideoPlayer.lastDiscoveredMs = cppTargetVideoPlayerInfo->GetLastDiscovered().count();
        objCVideoPlayer.instanceName = [NSString stringWithCString:cppTargetVideoPlayerInfo->GetInstanceName()
                                                          encoding:NSUTF8StringEncoding];
        objCVideoPlayer.hostName = [NSString stringWithCString:cppTargetVideoPlayerInfo->GetHostName()
                                                      encoding:NSUTF8StringEncoding];

        objCVideoPlayer.contentApps = [NSMutableArray new];
        TargetEndpointInfo * cppTargetEndpointInfos = cppTargetVideoPlayerInfo->GetEndpoints();
        for (size_t i = 0; i < kMaxNumberOfEndpoints && cppTargetEndpointInfos[i].IsInitialized(); i++) {
            objCVideoPlayer.contentApps[i] = [ConversionUtils convertToObjCContentAppFrom:&cppTargetEndpointInfos[i]];
        }
        objCVideoPlayer.isInitialized = true;
    }
    return objCVideoPlayer;
}

@end
