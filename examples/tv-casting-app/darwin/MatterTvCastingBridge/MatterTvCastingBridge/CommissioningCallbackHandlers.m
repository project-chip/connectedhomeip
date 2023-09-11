/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "CommissioningCallbackHandlers.h"

@implementation CommissioningCallbackHandlers

- (CommissioningCallbackHandlers * _Nonnull)
    initWithCommissioningWindowRequestedHandler:(void (^_Nonnull)(bool))commissioningWindowRequestedHandler
                  commissioningCompleteCallback:(void (^_Nonnull)(bool))commissioningCompleteCallback
            sessionEstablishmentStartedCallback:(void (^_Nullable)(void))sessionEstablishmentStartedCallback
                     sessionEstablishedCallback:(void (^_Nullable)(void))sessionEstablishedCallback
              sessionEstablishmentErrorCallback:(void (^_Nullable)(MatterError * _Nonnull))sessionEstablishmentErrorCallback
            sessionEstablishmentStoppedCallback:(void (^_Nullable)(void))sessionEstablishmentStoppedCallback
{
    self = [super init];
    if (self) {
        _commissioningWindowRequestedHandler = commissioningWindowRequestedHandler;
        _commissioningCompleteCallback = commissioningCompleteCallback;
        _sessionEstablishmentStartedCallback = sessionEstablishmentStartedCallback;
        _sessionEstablishedCallback = sessionEstablishedCallback;
        _sessionEstablishmentErrorCallback = sessionEstablishmentErrorCallback;
        _sessionEstablishmentStoppedCallback = sessionEstablishmentStoppedCallback;
    }
    return self;
}

@end
