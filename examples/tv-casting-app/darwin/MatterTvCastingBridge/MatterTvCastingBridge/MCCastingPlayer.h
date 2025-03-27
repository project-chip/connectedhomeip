/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#import "MCConnectionCallbacks.h"
#import "MCEndpointFilter.h"
#import "MCIdentificationDeclarationOptions.h"

#import <Foundation/Foundation.h>

#ifndef MCCastingPlayer_h
#define MCCastingPlayer_h

@class MCEndpoint;

/**
 * @brief Represents CastingPlayer ConnectionState.
 * @note Should be kept up to date with matter::casting::core::ConnectionState.
 */
typedef enum {
    MC_CASTING_PLAYER_NOT_CONNECTED,
    MC_CASTING_PLAYER_CONNECTING,
    MC_CASTING_PLAYER_CONNECTED,
} MCCastingPlayerConnectionState;

/**
 * @brief MCCastingPlayer represents a Matter commissioner that is able to play media to a physical
 * output or to a display screen which is part of the device.
 */
@interface MCCastingPlayer : NSObject

+ (NSInteger)kMinCommissioningWindowTimeoutSec;

/**
 * @brief Verifies that a connection exists with this CastingPlayer, or triggers a new
 *     commissioning session request. If the CastingApp does not have the nodeId and fabricIndex
 *     of this CastingPlayer cached on disk, this will execute the User Directed Commissioning
 *     (UDC) process by sending an IdentificationDeclaration message to the Commissioner. For
 *     certain UDC features, where a Commissioner reply is expected, this API needs to be followed
 *     up with the continueConnecting() API defiend below. See the Matter UDC specification or
 *     parameter class definitions for details on features not included in the description below.
 * @param connectionCallbacks contains the connectionCompleteCallback (Required) and
 *     commissionerDeclarationCallback (Optional) callbacks defiend in MCConnectionCallbacks.
 *     <p>For example: During CastingPlayer/Commissioner-Generated passcode commissioning, the
 *     Commissioner replies with a CommissionerDeclaration message with PasscodeDialogDisplayed
 *     and CommissionerPasscode set to true. Given these Commissioner state details, the client is
 *     expected to perform some actions, detailed in the continueConnecting() API below, and then
 *     call the continueConnecting() API to complete the process.
 * @param timeout time (in sec) to keep the commissioning window open, if commissioning is required.
 *     Needs to be >= MCCastingPlayer.kMinCommissioningWindowTimeoutSec.
 * @param identificationDeclarationOptions (Optional) Parameters in the IdentificationDeclaration
 *     message sent by the Commissionee to the Commissioner. These parameters specify the
 *     information relating to the requested commissioning session.
 *     <p>For example: To invoke the CastingPlayer/Commissioner-Generated passcode commissioning
 *     flow, the client would call this API with IdentificationDeclarationOptions containing
 *     CommissionerPasscode set to true. See IdentificationDeclarationOptions.java for a complete
 *     list of optional parameters.
 *     <p>Furthermore, attributes (such as VendorId) describe the TargetApp/Endpoint that the client
 *     wants to interact with after commissioning. If this value is passed in,
 *     verifyOrEstablishConnection() will force UDC, in case the desired TargetApp is not found in
 *     the on-device cached information/CastingStore.
 * @return nil if request submitted successfully, otherwise a NSError object corresponding to the error.
 */
- (NSError * _Nullable)verifyOrEstablishConnectionWithCallbacks:(MCConnectionCallbacks * _Nonnull)connectionCallbacks
                                                        timeout:(long)timeout
                               identificationDeclarationOptions:(MCIdentificationDeclarationOptions * _Nullable)identificationDeclarationOptions;

/**
 * @brief Verifies that a connection exists with this CastingPlayer, or triggers a new
 *     commissioning session request. If the CastingApp does not have the nodeId and fabricIndex
 *     of this CastingPlayer cached on disk, this will execute the User Directed Commissioning
 *     (UDC) process by sending an IdentificationDeclaration message to the Commissioner. This method
 *     will run verifyOrEstablishConnection() with a default timeout of
 *     MCCastingPlayer.kMinCommissioningWindowTimeoutSec.
 * @param identificationDeclarationOptions (Optional) Parameters in the IdentificationDeclaration
 *     message sent by the Commissionee to the Commissioner. These parameters specify the
 *     information relating to the requested commissioning session.
 *     <p>For example: To invoke the CastingPlayer/Commissioner-Generated passcode commissioning
 *     flow, the client would call this API with IdentificationDeclarationOptions containing
 *     CommissionerPasscode set to true. See IdentificationDeclarationOptions.java for a complete
 *     list of optional parameters.
 *     <p>Furthermore, attributes (such as VendorId) describe the TargetApp/Endpoint that the client
 *     wants to interact with after commissioning. If this value is passed in,
 *     verifyOrEstablishConnection() will force UDC, in case the desired TargetApp is not found in
 *     the on-device cached information/CastingStore.
 * @return nil if request submitted successfully, otherwise a NSError object corresponding to the error.
 * @see verifyOrEstablishConnectionWithCallbacks:timeout:identificationDeclarationOptions:
 */
- (NSError * _Nullable)verifyOrEstablishConnectionWithCallbacks:(MCConnectionCallbacks * _Nonnull)connectionCallbacks
                               identificationDeclarationOptions:(MCIdentificationDeclarationOptions * _Nullable)identificationDeclarationOptions;

/**
 * @brief Verifies that a connection exists with this CastingPlayer, or triggers a new
 *     commissioning session request. If the CastingApp does not have the nodeId and fabricIndex
 *     of this CastingPlayer cached on disk, this will execute the User Directed Commissioning
 *     (UDC) process by sending an IdentificationDeclaration message to the Commissioner. This method
 *     will run verifyOrEstablishConnection() with a default timeout of
 *     MCCastingPlayer.kMinCommissioningWindowTimeoutSec and MCIdentificationDeclarationOptions
 *     initailized with the defualt values.
 * @return nil if request submitted successfully, otherwise a NSError object corresponding to the error.
 * @see verifyOrEstablishConnectionWithCallbacks:timeout:identificationDeclarationOptions:
 */
- (NSError * _Nullable)verifyOrEstablishConnectionWithCallbacks:(MCConnectionCallbacks * _Nonnull)connectionCallbacks;

/**
 * @brief This is a continuation of the CastingPlayer/Commissioner-Generated passcode
 *     commissioning flow started via the verifyOrEstablishConnection() API above. It continues
 *     the UDC process by sending a second IdentificationDeclaration message to Commissioner
 *     containing CommissionerPasscode and CommissionerPasscodeReady set to true. At this point it
 *     is assumed that the following have occurred:
 *     <p>1. Client (Commissionee) has sent the first IdentificationDeclaration message, via
 *     verifyOrEstablishConnection(), to the Commissioner containing CommissionerPasscode set to
 *     true.
 *     <p>2. Commissioner generated and displayed a passcode.
 *     <p>3. The Commissioner replied with a CommissionerDecelration message with
 *     PasscodeDialogDisplayed and CommissionerPasscode set to true.
 *     <p>4. Client has handled the Commissioner's CommissionerDecelration message.
 *     <p>5. Client prompted user to input Passcode from Commissioner.
 *     <p>6. Client has updated the CastingApp's MCCommissionableDataProvider with the user entered
 *     passcode via the following function call:
 *     MCDataSource.update(MCCommissionableData). This updates the
 *     commissioning session's PAKE verifier with the user entered passcode.
 *     <p>Note: The same connectionCallbacks and commissioningWindowTimeoutSec parameters passed
 *     into verifyOrEstablishConnection() will be used.
 * @return nil if request submitted successfully, otherwise a NSError object corresponding to the error.
 */
- (NSError * _Nullable)continueConnecting;

/**
 * @brief This cancels the CastingPlayer/Commissioner-Generated passcode commissioning flow
 *     started via the VerifyOrEstablishConnection() API above. It constructs and sends an
 *     IdentificationDeclaration message to the CastingPlayer/Commissioner containing
 *     CancelPasscode set to true. It is used to indicate that the user, and thus the
 *     Client/Commissionee, have cancelled the commissioning process. This indicates that the
 *     CastingPlayer/Commissioner can dismiss any dialogs corresponding to commissioning, such as
 *     a Passcode input dialog or a Passcode display dialog.
 *     <p>Note: stopConnecting() does not call the connectionCompleteCallback() callback passed to the
 *     VerifyOrEstablishConnection() API above since no connection is established.
 * @return nil if request submitted successfully, otherwise a NSError object corresponding to the error.
 */
- (NSError * _Nullable)stopConnecting;

/**
 * @brief Sets the internal connection state of this MCCastingPlayer to "disconnected"
 */
- (void)disconnect;

/**
 * @brief Get the CastingPlayer's connection state
 * @param state The current connection state that will be return.
 * @return nil if request submitted successfully, otherwise a NSError object corresponding to the error.
 */
- (NSError * _Nullable)getConnectionState:(MCCastingPlayerConnectionState * _Nonnull)state;

- (NSString * _Nonnull)identifier;
- (NSString * _Nonnull)deviceName;
- (uint16_t)vendorId;
- (uint16_t)productId;
- (uint32_t)deviceType;
- (bool)supportsCommissionerGeneratedPasscode;
- (NSArray * _Nonnull)ipAddresses;
- (NSString * _Nonnull)hostName;
- (NSString * _Nonnull)instanceName;

/**
 * @brief Returns the NSArray of MCEndpoints associated with this MCCastingPlayer
 */
- (NSArray<MCEndpoint *> * _Nonnull)endpoints;
- (void)logAllEndpoints;

- (nonnull instancetype)init UNAVAILABLE_ATTRIBUTE;
+ (nonnull instancetype)new UNAVAILABLE_ATTRIBUTE;

@end

#endif /* MCCastingPlayer_h */
