/**
 * SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
