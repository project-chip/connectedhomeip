/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

NS_ASSUME_NONNULL_BEGIN

typedef void (^MTRQueryImageCompletionHandler)(
    MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data, NSError * _Nullable error);

typedef void (^MTRApplyUpdateRequestCompletionHandler)(
    MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data, NSError * _Nullable error);

typedef void (^MTRBDXQueryCompletionHandler)(NSData * _Nullable data, BOOL isEOF);

/**
 * The protocol definition for the MTROTAProviderDelegate
 *
 * All delegate methods will be called on the supplied Delegate Queue.
 */
@protocol MTROTAProviderDelegate <NSObject>
@required
/**
 * Notify the delegate when the query image command is received from some node.
 * The controller identifies the fabric the node is on, and the nodeID
 * identifies the node within that fabric.
 *
 * If completionHandler is passed a non-nil error, that will be converted into
 * an error response to the client.  Otherwise it must have a non-nil data,
 * which will be returned to the client.
 */
- (void)handleQueryImageForNodeID:(NSNumber *)nodeID
                       controller:(MTRDeviceController *)controller
                           params:(MTROtaSoftwareUpdateProviderClusterQueryImageParams *)params
                completionHandler:(MTRQueryImageCompletionHandler)completionHandler;

/**
 * Notify the delegate when the apply update request command is received from
 * some node.  The controller identifies the fabric the node is on, and the
 * nodeID identifies the node within that fabric.
 *
 * If completionHandler is passed a non-nil error, that will be converted into
 * an error response to the client.  Otherwise it must have a non-nil data,
 * which will be returned to the client.
 */
- (void)handleApplyUpdateRequestForNodeID:(NSNumber *)nodeID
                               controller:(MTRDeviceController *)controller
                                   params:(MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams *)params
                        completionHandler:(MTRApplyUpdateRequestCompletionHandler)completionHandler;

/**
 * Notify the delegate when the notify update applied command is received from
 * some node.  The controller identifies the fabric the node is on, and the
 * nodeID identifies the node within that fabric.
 *
 * If completionHandler is passed a non-nil error, that will be converted into
 * an error response to the client.  Otherwise a success response will be sent.
 */
- (void)handleNotifyUpdateAppliedForNodeID:(NSNumber *)nodeID
                                controller:(MTRDeviceController *)controller
                                    params:(MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams *)params
                         completionHandler:(StatusCompletion)completionHandler;

/**
 * Notify the delegate when a BDX Session starts for some node.  The controller
 * identifies the fabric the node is on, and the nodeID identifies the node
 * within that fabric.
 */
- (void)handleBDXTransferSessionBeginForNodeID:(NSNumber *)nodeID
                                    controller:(MTRDeviceController *)controller
                                fileDesignator:(NSString *)fileDesignator
                                        offset:(NSNumber *)offset
                             completionHandler:(StatusCompletion)completionHandler;

/**
 * Notify the delegate when a BDX Session ends for some node.  The controller
 * identifies the fabric the node is on, and the nodeID identifies the node
 * within that fabric.
 */
- (void)handleBDXTransferSessionEndForNodeID:(NSNumber *)nodeID
                                  controller:(MTRDeviceController *)controller
                                       error:(NSError * _Nullable)error;

/**
 * Notify the delegate when a BDX Query message has been received for some node.
 * The controller identifies the fabric the node is on, and the nodeID
 * identifies the node within that fabric.
 */
- (void)handleBDXQueryForNodeID:(NSNumber *)nodeID
                     controller:(MTRDeviceController *)controller
                      blockSize:(NSNumber *)blockSize
                     blockIndex:(NSNumber *)blockIndex
                    bytesToSkip:(NSNumber *)bytesToSkip
              completionHandler:(MTRBDXQueryCompletionHandler)completionHandler;
@end

NS_ASSUME_NONNULL_END
