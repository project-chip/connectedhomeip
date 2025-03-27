/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#import "MTRDiagnosticLogsDownloader.h"
#import <Matter/Matter.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/LockTracker.h>
#include <protocols/bdx/BdxTransferServerDelegate.h>
#include <protocols/bdx/DiagnosticLogs.h>

#import "MTRDeviceControllerFactory_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#import "zap-generated/MTRClusters.h"

typedef void (^AbortHandler)(NSError * error);

static NSString * const kErrorInitDiagnosticLogsDownloader = @"Init failure while initializing Diagnostic Logs bridge.";
static NSString * const kEndUserSupport = @"EndUserSupport";
static NSString * const kNetworkDiagnostics = @"NetworkDiagnostics";
static NSString * const kCrash = @"Crash";

constexpr uint8_t kDiagnosticLogsEndPoint = 0;

class DiagnosticLogsDownloaderBridge;

NS_ASSUME_NONNULL_BEGIN

@interface MTRDownload : NSObject
@property (nonatomic) NSString * fileDesignator;
@property (nonatomic) NSNumber * fabricIndex;
@property (nonatomic) NSNumber * nodeID;
@property (nonatomic) NSURL * fileURL;
@property (nonatomic) NSFileHandle * fileHandle;
@property (nonatomic) AbortHandler abortHandler;
@property (nonatomic) MTRStatusCompletion finalize;

- (instancetype)initWithType:(MTRDiagnosticLogType)type
                 fabricIndex:(NSNumber *)fabricIndex
                      nodeID:(NSNumber *)nodeID
                     timeout:(NSTimeInterval)timeout
                       queue:(dispatch_queue_t)queue
                  completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
                        done:(void (^)(MTRDownload * finishedDownload))done;

- (void)writeToFile:(NSData *)data error:(out NSError **)error;

- (BOOL)matches:(NSString *)fileDesignator
    fabricIndex:(NSNumber *)fabricIndex
         nodeID:(NSNumber *)nodeID;

- (void)checkInteractionModelResponse:(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable)response error:(NSError * _Nullable)error;

// TODO: The lifetime management for these objects is very odd.  Nothing
// _really_ prevents a BDX transfer starting after a failure: call, for example.
// We should move more of the state into a single place that will know the exact
// state of the object.
- (void)success;
- (void)failure:(NSError *)error;
- (void)cancelTimeoutTimer;
- (void)abort:(NSError *)error;
@end

@interface MTRDownloads : NSObject
@property (nonatomic, strong) NSMutableArray<MTRDownload *> * downloads;

- (MTRDownload * _Nullable)get:(NSString *)fileDesignator
                   fabricIndex:(NSNumber *)fabricIndex
                        nodeID:(NSNumber *)nodeID;

- (MTRDownload * _Nullable)add:(MTRDiagnosticLogType)type
                   fabricIndex:(NSNumber *)fabricIndex
                        nodeID:(NSNumber *)nodeID
                       timeout:(NSTimeInterval)timeout
                         queue:(dispatch_queue_t)queue
                    completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
                          done:(void (^)(MTRDownload * finishedDownload))done;

- (void)abortDownloadsForController:(MTRDeviceController_Concrete *)controller;

@end

@interface MTRDiagnosticLogsDownloader ()
@property (readonly) DiagnosticLogsDownloaderBridge * bridge;
@property (nonatomic, strong) MTRDownloads * downloads;

/**
 * Notify the delegate when a BDX Session starts for some logs.
 *
 * If completion is passed a non-nil error, that will be converted into
 * an error response to the BDX initiatior. Otherwise a success response will be sent.
 */
- (void)handleBDXTransferSessionBeginForFileDesignator:(NSString *)fileDesignator
                                           fabricIndex:(NSNumber *)fabricIndex
                                                nodeID:(NSNumber *)nodeID
                                            completion:(MTRStatusCompletion)completion
                                          abortHandler:(AbortHandler)abortHandler;

/**
 * Notify the delegate when some data is received on the BDX Session.
 *
 * If completion is passed a non-nil error, that will be converted into
 * an error response to the sender. Otherwise a success response will be sent.
 */
- (void)handleBDXTransferSessionDataForFileDesignator:(NSString *)fileDesignator
                                          fabricIndex:(NSNumber *)fabricIndex
                                               nodeID:(NSNumber *)nodeID
                                                 data:(NSData *)data
                                           completion:(MTRStatusCompletion)completion;

/**
 * Notify the delegate when a BDX Session ends for some logs.
 */
- (void)handleBDXTransferSessionEndForFileDesignator:(NSString *)fileDesignator
                                         fabricIndex:(NSNumber *)fabricIndex
                                              nodeID:(NSNumber *)nodeID
                                               error:(NSError * _Nullable)error;

@end

NS_ASSUME_NONNULL_END

class DiagnosticLogsDownloaderBridge : public chip::bdx::BDXTransferServerDelegate {
public:
    DiagnosticLogsDownloaderBridge(MTRDiagnosticLogsDownloader * delegate);
    ~DiagnosticLogsDownloaderBridge();

    /////////// BDXTransferServerDelegate Interface /////////
    CHIP_ERROR OnTransferBegin(chip::bdx::BDXTransferProxy * transfer) override;
    CHIP_ERROR OnTransferEnd(chip::bdx::BDXTransferProxy * transfer, CHIP_ERROR error) override;
    CHIP_ERROR OnTransferData(chip::bdx::BDXTransferProxy * transfer, const chip::ByteSpan & data) override;

private:
    MTRDiagnosticLogsDownloader * __weak mDelegate;
};

@implementation MTRDownload

static void OnTransferTimeout(chip::System::Layer * layer, void * context)
{
    assertChipStackLockedByCurrentThread();

    auto * download = (__bridge MTRDownload *) context;
    VerifyOrReturn(nil != download);

    auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:download.fabricIndex.unsignedCharValue];

    MTR_LOG("%@ Diagnostic log transfer timed out for %016llX-%016llX (%llu), abortHandler: %@", download,
        controller.compressedFabricID.unsignedLongLongValue, download.nodeID.unsignedLongLongValue,
        download.nodeID.unsignedLongLongValue, download.abortHandler);

    [download abort:[MTRError errorForCHIPErrorCode:CHIP_ERROR_TIMEOUT]];
}

- (instancetype)initWithType:(MTRDiagnosticLogType)type
                 fabricIndex:(NSNumber *)fabricIndex
                      nodeID:(NSNumber *)nodeID
                     timeout:(NSTimeInterval)timeout
                       queue:(dispatch_queue_t)queue
                  completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
                        done:(void (^)(MTRDownload * finishedDownload))done;
{
    assertChipStackLockedByCurrentThread();

    self = [super init];
    if (self) {
        auto * fileDesignator = [self _toFileDesignatorString:type nodeID:nodeID];
        auto * fileURL = [self _toFileURL:type nodeID:nodeID];

        __weak typeof(self) weakSelf = self;
        auto bdxTransferDone = ^(NSError * bdxError) {
            dispatch_async(queue, ^{
                MTRDownload * strongSelf = weakSelf;
                if (strongSelf) {
                    // If a fileHandle exists, it means that the BDX session has been initiated and a file has
                    // been created to host the data of the session. So even if there is an error there may be some
                    // data in the logs that the caller may find useful. For this reason, fileURL is passed in even
                    // when there is an error but fileHandle is not nil.
                    completion(strongSelf->_fileHandle ? fileURL : nil, bdxError);

                    done(strongSelf);
                }
            });
        };

        _fileDesignator = fileDesignator;
        _fabricIndex = fabricIndex;
        _nodeID = nodeID;
        _fileURL = fileURL;
        _fileHandle = nil;
        _finalize = bdxTransferDone;

        if (timeout <= 0) {
            timeout = 0;
        } else if (timeout > UINT16_MAX) {
            MTR_LOG("Warning: timeout is too large. It will be truncated to UINT16_MAX.");
            timeout = UINT16_MAX;
        }

        if (timeout > 0) {
            CHIP_ERROR timerStartErr = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(static_cast<uint16_t>(timeout)),
                OnTransferTimeout, (__bridge void *) self);
            if (timerStartErr != CHIP_NO_ERROR) {
                MTR_LOG_ERROR("Failed to start timer for diagnostic log download timeout");
                return nil;
            }
        }
    }
    return self;
}

- (void)checkInteractionModelResponse:(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable)response error:(NSError * _Nullable)error
{
    VerifyOrReturn(nil == error, [self failure:error]);

    auto status = response.status;

    VerifyOrReturn(![status isEqual:@(MTRDiagnosticLogsStatusBusy)], [self failure:[MTRError errorForCHIPErrorCode:CHIP_ERROR_BUSY]]);
    VerifyOrReturn(![status isEqual:@(MTRDiagnosticLogsStatusDenied)], [self failure:[MTRError errorForCHIPErrorCode:CHIP_ERROR_ACCESS_DENIED]]);
    VerifyOrReturn(![status isEqual:@(MTRDiagnosticLogsStatusNoLogs)], [self failure:[MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_FOUND]]);

    // If the whole log content fits into the response LogContent field or if there is no log, forward it to the caller
    // and stop here.
    if ([status isEqual:@(MTRDiagnosticLogsStatusExhausted)]) {
        NSError * writeError = nil;
        [self writeToFile:response.logContent error:&writeError];
        VerifyOrReturn(nil == writeError, [self failure:writeError]);

        [self success];
        return;
    };

    // The file is going to be transferred over BDX. Everything past this point will be handled in bdxTransferDone.
}

- (void)createFile:(NSError **)error
{
    VerifyOrReturn(nil == _fileHandle);

    auto * fileManager = [NSFileManager defaultManager];
    [fileManager URLForDirectory:NSItemReplacementDirectory
                        inDomain:NSUserDomainMask
               appropriateForURL:_fileURL
                          create:YES
                           error:error];
    VerifyOrReturn(nil == *error);

    BOOL success = [fileManager createFileAtPath:[_fileURL path] contents:nil attributes:nil];
    VerifyOrReturn(success, *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTERNAL]);

    auto * fileHandle = [NSFileHandle fileHandleForWritingToURL:_fileURL error:error];
    VerifyOrReturn(nil == *error);

    _fileHandle = fileHandle;
}

- (void)deleteFile
{
    VerifyOrReturn(nil != _fileHandle);

    NSError * error = nil;
    [[NSFileManager defaultManager] removeItemAtPath:[_fileURL path] error:&error];
    if (nil != error) {
        // There is an error but there is really not much we can do at that point besides logging it.
        MTR_LOG_ERROR("Error trying to delete the log file: %@. Error: %@", _fileURL, error);
    }
}

- (void)writeToFile:(NSData *)data error:(out NSError **)error
{
    [self createFile:error];
    [_fileHandle seekToEndOfFile];
    [_fileHandle writeData:data error:error];
}

- (BOOL)matches:(NSString *)fileDesignator
    fabricIndex:(NSNumber *)fabricIndex
         nodeID:(NSNumber *)nodeID
{
    return [_fileDesignator isEqualToString:fileDesignator] && [_fabricIndex isEqualToNumber:fabricIndex] && [_nodeID isEqualToNumber:nodeID];
}

- (void)failure:(NSError *)error
{
    MTR_LOG("%@ Diagnostic log transfer failure: %@", self, error);
    _finalize(error);
}

- (void)success
{
    _finalize(nil);
}

- (void)cancelTimeoutTimer
{
    assertChipStackLockedByCurrentThread();
    chip::DeviceLayer::SystemLayer().CancelTimer(OnTransferTimeout, (__bridge void *) self);
}

- (void)abort:(NSError *)error
{
    assertChipStackLockedByCurrentThread();

    [self cancelTimeoutTimer];

    // If there is no abortHandler, it means that the BDX transfer has not
    // started, so we can just call failure: directly.
    //
    // If there is an abortHandler, we need to call it to abort the transfer.
    if (self.abortHandler == nil) {
        [self failure:error];
    } else {
        self.abortHandler(error);
    }
}

- (NSURL *)_toFileURL:(MTRDiagnosticLogType)type nodeID:(NSNumber *)nodeID
{
    auto * dateFormatter = [[NSDateFormatter alloc] init];
    dateFormatter.dateFormat = @"yyyy-MM-dd_HH:mm:ss.SSSZZZ";
    auto * timeString = [dateFormatter stringFromDate:NSDate.now];
    auto * nodeIDString = [self _toNodeIDString:nodeID];
    auto * typeString = [self _toTypeString:type];
    auto * filename = [NSString stringWithFormat:@"%@_%@_%@", timeString, nodeIDString, typeString];
    return [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:filename] isDirectory:YES];
}

- (NSString *)_toFileDesignatorString:(MTRDiagnosticLogType)type nodeID:(NSNumber *)nodeID
{
    auto * nodeIDString = [self _toNodeIDString:nodeID];
    auto * typeString = [self _toTypeString:type];
    auto * fileDesignator = [NSString stringWithFormat:@"bdx://%@/%@", nodeIDString, typeString];
    auto substringIndex = MIN(chip::bdx::DiagnosticLogs::kMaxFileDesignatorLen, [fileDesignator length]);
    return [fileDesignator substringToIndex:substringIndex];
}

- (NSString *)_toNodeIDString:(NSNumber *)nodeID
{
    return [NSString stringWithFormat:@"%016llX", nodeID.unsignedLongLongValue];
}

- (NSString *)_toTypeString:(MTRDiagnosticLogType)type
{
    switch (type) {
    case MTRDiagnosticLogTypeEndUserSupport:
        return kEndUserSupport;
    case MTRDiagnosticLogTypeNetworkDiagnostics:
        return kNetworkDiagnostics;
    case MTRDiagnosticLogTypeCrash:
        return kCrash;
    default:
        // This should never happen.
        chipDie();
    }
}

@end

@implementation MTRDownloads
- (instancetype)init
{
    if (self = [super init]) {
        _downloads = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)dealloc
{
    auto error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTERNAL];
    for (MTRDownload * download in _downloads) {
        [download failure:error];
    }
    _downloads = nil;
}

- (MTRDownload * _Nullable)get:(NSString *)fileDesignator fabricIndex:(NSNumber *)fabricIndex nodeID:(NSNumber *)nodeID
{
    for (MTRDownload * download in _downloads) {
        if ([download matches:fileDesignator fabricIndex:fabricIndex nodeID:nodeID]) {
            return download;
        }
    }

    return nil;
}

- (MTRDownload * _Nullable)add:(MTRDiagnosticLogType)type
                   fabricIndex:(NSNumber *)fabricIndex
                        nodeID:(NSNumber *)nodeID
                       timeout:(NSTimeInterval)timeout
                         queue:(dispatch_queue_t)queue
                    completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
                          done:(void (^)(MTRDownload * finishedDownload))done
{
    assertChipStackLockedByCurrentThread();

    auto download = [[MTRDownload alloc] initWithType:type fabricIndex:fabricIndex nodeID:nodeID timeout:timeout queue:queue completion:completion done:done];
    VerifyOrReturnValue(nil != download, nil);

    [_downloads addObject:download];
    return download;
}

- (void)abortDownloadsForController:(MTRDeviceController_Concrete *)controller
{
    assertChipStackLockedByCurrentThread();

    auto fabricIndex = @(controller.fabricIndex);
    for (MTRDownload * download in [_downloads copy]) {
        if (![download.fabricIndex isEqual:fabricIndex]) {
            continue;
        }

        [download abort:[MTRError errorForCHIPErrorCode:CHIP_ERROR_CANCELLED]];
        // Remove directly instead of waiting for the async bits to catch up,
        // since those async bits might run after controller shutdown finishes
        // and then not be able to dispatch the async task to do the removal.
        [self remove:download];
    }
}

- (void)remove:(MTRDownload *)download
{
    assertChipStackLockedByCurrentThread();

    [download cancelTimeoutTimer];
    [_downloads removeObject:download];
}
@end

@implementation MTRDiagnosticLogsDownloader
- (instancetype)init
{
    assertChipStackLockedByCurrentThread();

    if (self = [super init]) {
        _downloads = [[MTRDownloads alloc] init];
        _bridge = new DiagnosticLogsDownloaderBridge(self);
        if (_bridge == nullptr) {
            MTR_LOG_ERROR("Error: %@", kErrorInitDiagnosticLogsDownloader);
            return nil;
        }
    }
    return self;
}

- (void)dealloc
{
    if (_bridge) {
        delete _bridge;
        _bridge = nil;
    }
    _downloads = nil;
}

- (chip::bdx::BDXTransferServerDelegate *)getBridge
{
    return _bridge;
}

- (void)downloadLogFromNodeWithID:(NSNumber *)nodeID
                       controller:(MTRDeviceController_Concrete *)controller
                             type:(MTRDiagnosticLogType)type
                          timeout:(NSTimeInterval)timeout
                            queue:(dispatch_queue_t)queue
                       completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion;
{
    assertChipStackLockedByCurrentThread();

    // Fow now, we only support one download at a time per controller; abort
    // any existing ones so we can start this new one.
    [self abortDownloadsForController:controller];

    // This block is always called when a download is finished.
    auto done = ^(MTRDownload * finishedDownload) {
        [controller asyncDispatchToMatterQueue:^() {
            [self->_downloads remove:finishedDownload];
        } errorHandler:nil];
    };

    auto fabricIndex = @(controller.fabricIndex);
    auto download = [_downloads add:type fabricIndex:fabricIndex nodeID:nodeID timeout:timeout queue:queue completion:completion done:done];
    VerifyOrReturn(nil != download,
        dispatch_async(queue, ^{ completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTERNAL]); }));

    auto interactionModelDone = ^(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable response, NSError * _Nullable error) {
        [download checkInteractionModelResponse:response error:error];
    };

    auto * device = [MTRDevice deviceWithNodeID:nodeID controller:controller];
    auto * cluster = [[MTRClusterDiagnosticLogs alloc] initWithDevice:device endpointID:@(kDiagnosticLogsEndPoint) queue:queue];

    auto * params = [[MTRDiagnosticLogsClusterRetrieveLogsRequestParams alloc] init];
    params.intent = @(type);
    params.requestedProtocol = @(MTRDiagnosticLogsTransferProtocolBDX);
    params.transferFileDesignator = download.fileDesignator;

    [cluster retrieveLogsRequestWithParams:params expectedValues:nil expectedValueInterval:nil completion:interactionModelDone];

    MTR_LOG("%@ Started log download attempt for node %016llX-%016llX (%llu)", download,
        controller.compressedFabricID.unsignedLongLongValue, nodeID.unsignedLongLongValue, nodeID.unsignedLongLongValue);
}

- (void)abortDownloadsForController:(MTRDeviceController_Concrete *)controller;
{
    assertChipStackLockedByCurrentThread();

    [_downloads abortDownloadsForController:controller];
}

- (void)handleBDXTransferSessionBeginForFileDesignator:(NSString *)fileDesignator
                                           fabricIndex:(NSNumber *)fabricIndex
                                                nodeID:(NSNumber *)nodeID
                                            completion:(MTRStatusCompletion)completion
                                          abortHandler:(AbortHandler)abortHandler;
{
    assertChipStackLockedByCurrentThread();

    auto * download = [_downloads get:fileDesignator fabricIndex:fabricIndex nodeID:nodeID];

    auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:fabricIndex.unsignedCharValue];

    MTR_LOG("%@ BDX Transfer Session Begin for log download: %016llX-%016llX (%llu), %@", download,
        controller.compressedFabricID.unsignedLongLongValue, nodeID.unsignedLongLongValue, nodeID.unsignedLongLongValue,
        fileDesignator);

    VerifyOrReturn(nil != download, completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_FOUND]));

    download.abortHandler = abortHandler;
    completion(nil);
}

- (void)handleBDXTransferSessionDataForFileDesignator:(NSString *)fileDesignator
                                          fabricIndex:(NSNumber *)fabricIndex
                                               nodeID:(NSNumber *)nodeID
                                                 data:(NSData *)data
                                           completion:(MTRStatusCompletion)completion
{
    assertChipStackLockedByCurrentThread();

    auto * download = [_downloads get:fileDesignator fabricIndex:fabricIndex nodeID:nodeID];

    auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:fabricIndex.unsignedCharValue];

    MTR_LOG("%@ BDX Transfer Session Data for log download: %016llX-%016llX (%llu), %@: %@", download,
        controller.compressedFabricID.unsignedLongLongValue, nodeID.unsignedLongLongValue, nodeID.unsignedLongLongValue,
        fileDesignator, data);

    VerifyOrReturn(nil != download, completion([MTRError errorForCHIPErrorCode:CHIP_ERROR_NOT_FOUND]));

    NSError * error = nil;
    [download writeToFile:data error:&error];
    VerifyOrReturn(nil != error, completion(error));

    completion(nil);
}

- (void)handleBDXTransferSessionEndForFileDesignator:(NSString *)fileDesignator
                                         fabricIndex:(NSNumber *)fabricIndex
                                              nodeID:(NSNumber *)nodeID
                                               error:(NSError * _Nullable)error
{
    assertChipStackLockedByCurrentThread();

    auto * download = [_downloads get:fileDesignator fabricIndex:fabricIndex nodeID:nodeID];

    auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:fabricIndex.unsignedCharValue];

    MTR_LOG("%@ BDX Transfer Session End for log download: %016llX-%016llX (%llu), %@: %@", download,
        controller.compressedFabricID.unsignedLongLongValue, nodeID.unsignedLongLongValue, nodeID.unsignedLongLongValue,
        fileDesignator, error);

    VerifyOrReturn(nil != download);

    VerifyOrReturn(nil == error, [download failure:error]);
    [download success];
}
@end

DiagnosticLogsDownloaderBridge::DiagnosticLogsDownloaderBridge(MTRDiagnosticLogsDownloader * delegate)
{
    mDelegate = delegate;
}

DiagnosticLogsDownloaderBridge::~DiagnosticLogsDownloaderBridge()
{
    mDelegate = nil;
}

CHIP_ERROR DiagnosticLogsDownloaderBridge::OnTransferBegin(chip::bdx::BDXTransferProxy * transfer)
{
    VerifyOrReturnError(nil != mDelegate, CHIP_ERROR_INCORRECT_STATE);

    auto fileDesignatorSpan = transfer->GetFileDesignator();
    auto fileDesignator = AsString(fileDesignatorSpan);
    VerifyOrReturnError(nil != fileDesignator, CHIP_ERROR_INCORRECT_STATE);

    auto * fabricIndex = @(transfer->GetFabricIndex());
    auto * nodeId = @(transfer->GetPeerNodeId());

    auto completionHandler = ^(NSError * _Nullable error) {
        assertChipStackLockedByCurrentThread();

        if (error != nil) {
            auto err = [MTRError errorToCHIPErrorCode:error];
            transfer->Reject(err);
        } else {
            transfer->Accept();
        }
    };

    auto abortHandler = ^(NSError * error) {
        assertChipStackLockedByCurrentThread();
        auto err = [MTRError errorToCHIPErrorCode:error];
        transfer->Reject(err);
    };

    [mDelegate handleBDXTransferSessionBeginForFileDesignator:fileDesignator
                                                  fabricIndex:fabricIndex
                                                       nodeID:nodeId
                                                   completion:completionHandler
                                                 abortHandler:abortHandler];
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticLogsDownloaderBridge::OnTransferEnd(chip::bdx::BDXTransferProxy * transfer, CHIP_ERROR error)
{
    VerifyOrReturnError(nil != mDelegate, CHIP_ERROR_INCORRECT_STATE);

    auto fileDesignatorSpan = transfer->GetFileDesignator();
    auto fileDesignator = AsString(fileDesignatorSpan);
    VerifyOrReturnError(nil != fileDesignator, CHIP_ERROR_INCORRECT_STATE);

    NSError * mtrError = nil;
    if (CHIP_NO_ERROR != error) {
        mtrError = [MTRError errorForCHIPErrorCode:error];
    }

    auto * fabricIndex = @(transfer->GetFabricIndex());
    auto * nodeId = @(transfer->GetPeerNodeId());
    [mDelegate handleBDXTransferSessionEndForFileDesignator:fileDesignator
                                                fabricIndex:fabricIndex
                                                     nodeID:nodeId
                                                      error:mtrError];
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticLogsDownloaderBridge::OnTransferData(chip::bdx::BDXTransferProxy * transfer, const chip::ByteSpan & dataSpan)
{
    VerifyOrReturnError(nil != mDelegate, CHIP_ERROR_INCORRECT_STATE);

    auto fileDesignatorSpan = transfer->GetFileDesignator();
    auto fileDesignator = AsString(fileDesignatorSpan);
    VerifyOrReturnError(nil != fileDesignator, CHIP_ERROR_INCORRECT_STATE);

    auto * fabricIndex = @(transfer->GetFabricIndex());
    auto * nodeId = @(transfer->GetPeerNodeId());

    auto data = AsData(dataSpan);
    VerifyOrReturnError(nil != data, CHIP_ERROR_INCORRECT_STATE);

    auto completionHandler = ^(NSError * _Nullable error) {
        assertChipStackLockedByCurrentThread();

        if (error != nil) {
            auto err = [MTRError errorToCHIPErrorCode:error];
            transfer->Reject(err);
        } else {
            transfer->Continue();
        }
    };

    [mDelegate handleBDXTransferSessionDataForFileDesignator:fileDesignator
                                                 fabricIndex:fabricIndex
                                                      nodeID:nodeId
                                                        data:data
                                                  completion:completionHandler];
    return CHIP_NO_ERROR;
}
