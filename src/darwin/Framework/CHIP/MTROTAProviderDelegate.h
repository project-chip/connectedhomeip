/**
 *
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
#import <Matter/MTRCluster.h>
#import <Matter/MTRCommandPayloadsObjc.h>
#import <Matter/MTRDefines.h>

@class MTRDeviceController;
@class MTRMetrics;

NS_ASSUME_NONNULL_BEGIN

/**
 * The protocol definition for the MTROTAProviderDelegate
 *
 * All delegate methods will be called on the supplied Delegate Queue.
 *
 * While the selectors on this protocol are marked @optional, in practice an
 * implementation must provide an implementation for one of each pair of
 * selectors (e.g. one of the two handleQueryImageForNodeID selectors must be
 * implemented).  The selector ending in "completion:" will be used if present;
 * otherwise the one ending in "completionHandler:" will be used.
 */
@protocol MTROTAProviderDelegate <NSObject>
@optional
/**
 * Notify the delegate when the query image command is received from some node.
 * The controller identifies the fabric the node is on, and the nodeID
 * identifies the node within that fabric.
 *
 * If completion is passed a non-nil error, that will be converted into
 * an error response to the client.  Otherwise it must have a non-nil data,
 * which will be returned to the client.
 *
 * When providing an OTA image, the imageURI in the
 * MTROTASoftwareUpdateProviderClusterQueryImageResponseParams must be set to
 * the file designator of the image.  The completion will create the right BDX
 * URI from that file designator, as needed.  The file designator returned here
 * via imageURI will be the same as the file designator passed to
 * handleBDXTransferSessionBeginForNodeID:controller:fileDesignator:offset:completion:.
 */
- (void)handleQueryImageForNodeID:(NSNumber *)nodeID
                       controller:(MTRDeviceController *)controller
                           params:(MTROTASoftwareUpdateProviderClusterQueryImageParams *)params
                       completion:(void (^)(MTROTASoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                                      NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)handleQueryImageForNodeID:(NSNumber *)nodeID
                       controller:(MTRDeviceController *)controller
                           params:(MTROtaSoftwareUpdateProviderClusterQueryImageParams *)params
                completionHandler:(void (^)(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                                      NSError * _Nullable error))completionHandler
    MTR_DEPRECATED_WITH_REPLACEMENT("handleQueryImageForNodeID:controller:params:completion:", ios(16.1, 16.4), macos(13.0, 13.3),
        watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Notify the delegate when the apply update request command is received from
 * some node.  The controller identifies the fabric the node is on, and the
 * nodeID identifies the node within that fabric.
 *
 * If completion is passed a non-nil error, that will be converted into
 * an error response to the client.  Otherwise it must have a non-nil data,
 * which will be returned to the client.
 */
- (void)handleApplyUpdateRequestForNodeID:(NSNumber *)nodeID
                               controller:(MTRDeviceController *)controller
                                   params:(MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
                               completion:(void (^)(MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                              NSError * _Nullable error))completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)handleApplyUpdateRequestForNodeID:(NSNumber *)nodeID
                               controller:(MTRDeviceController *)controller
                                   params:(MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
                        completionHandler:(void (^)(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                              NSError * _Nullable error))completionHandler
    MTR_DEPRECATED_WITH_REPLACEMENT("handleApplyUpdateRequestForNodeID:controller:params:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Notify the delegate when the notify update applied command is received from
 * some node.  The controller identifies the fabric the node is on, and the
 * nodeID identifies the node within that fabric.
 *
 * If completion is passed a non-nil error, that will be converted into
 * an error response to the client.  Otherwise a success response will be sent.
 */
- (void)handleNotifyUpdateAppliedForNodeID:(NSNumber *)nodeID
                                controller:(MTRDeviceController *)controller
                                    params:(MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                                completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)handleNotifyUpdateAppliedForNodeID:(NSNumber *)nodeID
                                controller:(MTRDeviceController *)controller
                                    params:(MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                         completionHandler:(StatusCompletion)completionHandler
    MTR_DEPRECATED_WITH_REPLACEMENT("handleNotifyUpdateAppliedForNodeID:controller:params:completion:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Notify the delegate when a BDX Session starts for some node.  The controller
 * identifies the fabric the node is on, and the nodeID identifies the node
 * within that fabric.
 *
 * The offset is the start offset (STARTOFS) from the node's BDX ReceiveInit
 * message, passed through unchanged: the byte offset in the OTA image at which the
 * transfer starts.
 *
 * To reject the transfer with a specific BDX status, pass completion an error
 * in MTRBDXErrorDomain:
 *
 * - MTRBDXErrorCodeStartOffsetNotSupported if the delegate cannot provide data
 *   starting at offset.
 * - MTRBDXErrorCodeFileDesignatorUnknown if the delegate has no OTA image for
 *   fileDesignator.
 * - MTRBDXErrorCodeResponderBusy if the delegate cannot handle the transfer
 *   right now.  The Matter specification says the requestor SHOULD then wait
 *   at least 60 seconds before it tries again.
 *
 * Any other error also rejects the transfer, with an unspecified BDX status.
 *
 * When the transfer is rejected with an error in MTRBDXErrorDomain, that same
 * NSError is the error passed to
 * handleBDXTransferSessionEndForNodeID:controller:metrics:error:.
 */
- (void)handleBDXTransferSessionBeginForNodeID:(NSNumber *)nodeID
                                    controller:(MTRDeviceController *)controller
                                fileDesignator:(NSString *)fileDesignator
                                        offset:(NSNumber *)offset
                                    completion:(MTRStatusCompletion)completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)handleBDXTransferSessionBeginForNodeID:(NSNumber *)nodeID
                                    controller:(MTRDeviceController *)controller
                                fileDesignator:(NSString *)fileDesignator
                                        offset:(NSNumber *)offset
                             completionHandler:(StatusCompletion)completionHandler
    MTR_DEPRECATED_WITH_REPLACEMENT("handleBDXTransferSessionBeginForNodeID:controller:fileDesignator:offset:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

/**
 * Notify the delegate when a BDX Session ends for some node.  The controller
 * identifies the fabric the node is on, and the nodeID identifies the node
 * within that fabric.
 */
- (void)handleBDXTransferSessionEndForNodeID:(NSNumber *)nodeID
                                  controller:(MTRDeviceController *)controller
                                     metrics:(MTRMetrics *)metrics
                                       error:(NSError * _Nullable)error
    MTR_AVAILABLE(ios(26.1), macos(26.1), watchos(26.1), tvos(26.1));

- (void)handleBDXTransferSessionEndForNodeID:(NSNumber *)nodeID
                                  controller:(MTRDeviceController *)controller
                                       error:(NSError * _Nullable)error
    MTR_DEPRECATED_WITH_REPLACEMENT("handleBDXTransferSessionEndForNodeID:controller:metrics:error:", ios(16.1, 26.1),
        macos(13.0, 26.1), watchos(9.1, 26.1), tvos(16.1, 26.1));

/**
 * Notify the delegate when a BDX Query message has been received for some node.
 * The controller identifies the fabric the node is on, and the nodeID
 * identifies the node within that fabric.
 *
 * blockSize, blockIndex and bytesToSkip are the BDX values for this query,
 * passed through unchanged. blockSize is the maximum block size negotiated for
 * the transfer. blockIndex is the block counter of the query: 0 for the first
 * query of every transfer incrementing by 1 for each query thereafter. The
 * delegate is called once for each blockIndex. bytesToSkip is the BytesToSkip
 * value of a BlockQueryWithSkip message, or 0 for a BlockQuery. bytesToSkip
 * counts only the skip requested by this query, and need not be a multiple of
 * blockSize.
 *
 * A skip moves the position in the file forward for the rest of the transfer,
 * not just for this block. The data for this block starts at the transfer's
 * offset, plus the length of all the data already sent in the transfer, plus
 * every bytesToSkip received in the transfer, including this one.
 *
 * The data passed to the completion must be of size blockSize, unless it's the
 * last block of data.  In that case, it may be smaller than blockSize, and
 * isEOF must be set to YES. If the start of the data being requested is at or
 * past the end of the OTA image, which a skip can cause, the completion must be
 * passed empty data and isEOF set to YES.
 */
- (void)handleBDXQueryForNodeID:(NSNumber *)nodeID
                     controller:(MTRDeviceController *)controller
                      blockSize:(NSNumber *)blockSize
                     blockIndex:(NSNumber *)blockIndex
                    bytesToSkip:(NSNumber *)bytesToSkip
                     completion:(void (^)(NSData * _Nullable data, BOOL isEOF))completion
    MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)handleBDXQueryForNodeID:(NSNumber *)nodeID
                     controller:(MTRDeviceController *)controller
                      blockSize:(NSNumber *)blockSize
                     blockIndex:(NSNumber *)blockIndex
                    bytesToSkip:(NSNumber *)bytesToSkip
              completionHandler:(void (^)(NSData * _Nullable data, BOOL isEOF))completionHandler
    MTR_DEPRECATED_WITH_REPLACEMENT("handleBDXQueryForNodeID:controller:blockSize:blockIndex:bytesToSkip:completion:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

NS_ASSUME_NONNULL_END
