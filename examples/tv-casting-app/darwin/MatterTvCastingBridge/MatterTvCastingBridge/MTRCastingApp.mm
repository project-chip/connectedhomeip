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

#import "MTRCastingApp.h"

#import "MTRCommissionableDataProvider.h"
#import "MTRCommonCaseDeviceServerInitParamsProvider.h"
#import "MTRDeviceAttestationCredentialsProvider.h"
#import "MTRErrorUtils.h"
#import "MTRRotatingDeviceIdUniqueIdProvider.h"

#import "core/Types.h"
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>

#import <Foundation/Foundation.h>

@interface MTRCastingApp ()

@property matter::casting::support::AppParameters appParameters;
@property matter::casting::support::MTRRotatingDeviceIdUniqueIdProvider uniqueIdProvider;
@property matter::casting::support::MTRCommissionableDataProvider * commissionableDataProvider;
@property matter::casting::support::MTRDeviceAttestationCredentialsProvider * dacProvider;
@property MTRCommonCaseDeviceServerInitParamsProvider * serverInitParamsProvider;

// queue used when calling the client code on completion blocks from any MatterTvCastingBridge API
@property dispatch_queue_t _Nonnull clientQueue;

// queue used to perform all work performed by the MatterTvCastingBridge
@property (atomic) dispatch_queue_t workQueue;

@end

@implementation MTRCastingApp

+ (MTRCastingApp *)getSharedInstance
{
    static MTRCastingApp * instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (dispatch_queue_t)getWorkQueue
{
    return _workQueue;
}

- (dispatch_queue_t)getClientQueue
{
    return _clientQueue;
}

- (NSError *)initializeWithDataSource:(id)dataSource
{
    ChipLogProgress(AppServer, "MTRCastingApp.initializeWithDataSource called");

    // get the clientQueue
    VerifyOrReturnValue([dataSource clientQueue] != nil, [MTRErrorUtils NSErrorFromChipError:CHIP_ERROR_INVALID_ARGUMENT]);
    _clientQueue = [dataSource clientQueue];

    // Initialize cpp Providers
    VerifyOrReturnValue(_uniqueIdProvider.Initialize(dataSource) == CHIP_NO_ERROR, [MTRErrorUtils NSErrorFromChipError:CHIP_ERROR_INVALID_ARGUMENT]);

    _commissionableDataProvider = new matter::casting::support::MTRCommissionableDataProvider();
    VerifyOrReturnValue(_commissionableDataProvider->Initialize(dataSource) == CHIP_NO_ERROR, [MTRErrorUtils NSErrorFromChipError:CHIP_ERROR_INVALID_ARGUMENT]);

    _dacProvider = new matter::casting::support::MTRDeviceAttestationCredentialsProvider();
    VerifyOrReturnValue(_dacProvider->Initialize(dataSource) == CHIP_NO_ERROR, [MTRErrorUtils NSErrorFromChipError:CHIP_ERROR_INVALID_ARGUMENT]);

    _serverInitParamsProvider = new MTRCommonCaseDeviceServerInitParamsProvider();

    // Create cpp AppParameters
    VerifyOrReturnValue(_appParameters.Create(&_uniqueIdProvider, _commissionableDataProvider, _dacProvider,
                            GetDefaultDACVerifier(chip::Credentials::GetTestAttestationTrustStore()), _serverInitParamsProvider)
            == CHIP_NO_ERROR,
        [MTRErrorUtils NSErrorFromChipError:CHIP_ERROR_INVALID_ARGUMENT]);

    // Initialize cpp CastingApp
    VerifyOrReturnValue(matter::casting::core::CastingApp::GetInstance()->Initialize(_appParameters) == CHIP_NO_ERROR,
        [MTRErrorUtils NSErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);

    // Get and store the CHIP Work queue
    _workQueue = chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue();

    return [MTRErrorUtils NSErrorFromChipError:CHIP_NO_ERROR];
}

- (void)startWithCompletionBlock:(void (^)(NSError *))completion
{
    ChipLogProgress(AppServer, "MTRCastingApp.startWithCompletionBlock called");
    VerifyOrReturn(_workQueue != nil && _clientQueue != nil, dispatch_async(self->_clientQueue, ^{
        completion([MTRErrorUtils NSErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
    }));

    dispatch_async(_workQueue, ^{
        __block CHIP_ERROR err = matter::casting::core::CastingApp::GetInstance()->Start();
        dispatch_async(self->_clientQueue, ^{
            completion([MTRErrorUtils NSErrorFromChipError:err]);
        });
    });
    __block CHIP_ERROR err = chip::DeviceLayer::PlatformMgrImpl().StartEventLoopTask();
    VerifyOrReturn(err == CHIP_NO_ERROR, dispatch_async(self->_clientQueue, ^{
        completion([MTRErrorUtils NSErrorFromChipError:err]);
    }));
}

- (void)stopWithCompletionBlock:(void (^)(NSError *))completion
{
    ChipLogProgress(AppServer, "MTRCastingApp.stopWithCompletionBlock called");
    VerifyOrReturn(_workQueue != nil && _clientQueue != nil, dispatch_async(self->_clientQueue, ^{
        completion([MTRErrorUtils NSErrorFromChipError:CHIP_ERROR_INCORRECT_STATE]);
    }));

    dispatch_async(_workQueue, ^{
        __block CHIP_ERROR err = matter::casting::core::CastingApp::GetInstance()->Stop();

        dispatch_async(self->_clientQueue, ^{
            completion([MTRErrorUtils NSErrorFromChipError:err]);
        });
    });
}

- (bool)isRunning
{
    VerifyOrReturnValue(_workQueue != nil && _clientQueue != nil, false);

    __block bool running = false;
    dispatch_sync(_workQueue, ^{
        running = matter::casting::core::CastingApp::GetInstance()->isRunning();
    });
    return running;
}

- (NSError *)ShutdownAllSubscriptions
{
    ChipLogProgress(AppServer, "MTRCastingApp.ShutdownAllSubscriptions called");
    __block CHIP_ERROR err = CHIP_NO_ERROR;
    dispatch_sync(_workQueue, ^{
        err = matter::casting::core::CastingApp::GetInstance()->ShutdownAllSubscriptions();
    });
    return [MTRErrorUtils NSErrorFromChipError:err];
}

- (NSError *)ClearCache
{
    ChipLogProgress(AppServer, "MTRCastingApp.ClearCache called");
    __block CHIP_ERROR err = CHIP_NO_ERROR;
    dispatch_sync(_workQueue, ^{
        err = matter::casting::core::CastingApp::GetInstance()->ClearCache();
    });
    return [MTRErrorUtils NSErrorFromChipError:err];
}

@end
