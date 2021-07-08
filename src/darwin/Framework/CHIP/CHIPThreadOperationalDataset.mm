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

#import "CHIPThreadOperationalDataset.h"

#include "CHIPLogging.h"
#include <support/Span.h>
#include <support/ThreadOperationalDataset.h>

size_t const CHIPSizeThreadNetworkName = chip::Thread::kSizeNetworkName;
size_t const CHIPSizeThreadExtendedPanId = chip::Thread::kSizeExtendedPanId;
size_t const CHIPSizeThreadMasterKey = chip::Thread::kSizeMasterKey;
size_t const CHIPSizeThreadPSKc = chip::Thread::kSizePSKc;

@interface CHIPThreadOperationalDataset ()

@property (readonly) chip::Thread::OperationalDataset cppThreadOperationalDataset;

@end

@implementation CHIPThreadOperationalDataset

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
        CHIP_LOG_ERROR("Invalid ExtendedPANID");
        return NO;
    }
    uint8_t extendedPanId[chip::Thread::kSizeExtendedPanId];
    [self.extendedPANID getBytes:&extendedPanId length:chip::Thread::kSizeExtendedPanId];
    _cppThreadOperationalDataset.SetExtendedPanId(extendedPanId);

    if (![self _checkDataLength:self.masterKey expectedLength:chip::Thread::kSizeMasterKey]) {
        CHIP_LOG_ERROR("Invalid MasterKey");
        return NO;
    }
    uint8_t masterKey[chip::Thread::kSizeMasterKey];
    [self.masterKey getBytes:&masterKey length:chip::Thread::kSizeMasterKey];
    _cppThreadOperationalDataset.SetMasterKey(masterKey);

    if (![self _checkDataLength:self.PSKc expectedLength:chip::Thread::kSizePSKc]) {
        CHIP_LOG_ERROR("Invalid PKSc");
        return NO;
    }
    uint8_t PSKc[chip::Thread::kSizePSKc];
    [self.PSKc getBytes:&PSKc length:chip::Thread::kSizePSKc];
    _cppThreadOperationalDataset.SetPSKc(PSKc);

    _cppThreadOperationalDataset.SetChannel(self.channel);

    // Thread's PAN ID is 2 bytes
    if (![self _checkDataLength:self.panID expectedLength:2]) {
        CHIP_LOG_ERROR("Invalid PAN ID");
        return NO;
    }
    uint16_t * valuePtr = (uint16_t *) [self.panID bytes];
    if (valuePtr == nullptr) {
        return NO;
    }
    _cppThreadOperationalDataset.SetPanId(*valuePtr);

    return YES;
}

- (BOOL)_checkDataLength:(NSData *)data expectedLength:(size_t)expectedLength
{
    if (data.length != expectedLength) {
        CHIP_LOG_ERROR("Length Check Failed. Length:%tu is incorrect, must be %tu", data.length, expectedLength);
        return NO;
    }
    return YES;
}

- (NSData *)asData
{
    chip::ByteSpan span = _cppThreadOperationalDataset.AsByteSpan();
    return [NSData dataWithBytes:span.data() length:span.size()];
}

@end
