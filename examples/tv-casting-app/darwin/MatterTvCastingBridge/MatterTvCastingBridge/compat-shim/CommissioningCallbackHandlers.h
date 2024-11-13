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

#import "../MatterError.h"
#import <Foundation/Foundation.h>

#ifndef CommissioningCallbacksHandlers_h
#define CommissioningCallbacksHandlers_h

__attribute__((deprecated("Use the APIs described in /examples/tv-casting-app/APIs.md instead.")))
@interface CommissioningCallbackHandlers : NSObject

- (CommissioningCallbackHandlers * _Nonnull)
    initWithCommissioningWindowRequestedHandler:(void (^_Nonnull)(MatterError * _Nonnull))commissioningWindowRequestedHandler
              commissioningWindowOpenedCallback:(void (^_Nonnull)(MatterError * _Nonnull))commissioningWindowOpenedCallback
                  commissioningCompleteCallback:(void (^_Nonnull)(MatterError * _Nonnull))commissioningCompleteCallback
            sessionEstablishmentStartedCallback:(void (^_Nullable)(void))sessionEstablishmentStartedCallback
                     sessionEstablishedCallback:(void (^_Nullable)(void))sessionEstablishedCallback
              sessionEstablishmentErrorCallback:(void (^_Nullable)(MatterError * _Nonnull))sessionEstablishmentErrorCallback
            sessionEstablishmentStoppedCallback:(void (^_Nullable)(void))sessionEstablishmentStoppedCallback;

/**
 * This is called when request to open the commissioning window has been made.
 */
@property void (^_Nullable commissioningWindowRequestedHandler)(MatterError * _Nonnull);

@property void (^_Nullable commissioningWindowOpenedCallback)(MatterError * _Nonnull);

/**
 * This is called when the commissioning has been completed
 */
@property void (^_Nullable commissioningCompleteCallback)(MatterError * _Nonnull);

/**
 * This is called when the PBKDFParamRequest is received and indicates the start of the session establishment process
 */
@property void (^_Nullable sessionEstablishmentStartedCallback)(void);

/**
 * This is called when the commissioning session has been established
 */
@property void (^_Nullable sessionEstablishedCallback)(void);

/**
 * This is called when the PASE establishment failed (such as, when an invalid passcode is provided) or PASE was established
 * fine but then the fail-safe expired (including being expired by the commissioner). The error param indicates the error that
 * occurred during session establishment or the error accompanying the fail-safe timeout.
 */
@property void (^_Nullable sessionEstablishmentErrorCallback)(MatterError * _Nonnull);

/**
 * This is called when the PASE establishment failed or PASE was established fine but then the fail-safe expired (including
 * being expired by the commissioner) AND the commissioning window is closed. The window may be closed because the commissioning
 * attempts limit was reached or advertising/listening for PASE failed.
 */
@property void (^_Nullable sessionEstablishmentStoppedCallback)(void);

@end

#endif /* CommissioningCallbacksHandlers_h */
