/**
 *    Copyright (c) 2022 Project CHIP Authors
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

#import "CHIPDeviceControllerStartupParams.h"
#import "CHIPDeviceControllerStartupParams_Internal.h"
#import "CHIPLogging.h"
#import "CHIPP256KeypairBridge.h"
#import "MTRCertificates.h"

#include <credentials/CHIPCert.h>
#include <lib/core/PeerId.h>

@implementation CHIPDeviceControllerStartupParams

- (instancetype)initWithKeypair:(id<CHIPKeypair>)nocSigner fabricId:(uint64_t)fabricId ipk:(NSData *)ipk
{
    if (!(self = [super init])) {
        return nil;
    }

    if (fabricId == chip::kUndefinedFabricId) {
        CHIP_LOG_ERROR("%llu is not a valid fabric id to initialize a device controller with", fabricId);
        return nil;
    }

    _nocSigner = nocSigner;
    _fabricId = fabricId;
    _ipk = ipk;

    return self;
}

@end

@implementation CHIPDeviceControllerStartupParamsInternal

- (BOOL)nocSignerMatchesCerts
{
    NSData * signingCert = self.intermediateCertificate;
    if (signingCert == nil) {
        signingCert = self.rootCertificate;
        if (signingCert == nil) {
            return NO;
        }
    }

    return [MTRCertificates keypair:self.nocSigner matchesCertificate:signingCert];
}

- (void)dealloc
{
    if (self.operationalKeypair != nullptr) {
        delete self.operationalKeypair;
        self.operationalKeypair = nullptr;
    }
}
@end
