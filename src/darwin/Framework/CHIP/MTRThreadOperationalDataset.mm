/**
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

#import "MTRThreadOperationalDataset.h"
#import "NSDataSpanConversion.h"

#include "MTRLogging_Internal.h"
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>

size_t const MTRSizeThreadNetworkName = chip::Thread::kSizeNetworkName;
size_t const MTRSizeThreadExtendedPANID = chip::Thread::kSizeExtendedPanId;
size_t const MTRSizeThreadExtendedPanId = MTRSizeThreadExtendedPANID;
size_t const MTRSizeThreadMasterKey = chip::Thread::kSizeMasterKey;
size_t const MTRSizeThreadPSKc = chip::Thread::kSizePSKc;
size_t const MTRSizeThreadPANID = 2; // Thread's PAN ID is 2 bytes

@implementation MTRThreadOperationalDataset {
    chip::Thread::OperationalDataset _cppThreadOperationalDataset;
}

- (instancetype _Nullable)initWithNetworkName:(NSString *)networkName
                                extendedPANID:(NSData *)extendedPANID
                                    masterKey:(NSData *)masterKey
                                         PSKc:(NSData *)PSKc
                                channelNumber:(NSNumber *)channelNumber
                                        panID:(NSData *)panID
{
    if (self = [super init]) {
        _networkName = [networkName copy];
        _extendedPANID = [extendedPANID copy];
        _masterKey = [masterKey copy];
        _PSKc = [PSKc copy];
        _channelNumber = [channelNumber copy];
        _panID = [panID copy];
        _cppThreadOperationalDataset = chip::Thread::OperationalDataset();
        if ([self _populateCppOperationalDataset]) {
            return self;
        }
    }
    return nil;
}

- (BOOL)_populateCppOperationalDataset
{
    _cppThreadOperationalDataset.Clear();
    _cppThreadOperationalDataset.SetNetworkName(self.networkName.UTF8String);

    if (![self _checkDataLength:self.extendedPANID expectedLength:MTRSizeThreadExtendedPANID]) {
        MTR_LOG_ERROR("Invalid ExtendedPANID");
        return NO;
    }
    uint8_t extendedPanId[MTRSizeThreadExtendedPANID];
    [self.extendedPANID getBytes:&extendedPanId length:MTRSizeThreadExtendedPANID];
    _cppThreadOperationalDataset.SetExtendedPanId(extendedPanId);

    if (![self _checkDataLength:self.masterKey expectedLength:MTRSizeThreadMasterKey]) {
        MTR_LOG_ERROR("Invalid MasterKey");
        return NO;
    }
    uint8_t masterKey[MTRSizeThreadMasterKey];
    [self.masterKey getBytes:&masterKey length:MTRSizeThreadMasterKey];
    _cppThreadOperationalDataset.SetMasterKey(masterKey);

    if (![self _checkDataLength:self.PSKc expectedLength:MTRSizeThreadPSKc]) {
        MTR_LOG_ERROR("Invalid PKSc");
        return NO;
    }
    uint8_t PSKc[MTRSizeThreadPSKc];
    [self.PSKc getBytes:&PSKc length:MTRSizeThreadPSKc];
    _cppThreadOperationalDataset.SetPSKc(PSKc);

    _cppThreadOperationalDataset.SetChannel([self.channelNumber unsignedShortValue]);

    // Thread's PAN ID is 2 bytes
    if (![self _checkDataLength:self.panID expectedLength:MTRSizeThreadPANID]) {
        MTR_LOG_ERROR("Invalid PAN ID");
        return NO;
    }
    uint16_t panID;
    memcpy(&panID, [self.panID bytes], MTRSizeThreadPANID);
    // The underlying CPP class assumes Big Endianness for the panID
    _cppThreadOperationalDataset.SetPanId(CFSwapInt16HostToBig(panID));

    return YES;
}

- (BOOL)_checkDataLength:(NSData *)data expectedLength:(size_t)expectedLength
{
    if (data.length != expectedLength) {
        MTR_LOG_ERROR("Length Check Failed. Length:%lu is incorrect, must be %tu", static_cast<unsigned long>(data.length), expectedLength);
        return NO;
    }
    return YES;
}

- (instancetype _Nullable)initWithData:(NSData *)data
{
    chip::ByteSpan span = chip::ByteSpan((uint8_t *) data.bytes, data.length);
    auto dataset = chip::Thread::OperationalDataset();
    CHIP_ERROR error = dataset.Init(span);
    if (error != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Failed to parse data, cannot construct Operational Dataset. %s", chip::ErrorStr(error));
        return nil;
    }
    // len+1 for null termination
    char networkName[MTRSizeThreadNetworkName + 1];
    uint8_t pskc[MTRSizeThreadPSKc];
    uint8_t extendedPANID[MTRSizeThreadExtendedPANID];
    uint8_t masterKey[MTRSizeThreadMasterKey];
    uint16_t panID;
    uint16_t channel;
    dataset.GetNetworkName(networkName);
    dataset.GetExtendedPanId(extendedPANID);
    dataset.GetMasterKey(masterKey);
    dataset.GetPSKc(pskc);
    dataset.GetPanId(panID);
    dataset.GetChannel(channel);
    panID = CFSwapInt16BigToHost(panID);

    return [self initWithNetworkName:[NSString stringWithUTF8String:networkName]
                       extendedPANID:AsData(chip::ByteSpan(extendedPANID))
                           masterKey:AsData(chip::ByteSpan(masterKey))
                                PSKc:AsData(chip::ByteSpan(pskc))
                       channelNumber:@(channel)
                               panID:[NSData dataWithBytes:&panID length:sizeof(uint16_t)]];
}

- (NSData *)data
{
    chip::ByteSpan span = _cppThreadOperationalDataset.AsByteSpan();
    return AsData(span);
}

@end

@implementation MTRThreadOperationalDataset (Deprecated)

- (void)setChannel:(uint16_t)channel
{
    _channelNumber = @(channel);
}

- (uint16_t)channel
{
    return [self.channelNumber unsignedShortValue];
}

- (nullable instancetype)initWithNetworkName:(NSString *)networkName
                               extendedPANID:(NSData *)extendedPANID
                                   masterKey:(NSData *)masterKey
                                        PSKc:(NSData *)PSKc
                                     channel:(uint16_t)channel
                                       panID:(NSData *)panID
{
    return [self initWithNetworkName:networkName
                       extendedPANID:extendedPANID
                           masterKey:masterKey
                                PSKc:PSKc
                       channelNumber:@(channel)
                               panID:panID];
}

@end
