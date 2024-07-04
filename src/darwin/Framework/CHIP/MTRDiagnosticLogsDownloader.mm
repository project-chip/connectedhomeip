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

#include <protocols/bdx/BdxTransferServerDelegate.h>

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
                       queue:(dispatch_queue_t)queue
                  completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
                        done:(void (^)(MTRDownload * finishedDownload))done;

- (void)writeToFile:(NSData *)data error:(out NSError **)error;

- (BOOL)matches:(NSString *)fileDesignator
    fabricIndex:(NSNumber *)fabricIndex
         nodeID:(NSNumber *)nodeID;

- (void)checkInteractionModelResponse:(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable)response error:(NSError * _Nullable)error;

- (void)success;
- (void)failure:(NSError * _Nullable)error;
@end

@interface MTRDownloads : NSObject
@property (nonatomic, strong) NSMutableArray<MTRDownload *> * downloads;

- (MTRDownload * _Nullable)get:(NSString *)fileDesignator
                   fabricIndex:(NSNumber *)fabricIndex
                        nodeID:(NSNumber *)nodeID;

- (MTRDownload * _Nullable)add:(MTRDiagnosticLogType)type
                   fabricIndex:(NSNumber *)fabricIndex
                        nodeID:(NSNumber *)nodeID
                         queue:(dispatch_queue_t)queue
                    completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
                          done:(void (^)(MTRDownload * finishedDownload))done;
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

    CHIP_ERROR StartBDXTransferTimeout(MTRDownload * download, uint16_t timeoutInSeconds);
    void CancelBDXTransferTimeout(MTRDownload * download);

private:
    static void OnTransferTimeout(chip::System::Layer * layer, void * context);
    MTRDiagnosticLogsDownloader * __weak mDelegate;
};

@implementation MTRDownload
- (instancetype)initWithType:(MTRDiagnosticLogType)type
                 fabricIndex:(NSNumber *)fabricIndex
                      nodeID:(NSNumber *)nodeID
                       queue:(dispatch_queue_t)queue
                  completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
                        done:(void (^)(MTRDownload * finishedDownload))done;
{
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
    }
    return self;
}

- (void)checkInteractionModelResponse:(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable)response error:(NSError * _Nullable)error
{
    VerifyOrReturn(nil == error, [self failure:error]);

    auto status = response.status;

    VerifyOrReturn(![status isEqual:@(MTRDiagnosticLogsStatusBusy)], [self failure:[MTRError errorForCHIPErrorCode:CHIP_ERROR_BUSY]]);
    VerifyOrReturn(![status isEqual:@(MTRDiagnosticLogsStatusDenied)], [self failure:[MTRError errorForCHIPErrorCode:CHIP_ERROR_ACCESS_DENIED]]);

    // If the whole log content fits into the response LogContent field or if there is no log, forward it to the caller
    // and stop here.
    if ([status isEqual:@(MTRDiagnosticLogsStatusExhausted)] || [status isEqual:@(MTRDiagnosticLogsStatusNoLogs)]) {
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

- (void)failure:(NSError * _Nullable)error
{
    _finalize(error);
}

- (void)success
{
    _finalize(nil);
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
                         queue:(dispatch_queue_t)queue
                    completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
                          done:(void (^)(MTRDownload * finishedDownload))done
{
    assertChipStackLockedByCurrentThread();

    auto download = [[MTRDownload alloc] initWithType:type fabricIndex:fabricIndex nodeID:nodeID queue:queue completion:completion done:done];
    VerifyOrReturnValue(nil != download, nil);

    [_downloads addObject:download];
    return download;
}

- (void)remove:(MTRDownload *)download
{
    assertChipStackLockedByCurrentThread();

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
                       controller:(MTRDeviceController *)controller
                             type:(MTRDiagnosticLogType)type
                          timeout:(NSTimeInterval)timeout
                            queue:(dispatch_queue_t)queue
                       completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion;
{
    assertChipStackLockedByCurrentThread();

    uint16_t timeoutInSeconds = 0;
    if (timeout <= 0) {
        timeoutInSeconds = 0;
    } else if (timeout > UINT16_MAX) {
        MTR_LOG("Warning: timeout is too large. It will be truncated to UINT16_MAX.");
        timeoutInSeconds = UINT16_MAX;
    } else {
        timeoutInSeconds = static_cast<uint16_t>(timeout);
    }

    // This block is always called when a download is finished.
    auto done = ^(MTRDownload * finishedDownload) {
        [controller asyncDispatchToMatterQueue:^() {
            [self->_downloads remove:finishedDownload];

            if (timeoutInSeconds > 0) {
                self->_bridge->CancelBDXTransferTimeout(finishedDownload);
            }
        } errorHandler:nil];
    };

    auto fabricIndex = @(controller.fabricIndex);
    auto download = [_downloads add:type fabricIndex:fabricIndex nodeID:nodeID queue:queue completion:completion done:done];
    VerifyOrReturn(nil != download,
        dispatch_async(queue, ^{ completion(nil, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INTERNAL]); }));

    auto interactionModelDone = ^(MTRDiagnosticLogsClusterRetrieveLogsResponseParams * _Nullable response, NSError * _Nullable error) {
        [download checkInteractionModelResponse:response error:error];
    };

    auto * device = [controller deviceForNodeID:nodeID];
    auto * cluster = [[MTRClusterDiagnosticLogs alloc] initWithDevice:device endpointID:@(kDiagnosticLogsEndPoint) queue:queue];

    auto * params = [[MTRDiagnosticLogsClusterRetrieveLogsRequestParams alloc] init];
    params.intent = @(type);
    params.requestedProtocol = @(MTRDiagnosticLogsTransferProtocolBDX);
    params.transferFileDesignator = download.fileDesignator;

    [cluster retrieveLogsRequestWithParams:params expectedValues:nil expectedValueInterval:nil completion:interactionModelDone];

    if (timeoutInSeconds > 0) {
        auto err = _bridge->StartBDXTransferTimeout(download, timeoutInSeconds);
        VerifyOrReturn(CHIP_NO_ERROR == err, [download failure:[MTRError errorForCHIPErrorCode:err]]);
    }
}

- (void)handleBDXTransferSessionBeginForFileDesignator:(NSString *)fileDesignator
                                           fabricIndex:(NSNumber *)fabricIndex
                                                nodeID:(NSNumber *)nodeID
                                            completion:(MTRStatusCompletion)completion
                                          abortHandler:(AbortHandler)abortHandler;
{
    assertChipStackLockedByCurrentThread();
    MTR_LOG("BDX Transfer Session Begin: %@", fileDesignator);

    auto * download = [_downloads get:fileDesignator fabricIndex:fabricIndex nodeID:nodeID];
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
    MTR_LOG("BDX Transfer Session Data: %@: %@", fileDesignator, data);

    auto * download = [_downloads get:fileDesignator fabricIndex:fabricIndex nodeID:nodeID];
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
    MTR_LOG("BDX Transfer Session End: %@: %@", fileDesignator, error);

    auto * download = [_downloads get:fileDesignator fabricIndex:fabricIndex nodeID:nodeID];
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

CHIP_ERROR DiagnosticLogsDownloaderBridge::StartBDXTransferTimeout(MTRDownload * download, uint16_t timeoutInSeconds)
{
    assertChipStackLockedByCurrentThread();
    return chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(timeoutInSeconds), OnTransferTimeout, (__bridge void *) download);
}

void DiagnosticLogsDownloaderBridge::CancelBDXTransferTimeout(MTRDownload * download)
{
    assertChipStackLockedByCurrentThread();
    chip::DeviceLayer::SystemLayer().CancelTimer(OnTransferTimeout, (__bridge void *) download);
}

void DiagnosticLogsDownloaderBridge::OnTransferTimeout(chip::System::Layer * layer, void * context)
{
    assertChipStackLockedByCurrentThread();

    auto * download = (__bridge MTRDownload *) context;
    VerifyOrReturn(nil != download);

    // If there is no abortHandler, it means that the BDX transfer has not started.
    // When a BDX transfer has started we need to abort the transfer and we would error out
    // at next poll. We would end up calling OnTransferEnd and eventually [download failure:error].
    // But if the transfer has not started we would stop right now.
    auto error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_TIMEOUT];
    if (download.abortHandler == nil) {
        [download failure:error];
    } else {
        download.abortHandler(error);
    }
}
