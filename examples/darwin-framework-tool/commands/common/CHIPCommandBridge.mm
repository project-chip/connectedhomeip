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

#include "MTRError_Utils.h"

#include <map>
#include <string>

static CHIPToolPersistentStorageDelegate * storage = nil;
std::set<CHIPCommandBridge *> CHIPCommandBridge::sDeferredCleanups;
std::map<std::string, MTRDeviceController *> CHIPCommandBridge::mControllers;
dispatch_queue_t CHIPCommandBridge::mOTAProviderCallbackQueue;
OTAProviderDelegate * CHIPCommandBridge::mOTADelegate;
constexpr char kTrustStorePathVariable[] = "PAA_TRUST_STORE_PATH";

CHIPToolKeypair * gNocSigner = [[CHIPToolKeypair alloc] init];

CHIP_ERROR CHIPCommandBridge::Run()
{
    ChipLogProgress(chipTool, "Running Command");
    ReturnErrorOnFailure(MaybeSetUpStack());
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
    if (IsInteractive()) {
        return CHIP_NO_ERROR;
    }
    NSData * ipk;
    gNocSigner = [[CHIPToolKeypair alloc] init];
    storage = [[CHIPToolPersistentStorageDelegate alloc] init];

    mOTADelegate = [[OTAProviderDelegate alloc] init];

    auto factory = [MTRDeviceControllerFactory sharedInstance];
    if (factory == nil) {
        ChipLogError(chipTool, "Controller factory is nil");
        return CHIP_ERROR_INTERNAL;
    }

    auto params = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    params.shouldStartServer = YES;
    params.otaProviderDelegate = mOTADelegate;
    NSArray<NSData *> * paaCertResults;
    ReturnLogErrorOnFailure(GetPAACertsFromFolder(&paaCertResults));
    if ([paaCertResults count] > 0) {
        params.productAttestationAuthorityCertificates = paaCertResults;
    }

    NSError * error;
    if ([factory startControllerFactory:params error:&error] == NO) {
        ChipLogError(chipTool, "Controller factory startup failed");
        return MTRErrorToCHIPErrorCode(error);
    }

    ReturnLogErrorOnFailure([gNocSigner createOrLoadKeys:storage]);

    ipk = [gNocSigner getIPK];

    constexpr const char * identities[] = { kIdentityAlpha, kIdentityBeta, kIdentityGamma };
    std::string commissionerName = mCommissionerName.HasValue() ? mCommissionerName.Value() : kIdentityAlpha;
    for (size_t i = 0; i < ArraySize(identities); ++i) {
        auto controllerParams = [[MTRDeviceControllerStartupParams alloc] initWithIPK:ipk fabricID:@(i + 1) nocSigner:gNocSigner];

        if (commissionerName.compare(identities[i]) == 0 && mCommissionerNodeId.HasValue()) {
            controllerParams.nodeId = @(mCommissionerNodeId.Value());
        }
        // We're not sure whether we're creating a new fabric or using an
        // existing one, so just try both.
        auto controller = [factory createControllerOnExistingFabric:controllerParams error:&error];
        if (controller == nil) {
            // Maybe we didn't have this fabric yet.
            controllerParams.vendorID = @(mCommissionerVendorId.ValueOr(chip::VendorId::TestVendor1));
            controller = [factory createControllerOnNewFabric:controllerParams error:&error];
        }
        if (controller == nil) {
            ChipLogError(chipTool, "Controller startup failure.");
            return MTRErrorToCHIPErrorCode(error);
        }

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
    mCurrentController = mControllers[name];
}

MTRDeviceController * CHIPCommandBridge::CurrentCommissioner() { return mCurrentController; }

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

    auto factory = [MTRDeviceControllerFactory sharedInstance];
    NSData * ipk = [gNocSigner getIPK];

    constexpr const char * identities[] = { kIdentityAlpha, kIdentityBeta, kIdentityGamma };
    for (size_t i = 0; i < ArraySize(identities); ++i) {
        auto controllerParams = [[MTRDeviceControllerStartupParams alloc] initWithIPK:ipk fabricID:@(i + 1) nocSigner:gNocSigner];

        auto controller = [factory createControllerOnExistingFabric:controllerParams error:nil];
        mControllers[identities[i]] = controller;
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
