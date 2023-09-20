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

// queue used to serialize all work performed by the CastingServerBridge
@property (atomic) dispatch_queue_t chipWorkQueue;

@end

@implementation MTRCastingApp

+ (MTRCastingApp * _Nullable)getSharedInstance
{
    static MTRCastingApp * instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (MatterError * _Nonnull)initializeWithDataSource:(id _Nonnull)dataSource
{
    ChipLogProgress(AppServer, "MTRCastingApp.initializeWithDataSource called");

    // Initialize cpp Providers
    VerifyOrReturnValue(_uniqueIdProvider.Initialize(dataSource) == CHIP_NO_ERROR, MATTER_ERROR_INVALID_ARGUMENT);

    _commissionableDataProvider = new matter::casting::support::MTRCommissionableDataProvider();
    VerifyOrReturnValue(_commissionableDataProvider->Initialize(dataSource) == CHIP_NO_ERROR, MATTER_ERROR_INVALID_ARGUMENT);

    _dacProvider = new matter::casting::support::MTRDeviceAttestationCredentialsProvider();
    VerifyOrReturnValue(_dacProvider->Initialize(dataSource) == CHIP_NO_ERROR, MATTER_ERROR_INVALID_ARGUMENT);

    _serverInitParamsProvider = new MTRCommonCaseDeviceServerInitParamsProvider();

    // Create cpp AppParameters
    VerifyOrReturnValue(_appParameters.Create(&_uniqueIdProvider, _commissionableDataProvider, _dacProvider,
                            GetDefaultDACVerifier(chip::Credentials::GetTestAttestationTrustStore()), _serverInitParamsProvider)
            == CHIP_NO_ERROR,
        MATTER_ERROR_INVALID_ARGUMENT);

    // Initialize cpp CastingApp
    VerifyOrReturnValue(matter::casting::core::CastingApp::GetInstance()->Initialize(_appParameters) == CHIP_NO_ERROR,
        MATTER_ERROR_INCORRECT_STATE);

    // Get and store the CHIP Work queue
    _chipWorkQueue = chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue();

    return MATTER_NO_ERROR;
}

- (MatterError * _Nonnull)start
{
    ChipLogProgress(AppServer, "MTRCastingApp.start called");
    VerifyOrReturnValue(_chipWorkQueue != nil, MATTER_ERROR_INCORRECT_STATE);

    __block CHIP_ERROR err = CHIP_NO_ERROR;

    dispatch_sync(_chipWorkQueue, ^{
        err = matter::casting::core::CastingApp::GetInstance()->Start();
    });

    return err == CHIP_NO_ERROR ? MATTER_NO_ERROR : MATTER_ERROR_INCORRECT_STATE;
}

- (MatterError * _Nonnull)stop
{
    ChipLogProgress(AppServer, "MTRCastingApp.stop called");
    VerifyOrReturnValue(_chipWorkQueue != nil, MATTER_ERROR_INCORRECT_STATE);

    __block CHIP_ERROR err = CHIP_NO_ERROR;

    dispatch_sync(_chipWorkQueue, ^{
        err = matter::casting::core::CastingApp::GetInstance()->Stop();
    });

    return err == CHIP_NO_ERROR ? MATTER_NO_ERROR : MATTER_ERROR_INCORRECT_STATE;
}

@end
