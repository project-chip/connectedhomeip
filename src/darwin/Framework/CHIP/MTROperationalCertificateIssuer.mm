/**
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#import "MTROperationalCertificateIssuer.h"

@implementation MTROperationalCertificateChain

- (instancetype)initWithOperationalCertificate:(MTRCertificateDERBytes)operationalCertificate
                       intermediateCertificate:(nullable MTRCertificateDERBytes)intermediateCertificate
                               rootCertificate:(MTRCertificateDERBytes)rootCertificate
                                  adminSubject:(nullable NSNumber *)adminSubject
{
    if (self = [super init]) {
        _operationalCertificate = operationalCertificate;
        _intermediateCertificate = intermediateCertificate;
        _rootCertificate = rootCertificate;
        _adminSubject = adminSubject;
    }
    return self;
}

@end
