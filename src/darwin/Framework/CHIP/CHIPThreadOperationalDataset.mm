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
#include <support/ThreadOperationalDataset.h>
#include <support/Span.h>

size_t const CHIPSizeThreadNetworkName     = chip::Thread::kSizeNetworkName;
size_t const CHIPSizeThreadExtendedPanId   = chip::Thread::kSizeExtendedPanId;
size_t const CHIPSizeThreadMasterKey       = chip::Thread::kSizeMasterKey;
size_t const CHIPSizeThreadMeshLocalPrefix = chip::Thread::kSizeMeshLocalPrefix;
size_t const CHIPSizeThreadPSKc            = chip::Thread::kSizePSKc;

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
    if (self = [super init])
    {
        _networkName = networkName;
        _extendedPANID = extendedPANID;
        _masterKey = masterKey;
        _PSKc = PSKc;
        _channel = channel;
        _panID = panID;
        _cppThreadOperationalDataset = chip::Thread::OperationalDataset();
        [self _populateCppOperationalDataset];
        if (!_cppThreadOperationalDataset.IsValid(_cppThreadOperationalDataset.AsByteSpan()))
        {
            CHIP_LOG_ERROR("Error: Thread Operational Dataset is malformed, cannot initialize.");
            return nil;
        }
        return self;
    }
    return nil;
}

- (void)_populateCppOperationalDataset
{
    _cppThreadOperationalDataset.Clear();
    _cppThreadOperationalDataset.SetNetworkName([self.networkName cStringUsingEncoding:NSUTF8StringEncoding]);

    uint8_t extendedPanId[chip::Thread::kSizeExtendedPanId];
    [self.extendedPANID getBytes:&extendedPanId length:chip::Thread::kSizeExtendedPanId];
    _cppThreadOperationalDataset.SetExtendedPanId(extendedPanId);

    uint8_t masterKey[chip::Thread::kSizeMasterKey];
    [self.masterKey getBytes:&masterKey length:chip::Thread::kSizeMasterKey];
    _cppThreadOperationalDataset.SetMasterKey(masterKey);

    uint8_t PSKc[chip::Thread::kSizePSKc];
    [self.PSKc getBytes:&PSKc length:chip::Thread::kSizePSKc];
    _cppThreadOperationalDataset.SetPSKc(PSKc);

    _cppThreadOperationalDataset.SetChannel(self.channel);

    uint16_t *valuePtr = (uint16_t *)[self.panID bytes];
    if (valuePtr != nullptr)
    {
        _cppThreadOperationalDataset.SetPanId(*valuePtr);
    }
}

- (NSData *)asData
{
    chip::ByteSpan span = _cppThreadOperationalDataset.AsByteSpan();
    return [NSData dataWithBytes:span.data() length:span.size()];
}

@end
