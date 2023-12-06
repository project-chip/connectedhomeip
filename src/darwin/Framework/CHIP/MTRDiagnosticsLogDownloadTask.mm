//
/**
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

#import "MTRDiagnosticsLogDownloadTask.h"
#import "MTRCluster.h"
#import "MTRClusterConstants.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceController.h"
#import "MTRDeviceController_Internal.h"
#import "MTRDevice_Internal.h"
#import "MTRDiagnosticsLogTransferHandler.h"
#import "MTRError.h"
#import <Matter/MTRClusters.h>
#import <Matter/MTRDefines.h>

#import "zap-generated/MTRCommandPayloads_Internal.h"

#import <os/lock.h>

#include <controller/CHIPDeviceControllerFactory.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;

// Spec mandated max file designator length
const uint8_t kMaxFileDesignatorLen = 32;

@interface MTRDiagnosticsLogDownloadTask ()
@property (nonatomic, readonly) os_unfair_lock lock;
@property (nonatomic) dispatch_source_t timerSource;
@property (nonatomic) MTRDiagnosticsLogTransferHandler * diagnosticLogsTransferHandler;
@property (nonatomic, readonly, weak) MTRDevice * device;
@property (strong, nonatomic) dispatch_queue_t queue;

@end

@implementation MTRDiagnosticsLogDownloadTask
- (instancetype)initWithDevice:(MTRDevice *)device
{
    if (self = [super init]) {
        _lock = OS_UNFAIR_LOCK_INIT;
        _device = device;
        _queue = dispatch_queue_create("org.csa-iot.matter.framework.device.workqueue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    }
    return self;
}

- (const char *)_toLogTypeAbbreviatedString:(MTRDiagnosticLogType)type
{
    switch (type) {
    case MTRDiagnosticLogTypeEndUserSupport:
        return "EndUser";
    case MTRDiagnosticLogTypeNetworkDiagnostics:
        return "NWDiag";
    case MTRDiagnosticLogTypeCrash:
        return "Crash";
    default:
        return "";
    }
}

- (const char *)_toLogTypeString:(MTRDiagnosticLogType)type
{
    switch (type) {
    case MTRDiagnosticLogTypeEndUserSupport:
        return "EndUserSupport";
    case MTRDiagnosticLogTypeNetworkDiagnostics:
        return "NetworkDiagnostics";
    case MTRDiagnosticLogTypeCrash:
        return "Crash";
    default:
        return "";
    }
}

- (CHIP_ERROR)_generateFileDesignatorForLogType:(MTRDiagnosticLogType)type
                                 fileDesignator:(MutableCharSpan)fileDesignator
{
    uint8_t nodeIdBytes[sizeof(NodeId)];
    Encoding::BigEndian::Put64(nodeIdBytes, _device.nodeID.unsignedLongLongValue);

    char nodeIdHex[sizeof(NodeId) * 2];
    ReturnErrorOnFailure(Encoding::BytesToUppercaseHexBuffer(nodeIdBytes, sizeof(nodeIdBytes), nodeIdHex, sizeof(nodeIdHex)));

    const char kPrefix[] = "bdx://";
    const char * fileNameAbbreviated = [self _toLogTypeAbbreviatedString:type];

    // Reduce the buffer writer size by one to reserve the last byte for the null-terminator
    Encoding::BufferWriter writer(Uint8::from_char(fileDesignator.data()), fileDesignator.size() - 1);
    writer.Put(kPrefix, strlen(kPrefix));
    writer.Put(nodeIdHex, sizeof(nodeIdHex));
    writer.Put("/");
    writer.Put(fileNameAbbreviated, strlen(fileNameAbbreviated));

    VerifyOrReturnError(writer.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);
    fileDesignator.reduce_size(writer.WritePos());
    return CHIP_NO_ERROR;
}

- (void)_startTimerForDownload:(NSTimeInterval)timeout
{
    // TODO: Fix #30538 Fix the timeout code when downloadLogOfType API is called with a timeout value.
    self->_timerSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, DISPATCH_TIMER_STRICT, self.queue);
    VerifyOrDie(self->_timerSource != nullptr);

    dispatch_source_set_timer(
        self->_timerSource, dispatch_walltime(nullptr, static_cast<int64_t>(timeout * NSEC_PER_MSEC)), DISPATCH_TIME_FOREVER, 2 * NSEC_PER_MSEC);

    dispatch_source_set_event_handler(self->_timerSource, ^{
        dispatch_async(self.queue, ^{
            os_unfair_lock_lock(&self->_lock);
            if (self->_diagnosticLogsTransferHandler != nil) {
                self->_diagnosticLogsTransferHandler->AbortTransfer(chip::bdx::StatusCode::kUnknown);
            }
            os_unfair_lock_unlock(&self->_lock);
        });
        dispatch_source_cancel(self->_timerSource);
    });
    dispatch_resume(self->_timerSource);
}

- (NSURL * _Nullable)_temporaryFileURLForDownload:(MTRDiagnosticLogType)type
                                            queue:(dispatch_queue_t)queue
                                       completion:(void (^)(NSURL * _Nullable logResult, NSError * error))completion
{
    NSDateFormatter * dateFormatter = [[NSDateFormatter alloc] init];
    dateFormatter.dateFormat = @"yyyy-MM-dd_HH:mm:ss.SSSZZZ";

    NSString * timeString = [dateFormatter stringFromDate:NSDate.now];

    NSString * fileName = [NSString stringWithFormat:@"%s_%0llx_%s", timeString.UTF8String, _device.nodeID.unsignedLongLongValue, [self _toLogTypeString:type]];

    NSURL * filePath = [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:fileName] isDirectory:YES];
    NSError * error = nil;

    NSURL * downloadFileURL = [[NSFileManager defaultManager] URLForDirectory:NSItemReplacementDirectory
                                                                     inDomain:NSUserDomainMask
                                                            appropriateForURL:filePath
                                                                       create:YES
                                                                        error:&error];
    if (downloadFileURL == nil || error != nil) {
        return nil;
    }

    if ([[NSFileManager defaultManager] createFileAtPath:[filePath path] contents:nil attributes:nil]) {
        return filePath;
    }
    return nil;
}

using namespace chip::app::Clusters::DiagnosticLogs;
- (bool)_isErrorResponse:(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable)response
{
    if (response == nil || response.status == nil) {
        return true;
    }
    StatusEnum statusValue = static_cast<StatusEnum>(response.status.intValue);
    return ((statusValue != StatusEnum::kNoLogs && statusValue != StatusEnum::kExhausted) || response.logContent.length == 0);
}

- (void)_invokeCompletion:(void (^)(NSURL * _Nullable logResult, NSError * error))completion
                 filepath:(NSURL * _Nullable)filepath
                    queue:(dispatch_queue_t)queue
                    error:(NSError * _Nullable)error
{
    dispatch_async(queue, ^{
        completion(filepath, error);
    });

    dispatch_async(self.queue, ^{
        if (self->_diagnosticLogsTransferHandler != nil) {
            delete (self->_diagnosticLogsTransferHandler);
            self->_diagnosticLogsTransferHandler = nil;
        }
    });
}

- (void)_invokeCompletionWithError:(void (^)(NSURL * _Nullable logResult, NSError * error))completion
                             queue:(dispatch_queue_t)queue
                             error:(NSError * _Nullable)error
{
    [self _invokeCompletion:completion filepath:nil queue:queue error:error];
}

- (void)_handleResponse:(NSError *)error
               filepath:(NSURL * _Nullable)filepath
                  queue:(dispatch_queue_t)queue
             completion:(void (^)(NSURL * _Nullable logResult, NSError * error))completion
{
    if (self->_timerSource) {
        dispatch_source_cancel(self->_timerSource);
    }

    if (error == nil && filepath != nil) {
        [self _invokeCompletion:completion filepath:filepath queue:queue error:nil];
    } else {
        [self _invokeCompletionWithError:completion queue:queue error:error];
    }
    [self->_device removeDiagnosticsLogDownloadTask];
}

- (void)_downloadLogOfType:(MTRDiagnosticLogType)type
                   timeout:(NSTimeInterval)timeout
                     queue:(dispatch_queue_t)queue
                completion:(void (^)(NSURL * _Nullable logResult, NSError * error))completion
{
    if (type != MTRDiagnosticLogTypeEndUserSupport && type != MTRDiagnosticLogTypeNetworkDiagnostics && type != MTRDiagnosticLogTypeCrash) {
        [self _invokeCompletionWithError:completion queue:queue error:[NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidArgument userInfo:nil]];
        return;
    }

    NSURL * filePath = [self _temporaryFileURLForDownload:type queue:queue completion:completion];
    if (filePath == nil) {
        [self _invokeCompletionWithError:completion queue:queue error:[NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil]];
        return;
    }

    os_unfair_lock_lock(&self->_lock);
    self->_diagnosticLogsTransferHandler = new MTRDiagnosticsLogTransferHandler(filePath, ^(bool result) {
        if (result == YES) {
            [self _handleResponse:nil filepath:filePath queue:queue completion:completion];
        } else {
            [self _handleResponse:[NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil] filepath:nil queue:queue completion:completion];
        }
    });
    os_unfair_lock_unlock(&self->_lock);

    // Get the device commissionee and get the exchange manager to register for unsolicited message handler for BDX messages
    [_device.deviceController asyncGetCommissionerOnMatterQueue:^(Controller::DeviceCommissioner * commissioner) {
        os_unfair_lock_lock(&self->_lock);
        commissioner->ExchangeMgr()->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, self->_diagnosticLogsTransferHandler);
        os_unfair_lock_unlock(&self->_lock);

        dispatch_async(self.queue, ^{
            // Start a timer if a timeout is provided
            if (timeout > 0) {
                [self _startTimerForDownload:timeout];
            }

            char fileDesignatorBuffer[kMaxFileDesignatorLen];
            MutableCharSpan fileDesignator = MutableCharSpan(fileDesignatorBuffer);
            CHIP_ERROR err = [self _generateFileDesignatorForLogType:type fileDesignator:fileDesignator];

            if (err != CHIP_NO_ERROR) {
                [self _handleResponse:[NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil] filepath:nil queue:queue completion:completion];
                return;
            }

            MTRDiagnosticLogsClusterRetrieveLogsRequestParams * requestParams = [[MTRDiagnosticLogsClusterRetrieveLogsRequestParams alloc] init];
            requestParams.intent = @(type);
            requestParams.requestedProtocol = @(chip::to_underlying(chip::app::Clusters::DiagnosticLogs::TransferProtocolEnum::kBdx));
            requestParams.transferFileDesignator = [[NSString alloc] initWithCString:fileDesignator.data() encoding:NSUTF8StringEncoding];

            MTRClusterDiagnosticLogs * cluster = [[MTRClusterDiagnosticLogs alloc] initWithDevice:self->_device endpointID:@(0) queue:self.queue];
            [cluster retrieveLogsRequestWithParams:requestParams expectedValues:nil expectedValueInterval:nil
                                        completion:^(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable response, NSError * _Nullable error) {
                                            os_unfair_lock_lock(&self->_lock);
                                            // If we are in a BDX session and there is no error, do nothing. Completion will be called when BDX succeeds or fails.
                                            if (self->_diagnosticLogsTransferHandler != nil && error == nil) {
                                                return;
                                            }
                                            os_unfair_lock_unlock(&self->_lock);

                                            if ([self _isErrorResponse:response]) {
                                                [self _handleResponse:error filepath:nil queue:queue completion:completion];
                                                return;
                                            }

                                            // If the response has a log content, copy it into the temporary location and send the URL.
                                            if (response != nil && response.logContent != nil && response.logContent.length > 0) {
                                                if ([response.logContent writeToURL:filePath atomically:YES]) {
                                                    [self _handleResponse:nil filepath:filePath queue:queue completion:completion];
                                                    return;
                                                }
                                            }
                                        }];
        });
    }
        errorHandler:^(NSError * error) {
            [self _handleResponse:error filepath:nil queue:queue completion:completion];
        }];
}

- (void)downloadLogOfType:(MTRDiagnosticLogType)type
                  timeout:(NSTimeInterval)timeout
                    queue:(dispatch_queue_t)queue
               completion:(void (^)(NSURL * _Nullable logResult, NSError * error))completion
{
    [self _downloadLogOfType:type timeout:timeout queue:queue completion:completion];
}

@end
