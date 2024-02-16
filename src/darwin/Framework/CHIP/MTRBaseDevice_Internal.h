/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#import "MTRBaseDevice.h"
#import <Foundation/Foundation.h>

#include <app/AttributePathParams.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/ConcreteEventPath.h>
#include <app/DeviceProxy.h>
#include <app/EventHeader.h>
#include <app/EventLoggingTypes.h>
#include <app/EventPathParams.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/TLVTags.h>
#include <system/SystemPacketBuffer.h>

@class MTRDeviceController;

// An AttestationResponse command needs to have an attestationChallenge
// to make sense of the results.  Encode that with a profile-specific tag under
// the Apple vendor id.  Let's select profile 0xFFFF just because, and use 0xFF
// for the actual tag number, so that if someone accidentally casts it to a
// uint8 (aka context tag) that will not collide with anything interesting.
inline constexpr chip::TLV::Tag kAttestationChallengeTag = chip::TLV::ProfileTag(chip::VendorId::Apple, 0xFFFF, 0xFF);

// We have no way to extract the tag value as a single thing, so just do it
// manually.
inline constexpr unsigned kProfileIdShift = 32;
inline constexpr uint64_t kAttestationChallengeTagProfile = chip::TLV::ProfileIdFromTag(kAttestationChallengeTag);
inline constexpr uint64_t kAttestationChallengeTagNumber = chip::TLV::TagNumFromTag(kAttestationChallengeTag);
inline constexpr uint64_t kAttestationChallengeTagValue = (kAttestationChallengeTagProfile << kProfileIdShift) | kAttestationChallengeTagNumber;

NS_ASSUME_NONNULL_BEGIN

static inline MTRTransportType MTRMakeTransportType(chip::Transport::Type type)
{
    static_assert(MTRTransportTypeUndefined == (uint8_t) chip::Transport::Type::kUndefined, "MTRTransportType != Transport::Type");
    static_assert(MTRTransportTypeUDP == (uint8_t) chip::Transport::Type::kUdp, "MTRTransportType != Transport::Type");
    static_assert(MTRTransportTypeBLE == (uint8_t) chip::Transport::Type::kBle, "MTRTransportType != Transport::Type");
    static_assert(MTRTransportTypeTCP == (uint8_t) chip::Transport::Type::kTcp, "MTRTransportType != Transport::Type");
    return static_cast<MTRTransportType>(type);
}

@interface MTRBaseDevice ()

- (instancetype)initWithPASEDevice:(chip::DeviceProxy *)device controller:(MTRDeviceController *)controller;

/**
 * Invalidate the CASE session, so an attempt to getConnectedDevice for this
 * device id will have to create a new CASE session.  Ideally this API will go
 * away.
 */
- (void)invalidateCASESession;

/**
 * Whether this device represents a PASE session or not.
 */
@property (nonatomic, assign, readonly) BOOL isPASEDevice;

/**
 * Controller that that this MTRDevice was gotten from.
 */
@property (nonatomic, strong, readonly) MTRDeviceController * deviceController;

/**
 * Node id for this MTRDevice.  If this device represents a CASE session, this
 * is set to the node ID of the target node.  If this device represents a PASE
 * session, this is set to the device id of the PASE device.
 */
@property (nonatomic, assign, readonly) chip::NodeId nodeID;

/**
 * Initialize the device object as a CASE device with the given node id and
 * controller.  This will always succeed, even if there is no such node id on
 * the controller's fabric, but attempts to actually use the MTRBaseDevice will
 * fail (asynchronously) in that case.
 */
- (instancetype)initWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller;

/**
 * Create a report, suitable in including in the sort of data structure that
 * gets passed to MTRDeviceResponseHandler, from a given event header and
 * already-decoded event data.  The data is allowed to be nil in error cases
 * (e.g. when TLV decoding failed).
 */
+ (NSDictionary *)eventReportForHeader:(const chip::app::EventHeader &)header andData:(id _Nullable)data;

/**
 * Extract a data-value for the given response command from the given response-value
 * dictionary, encode it to TLV, and return a System::PacketBufferHandle with
 * the encoded data.
 *
 * Will return a null handle and an error if the given response-value does not represent a
 * data command response or is the wrong response command, or if encoding to TLV fails.
 */
+ (chip::System::PacketBufferHandle)_responseDataForCommand:(NSDictionary<NSString *, id> *)responseValue
                                                  clusterID:(chip::ClusterId)clusterID
                                                  commandID:(chip::CommandId)commandID
                                                      error:(NSError * __autoreleasing *)error;

/**
 * Like the public invokeCommandWithEndpointID but allows passing through a
 * serverSideProcessingTimeout.
 */
- (void)_invokeCommandWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                           commandID:(NSNumber *)commandID
                       commandFields:(id)commandFields
                  timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
         serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
                               queue:(dispatch_queue_t)queue
                          completion:(MTRDeviceResponseHandler)completion;

/**
 * Like the public invokeCommandWithEndpointID but:
 *
 * 1) Allows passing through a serverSideProcessingTimeout.
 * 2) Expects one of the command payload structs as commandPayload
 * 3) On success, returns an instance of responseClass via the completion (or
 *    nil if there is no responseClass, which indicates a status-only command).
 */
- (void)_invokeKnownCommandWithEndpointID:(NSNumber *)endpointID
                                clusterID:(NSNumber *)clusterID
                                commandID:(NSNumber *)commandID
                           commandPayload:(id)commandPayload
                       timedInvokeTimeout:(NSNumber * _Nullable)timeout
              serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
                            responseClass:(Class _Nullable)responseClass
                                    queue:(dispatch_queue_t)queue
                               completion:(void (^)(id _Nullable response, NSError * _Nullable error))completion;

/**
 * Like the public subscribeToAttributesWithEndpointID but:
 *
 * 1) Takes a concrete attribute path (not nullable).
 * 2) Requires non-nil MTRSubscribeParams (since that's what MTRBaseClusters
 *    have anyway).
 * 3) For the report handler, hands out the right type of value for the given
 *    attribute path.  This means we have to know the type ofthe attribute.
 *
 * The attribute path is not represented as MTRAttributePath just because it's
 * probably less code to pass in the three numbers instead of creating an object
 * at all the (numerous) callsites.
 */
- (void)_subscribeToKnownAttributeWithEndpointID:(NSNumber *)endpointID
                                       clusterID:(NSNumber *)clusterID
                                     attributeID:(NSNumber *)attributeID
                                          params:(MTRSubscribeParams *)params
                                           queue:(dispatch_queue_t)queue
                                   reportHandler:(void (^)(id _Nullable value, NSError * _Nullable error))reportHandler
                         subscriptionEstablished:(MTRSubscriptionEstablishedHandler _Nullable)subscriptionEstablished;

/**
 * Like the public readAttributesWithEndpointID but:
 *
 * 1) Takes a concrete attribute path (not nullable).
 * 2) For the completion handler, hands out the right type of value for the given
 *    attribute path.  This means we have to know the type ofthe attribute.
 * 3) Only calls the completion with a single value.
 *
 * The attribute path is not represented as MTRAttributePath just because it's
 * probably less code to pass in the three numbers instead of creating an object
 * at all the (numerous) callsites.
 */
- (void)_readKnownAttributeWithEndpointID:(NSNumber *)endpointID
                                clusterID:(NSNumber *)clusterID
                              attributeID:(NSNumber *)attributeID
                                   params:(MTRReadParams * _Nullable)params
                                    queue:(dispatch_queue_t)queue
                               completion:(void (^)(id _Nullable value, NSError * _Nullable error))completion;

/**
 * Same as the public -readAttributePaths:eventPaths:params:queue:completion: except also include the data version in the data-value dictionary in the response dictionary, if the includeDataVersion argument is set to YES.
 */
- (void)readAttributePaths:(NSArray<MTRAttributeRequestPath *> * _Nullable)attributePaths
                eventPaths:(NSArray<MTREventRequestPath *> * _Nullable)eventPaths
                    params:(MTRReadParams * _Nullable)params
        includeDataVersion:(BOOL)includeDataVersion
                     queue:(dispatch_queue_t)queue
                completion:(MTRDeviceResponseHandler)completion;

@end

@interface MTRClusterPath ()
- (instancetype)initWithPath:(const chip::app::ConcreteClusterPath &)path;
@end

@interface MTRAttributePath ()
- (instancetype)initWithPath:(const chip::app::ConcreteDataAttributePath &)path;
@end

@interface MTREventPath ()
- (instancetype)initWithPath:(const chip::app::ConcreteEventPath &)path;
@end

@interface MTRCommandPath ()
- (instancetype)initWithPath:(const chip::app::ConcreteCommandPath &)path;
@end

@interface MTRAttributeReport ()
- (instancetype)initWithPath:(const chip::app::ConcreteDataAttributePath &)path
                       value:(id _Nullable)value
                       error:(NSError * _Nullable)error;
@end

@interface MTREventReport ()
- (instancetype)initWithPath:(const chip::app::ConcreteEventPath &)path error:(NSError *)error;
- (instancetype)initWithPath:(const chip::app::ConcreteEventPath &)path
                 eventNumber:(NSNumber *)eventNumber
                    priority:(chip::app::PriorityLevel)priority
                   timestamp:(const chip::app::Timestamp &)timestamp
                       value:(id)value;
@end

@interface MTRAttributeRequestPath ()
- (void)convertToAttributePathParams:(chip::app::AttributePathParams &)params;
@end

@interface MTREventRequestPath ()
- (void)convertToEventPathParams:(chip::app::EventPathParams &)params;
@end

// Exported utility function
// Convert TLV data into data-value dictionary as described in MTRDeviceResponseHandler
NSDictionary<NSString *, id> * _Nullable MTRDecodeDataValueDictionaryFromCHIPTLV(chip::TLV::TLVReader * data, NSNumber * _Nullable dataVersion = nil);

// Convert a data-value dictionary as described in MTRDeviceResponseHandler into
// TLV Data with an anonymous tag.  This method assumes the encoding of the
// value fits in a single UDP MTU; for lists this method might need to be used
// on each list item separately.
NSData * _Nullable MTREncodeTLVFromDataValueDictionary(NSDictionary<NSString *, id> * value, NSError * __autoreleasing * error);

NS_ASSUME_NONNULL_END
