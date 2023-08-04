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
