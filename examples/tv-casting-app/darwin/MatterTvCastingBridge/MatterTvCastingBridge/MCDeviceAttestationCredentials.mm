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

#import "MCDeviceAttestationCredentials.h"
#import <Foundation/Foundation.h>

@implementation MCDeviceAttestationCredentials

- (instancetype)initWithCertificationDeclaration:(NSData * _Nonnull)certificationDeclaration
                             firmwareInformation:(NSData * _Nonnull)firmwareInformation
                           deviceAttestationCert:(NSData * _Nonnull)deviceAttestationCert
              productAttestationIntermediateCert:(NSData * _Nonnull)productAttestationIntermediateCert;
{
    if (self = [super init]) {
        _certificationDeclaration = certificationDeclaration;
        _firmwareInformation = firmwareInformation;
        _deviceAttestationCert = deviceAttestationCert;
        _productAttestationIntermediateCert = productAttestationIntermediateCert;
    }
    return self;
}

@end
