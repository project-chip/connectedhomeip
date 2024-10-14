/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "CHIPCommandBridge.h"

#import "CHIPToolKeypair.h"
#import <Matter/Matter.h>

#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <protocols/secure_channel/PASESession.h> // for chip::kTestControllerNodeId

#import "CHIPCommandStorageDelegate.h"
#import "CertificateIssuer.h"
#import "ControllerStorage.h"
#include "MTRError_Utils.h"

#include <map>
#include <string>

static CHIPToolPersistentStorageDelegate * storage = nil;
std::set<CHIPCommandBridge *> CHIPCommandBridge::sDeferredCleanups;
std::map<std::string, MTRDeviceController *> CHIPCommandBridge::mControllers;
dispatch_queue_t CHIPCommandBridge::mOTAProviderCallbackQueue;
OTAProviderDelegate * CHIPCommandBridge::mOTADelegate;
bool CHIPCommandBridge::sUseSharedStorage = true;
constexpr char kTrustStorePathVariable[] = "PAA_TRUST_STORE_PATH";

CHIP_ERROR CHIPCommandBridge::Run()
{
    // In interactive mode, we want to avoid memory accumulating in the main autorelease pool,
    // so we clear it after each command.
    @autoreleasepool {
        ChipLogProgress(chipTool, "Running Command");
        // Although the body of `Run` is within its own autorelease pool, this code block is further wrapped
        // in an additional autorelease pool. This ensures that when the memory dump graph command is used directly,
        // we can verify there’s no additional noise from the autorelease pools—a kind of sanity check.
        @autoreleasepool {
            ReturnErrorOnFailure(MaybeSetUpStack());
        }
        SetIdentity(mCommissionerName.HasValue() ? mCommissionerName.Value() : kIdentityAlpha);

        {
            std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
            mWaitingForResponse = YES;
        }

        ReturnLogErrorOnFailure(RunCommand());

        auto err = StartWaiting(GetWaitDuration());

        bool deferCleanup = (IsInteractive() && DeferInteractiveCleanup());

        Shutdown();

        if (deferCleanup) {
            sDeferredCleanups.insert(this);
        } else {
            Cleanup();
        }
        MaybeTearDownStack();

        return err;
    }
}

CHIP_ERROR CHIPCommandBridge::GetPAACertsFromFolder(NSArray<NSData *> * __autoreleasing * paaCertsResult)
{
    NSMutableArray * paaCerts = [[NSMutableArray alloc] init];

    if (!mPaaTrustStorePath.HasValue()) {
        char * const trust_store_path = getenv(kTrustStorePathVariable);
        if (trust_store_path != nullptr) {
            mPaaTrustStorePath.SetValue(trust_store_path);
        }
    }
    if (mPaaTrustStorePath.HasValue()) {
        NSError * error;
        NSString * paaStorePath = [NSString stringWithCString:mPaaTrustStorePath.Value() encoding:NSUTF8StringEncoding];
        NSArray * derFolder = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:paaStorePath error:&error];
        if (error) {
            NSLog(@"Error: %@", error);
            return CHIP_ERROR_INTERNAL;
        }

        NSArray * derFiles = [derFolder filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"self ENDSWITH '.der'"]];
        if ([derFiles count] == 0) {
            NSLog(@"Unable to find DER cert files");
            return CHIP_ERROR_INTERNAL;
        }
        for (id derFile in derFiles) {
            NSString * certPath = [NSString stringWithFormat:@"%@/%@", paaStorePath, derFile];
            NSData * fileData = [NSData dataWithContentsOfFile:certPath];
            if (fileData) {
                [paaCerts addObject:fileData];
            }
        }
    } else {
        return CHIP_NO_ERROR;
    }
    if ([paaCerts count] == 0) {
        NSLog(@"Unable to find PAA certs");
        return CHIP_ERROR_INTERNAL;
    }
    *paaCertsResult = paaCerts;
    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPCommandBridge::MaybeSetUpStack()
{
    VerifyOrReturnError(!IsInteractive(), CHIP_NO_ERROR);

    mOTADelegate = [[OTAProviderDelegate alloc] init];
    storage = [[CHIPToolPersistentStorageDelegate alloc] init];

    NSError * error;
    __auto_type * certificateIssuer = [CertificateIssuer sharedInstance];
    [certificateIssuer startWithStorage:storage error:&error];
    VerifyOrReturnError(nil == error, MTRErrorToCHIPErrorCode(error), ChipLogError(chipTool, "Can not start the certificate issuer: %@", error));

    NSArray<NSData *> * productAttestationAuthorityCertificates = nil;
    ReturnLogErrorOnFailure(GetPAACertsFromFolder(&productAttestationAuthorityCertificates));
    if ([productAttestationAuthorityCertificates count] == 0) {
        productAttestationAuthorityCertificates = nil;
    }

    sUseSharedStorage = mCommissionerSharedStorage.ValueOr(true);
    if (sUseSharedStorage) {
        return SetUpStackWithSharedStorage(productAttestationAuthorityCertificates);
    }

    return SetUpStackWithPerControllerStorage(productAttestationAuthorityCertificates);
}

CHIP_ERROR CHIPCommandBridge::SetUpStackWithPerControllerStorage(NSArray<NSData *> * productAttestationAuthorityCertificates)
{
    __auto_type * certificateIssuer = [CertificateIssuer sharedInstance];

    constexpr const char * identities[] = { kIdentityAlpha, kIdentityBeta, kIdentityGamma };
    std::string commissionerName = mCommissionerName.HasValue() ? mCommissionerName.Value() : kIdentityAlpha;
    for (size_t i = 0; i < ArraySize(identities); ++i) {
        __auto_type * uuidString = [NSString stringWithFormat:@"%@%@", @"8DCADB14-AF1F-45D0-B084-00000000000", @(i)];
        __auto_type * controllerId = [[NSUUID alloc] initWithUUIDString:uuidString];
        __auto_type * vendorId = @(mCommissionerVendorId.ValueOr(chip::VendorId::TestVendor1));
        __auto_type * fabricId = @(i + 1);
        __auto_type * nodeId = @(chip::kTestControllerNodeId);

        if (commissionerName.compare(identities[i]) == 0 && mCommissionerNodeId.HasValue()) {
            nodeId = @(mCommissionerNodeId.Value());
        }

        __auto_type * controllerStorage = [[ControllerStorage alloc] initWithControllerID:controllerId];

        NSError * error;
        __auto_type * operationalKeypair = [certificateIssuer issueOperationalKeypairWithControllerStorage:controllerStorage error:&error];
        __auto_type * operational = [certificateIssuer issueOperationalCertificateForNodeID:nodeId
                                                                                   fabricID:fabricId
                                                                                  publicKey:operationalKeypair.publicKey
                                                                                      error:&error];
        VerifyOrReturnError(nil == error, MTRErrorToCHIPErrorCode(error), ChipLogError(chipTool, "Can not issue an operational certificate: %@", error));

        __auto_type * controllerStorageQueue = dispatch_queue_create("com.chip.storage", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        __auto_type * params = [[MTRDeviceControllerExternalCertificateParameters alloc] initWithStorageDelegate:controllerStorage
                                                                                            storageDelegateQueue:controllerStorageQueue
                                                                                                uniqueIdentifier:controllerId
                                                                                                             ipk:certificateIssuer.ipk
                                                                                                        vendorID:vendorId
                                                                                              operationalKeypair:operationalKeypair
                                                                                          operationalCertificate:operational
                                                                                         intermediateCertificate:nil
                                                                                                 rootCertificate:certificateIssuer.rootCertificate];
        [params setOperationalCertificateIssuer:certificateIssuer queue:controllerStorageQueue];
        params.productAttestationAuthorityCertificates = productAttestationAuthorityCertificates;

        __auto_type * controller = [[MTRDeviceController alloc] initWithParameters:params error:&error];
        VerifyOrReturnError(nil != controller, MTRErrorToCHIPErrorCode(error), ChipLogError(chipTool, "Controller startup failure: %@", error));
        mControllers[identities[i]] = controller;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPCommandBridge::SetUpStackWithSharedStorage(NSArray<NSData *> * productAttestationAuthorityCertificates)
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    VerifyOrReturnError(nil != factory, CHIP_ERROR_INTERNAL, ChipLogError(chipTool, "Controller factory is nil"));

    auto factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.shouldStartServer = YES;
    factoryParams.otaProviderDelegate = mOTADelegate;
    factoryParams.productAttestationAuthorityCertificates = productAttestationAuthorityCertificates;

    NSError * error;
    auto started = [factory startControllerFactory:factoryParams error:&error];
    VerifyOrReturnError(started, MTRErrorToCHIPErrorCode(error), ChipLogError(chipTool, "Controller factory startup failed"));

    __auto_type * certificateIssuer = [CertificateIssuer sharedInstance];

    constexpr const char * identities[] = { kIdentityAlpha, kIdentityBeta, kIdentityGamma };
    std::string commissionerName = mCommissionerName.HasValue() ? mCommissionerName.Value() : kIdentityAlpha;
    for (size_t i = 0; i < ArraySize(identities); ++i) {
        __auto_type * fabricId = @(i + 1);
        __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:certificateIssuer.ipk
                                                                            fabricID:fabricId
                                                                           nocSigner:certificateIssuer.signingKey];
        if (commissionerName.compare(identities[i]) == 0 && mCommissionerNodeId.HasValue()) {
            params.nodeId = @(mCommissionerNodeId.Value());
        }

        // We're not sure whether we're creating a new fabric or using an existing one, so just try both.
        auto controller = [factory createControllerOnExistingFabric:params error:&error];
        if (controller == nil) {
            // Maybe we didn't have this fabric yet.
            params.vendorID = @(mCommissionerVendorId.ValueOr(chip::VendorId::TestVendor1));
            controller = [factory createControllerOnNewFabric:params error:&error];
        }
        VerifyOrReturnError(nil != controller, MTRErrorToCHIPErrorCode(error), ChipLogError(chipTool, "Controller startup failure: %@", error));
        mControllers[identities[i]] = controller;
    }

    return CHIP_NO_ERROR;
}

void CHIPCommandBridge::MaybeTearDownStack()
{
    if (IsInteractive()) {
        return;
    }
    ShutdownCommissioner();
}

void CHIPCommandBridge::SetIdentity(const char * identity)
{
    std::string name = std::string(identity);
    if (name.compare(kIdentityAlpha) != 0 && name.compare(kIdentityBeta) != 0 && name.compare(kIdentityGamma) != 0) {
        ChipLogError(chipTool, "Unknown commissioner name: %s. Supported names are [%s, %s, %s]", name.c_str(), kIdentityAlpha,
            kIdentityBeta, kIdentityGamma);
        chipDie();
    }
    mCurrentIdentity = name;
    mCurrentController = mControllers[name];
}

MTRDeviceController * CHIPCommandBridge::CurrentCommissioner() { return mCurrentController; }

NSNumber * CHIPCommandBridge::CurrentCommissionerFabricId()
{
    if (mCurrentIdentity.compare(kIdentityAlpha) == 0) {
        return @(1);
    } else if (mCurrentIdentity.compare(kIdentityBeta) == 0) {
        return @(2);
    } else if (mCurrentIdentity.compare(kIdentityGamma) == 0) {
        return @(3);
    } else {
        ChipLogError(chipTool, "Unknown commissioner name: %s. Supported names are [%s, %s, %s]", mCurrentIdentity.c_str(), kIdentityAlpha,
            kIdentityBeta, kIdentityGamma);
        chipDie();
    }

    return @(0); // This should never happens.
}

MTRDeviceController * CHIPCommandBridge::GetCommissioner(const char * identity) { return mControllers[identity]; }

MTRBaseDevice * CHIPCommandBridge::BaseDeviceWithNodeId(chip::NodeId nodeId)
{
    MTRDeviceController * controller = CurrentCommissioner();
    VerifyOrReturnValue(controller != nil, nil);
    return [controller deviceBeingCommissionedWithNodeID:@(nodeId) error:nullptr]
        ?: [MTRBaseDevice deviceWithNodeID:@(nodeId) controller:controller];
}

void CHIPCommandBridge::StopCommissioners()
{
    for (auto & pair : mControllers) {
        [pair.second shutdown];
    }
}

void CHIPCommandBridge::RestartCommissioners()
{
    StopCommissioners();

    if (sUseSharedStorage) {
        auto factory = [MTRDeviceControllerFactory sharedInstance];

        constexpr const char * identities[] = { kIdentityAlpha, kIdentityBeta, kIdentityGamma };
        for (size_t i = 0; i < ArraySize(identities); ++i) {
            __auto_type * certificateIssuer = [CertificateIssuer sharedInstance];
            auto controllerParams = [[MTRDeviceControllerStartupParams alloc] initWithIPK:certificateIssuer.ipk fabricID:@(i + 1) nocSigner:certificateIssuer.signingKey];

            auto controller = [factory createControllerOnExistingFabric:controllerParams error:nil];
            mControllers[identities[i]] = controller;
        }
    } else {
        NSArray<NSData *> * productAttestationAuthorityCertificates = nil;
        ReturnOnFailure(GetPAACertsFromFolder(&productAttestationAuthorityCertificates));
        if ([productAttestationAuthorityCertificates count] == 0) {
            productAttestationAuthorityCertificates = nil;
        }

        ReturnOnFailure(SetUpStackWithPerControllerStorage(productAttestationAuthorityCertificates));
    }
}

void CHIPCommandBridge::ShutdownCommissioner()
{
    ChipLogProgress(chipTool, "Shutting down controller");
    StopCommissioners();
    mControllers.clear();
    mCurrentController = nil;

    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];
}

CHIP_ERROR CHIPCommandBridge::StartWaiting(chip::System::Clock::Timeout duration)
{
    auto waitingUntil = std::chrono::system_clock::now() + std::chrono::duration_cast<std::chrono::seconds>(duration);
    {
        std::unique_lock<std::mutex> lk(cvWaitingForResponseMutex);
        if (!cvWaitingForResponse.wait_until(lk, waitingUntil, [this]() { return !this->mWaitingForResponse; })) {
            mCommandExitStatus = CHIP_ERROR_TIMEOUT;
        }
    }

    return mCommandExitStatus;
}

void CHIPCommandBridge::StopWaiting()
{
    {
        std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
        mWaitingForResponse = NO;
    }
    cvWaitingForResponse.notify_all();
}

void CHIPCommandBridge::SetCommandExitStatus(NSError * error, const char * logString)
{
    if (logString != nullptr) {
        LogNSError(logString, error);
    }
    CHIP_ERROR err = MTRErrorToCHIPErrorCode(error);
    SetCommandExitStatus(err);
}

void CHIPCommandBridge::LogNSError(const char * logString, NSError * error)
{
    CHIP_ERROR err = MTRErrorToCHIPErrorCode(error);
    if (err == CHIP_NO_ERROR) {
        ChipLogProgress(chipTool, "%s: %s", logString, chip::ErrorStr(err));
    } else {
        ChipLogError(chipTool, "%s: %s", logString, chip::ErrorStr(err));
    }
}

void CHIPCommandBridge::ExecuteDeferredCleanups()
{
    for (auto * cmd : sDeferredCleanups) {
        cmd->Cleanup();
    }
    sDeferredCleanups.clear();
}
