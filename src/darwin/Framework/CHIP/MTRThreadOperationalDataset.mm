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

#include "MTRLogging.h"
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>

size_t const MTRSizeThreadNetworkName = chip::Thread::kSizeNetworkName;
size_t const MTRSizeThreadExtendedPanId = chip::Thread::kSizeExtendedPanId;
size_t const MTRSizeThreadMasterKey = chip::Thread::kSizeMasterKey;
size_t const MTRSizeThreadPSKc = chip::Thread::kSizePSKc;

@interface MTRThreadOperationalDataset ()

@property (readonly) chip::Thread::OperationalDataset cppThreadOperationalDataset;

@end

@implementation MTRThreadOperationalDataset

- (nullable instancetype)initWithNetworkName:(NSString *)networkName
                               extendedPANID:(NSData *)extendedPANID
                                   masterKey:(NSData *)masterKey
                                        PSKc:(NSData *)PSKc
                                     channel:(uint16_t)channel
                                       panID:(NSData *)panID
{
    if (self = [super init]) {
        _networkName = networkName;
        _extendedPANID = extendedPANID;
        _masterKey = masterKey;
        _PSKc = PSKc;
        _channel = channel;
        _panID = panID;
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
    _cppThreadOperationalDataset.SetNetworkName([self.networkName cStringUsingEncoding:NSUTF8StringEncoding]);

    if (![self _checkDataLength:self.extendedPANID expectedLength:chip::Thread::kSizeExtendedPanId]) {
        MTR_LOG_ERROR("Invalid ExtendedPANID");
        return NO;
    }
    uint8_t extendedPanId[chip::Thread::kSizeExtendedPanId];
    [self.extendedPANID getBytes:&extendedPanId length:chip::Thread::kSizeExtendedPanId];
    _cppThreadOperationalDataset.SetExtendedPanId(extendedPanId);

    if (![self _checkDataLength:self.masterKey expectedLength:chip::Thread::kSizeMasterKey]) {
        MTR_LOG_ERROR("Invalid MasterKey");
        return NO;
    }
    uint8_t masterKey[chip::Thread::kSizeMasterKey];
    [self.masterKey getBytes:&masterKey length:chip::Thread::kSizeMasterKey];
    _cppThreadOperationalDataset.SetMasterKey(masterKey);

    if (![self _checkDataLength:self.PSKc expectedLength:chip::Thread::kSizePSKc]) {
        MTR_LOG_ERROR("Invalid PKSc");
        return NO;
    }
    uint8_t PSKc[chip::Thread::kSizePSKc];
    [self.PSKc getBytes:&PSKc length:chip::Thread::kSizePSKc];
    _cppThreadOperationalDataset.SetPSKc(PSKc);

    _cppThreadOperationalDataset.SetChannel(self.channel);

    // Thread's PAN ID is 2 bytes
    if (![self _checkDataLength:self.panID expectedLength:2]) {
        MTR_LOG_ERROR("Invalid PAN ID");
        return NO;
    }
    uint16_t * valuePtr = (uint16_t *) [self.panID bytes];
    if (valuePtr == nullptr) {
        return NO;
    }
    // The underlying CPP class assumes Big Endianness for the panID
    _cppThreadOperationalDataset.SetPanId(CFSwapInt16HostToBig(*valuePtr));

    return YES;
}

- (BOOL)_checkDataLength:(NSData *)data expectedLength:(size_t)expectedLength
{
    if (data.length != expectedLength) {
        MTR_LOG_ERROR("Length Check Failed. Length:%tu is incorrect, must be %tu", data.length, expectedLength);
        return NO;
    }
    return YES;
}

- (nullable instancetype)initWithData:(NSData *)data
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
    uint8_t extendedPANID[MTRSizeThreadExtendedPanId];
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
                       extendedPANID:[NSData dataWithBytes:extendedPANID length:MTRSizeThreadExtendedPanId]
                           masterKey:[NSData dataWithBytes:masterKey length:MTRSizeThreadMasterKey]
                                PSKc:[NSData dataWithBytes:pskc length:MTRSizeThreadPSKc]
                             channel:channel
                               panID:[NSData dataWithBytes:&panID length:sizeof(uint16_t)]];
}

- (NSData *)data
{
    chip::ByteSpan span = _cppThreadOperationalDataset.AsByteSpan();
    return [NSData dataWithBytes:span.data() length:span.size()];
}

@end
