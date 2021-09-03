/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app/util/util.h>
#include <controller/CHIPDevice.h>
#include <controller/CHIPDeviceController.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/RandUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <zap-generated/CHIPClientCallbacks.h>
#include <zap-generated/CHIPClusters.h>

#include "BDXDownloader.h"
#include "PersistentStorage.h"

#include <fstream>
#include <iostream>

using chip::ByteSpan;
using chip::EndpointId;
using chip::VendorId;
using chip::bdx::TransferSession;
using chip::Callback::Callback;

void OnQueryImageResponse(void * context, uint8_t status, uint32_t delayedActionTime, uint8_t * imageURI, uint32_t softwareVersion,
                          chip::ByteSpan updateToken, bool userConsentNeeded, chip::ByteSpan metadataForRequestor)
{
    ChipLogDetail(SoftwareUpdate, "%s", __FUNCTION__);
}

void OnFailure(void * context, uint8_t status)
{
    ChipLogDetail(SoftwareUpdate, "Received failure response %d\n", (int) status);
}

Callback<OtaSoftwareUpdateProviderClusterQueryImageResponseCallback> mQueryImageResponseCallback(OnQueryImageResponse, nullptr);
Callback<DefaultFailureCallback> mOnFailureCallback(OnFailure, nullptr);

chip::Controller::OtaSoftwareUpdateProviderCluster cluster;
const EndpointId kOtaProviderEndpoint = 0;
PersistentStorage mStorage;
chip::Controller::ExampleOperationalCredentialsIssuer mOpCredsIssuer;
chip::Controller::DeviceController mController;
chip::Controller::ControllerInitParams initParams;
chip::Controller::Device * providerDevice;
chip::Messaging::ExchangeManager * exchangeMgr;
BdxDownloader bdxDownloader;

// QueryImage params
constexpr VendorId testVendorId          = VendorId::Common;
constexpr uint16_t testProductId         = 77;
constexpr uint16_t testImageType         = 0;
constexpr uint16_t testHWVersion         = 3;
constexpr uint16_t testCurrentVersion    = 101;
constexpr uint8_t testProtocolsSupported = 0; // TODO: blocked because arrays are being generated as uint8_t
uint8_t locationBuf[3]                   = { 'U', 'S', '\0' };
ByteSpan location(locationBuf);
constexpr bool clientCanConsent = false;
ByteSpan metadata(locationBuf);

chip::Protocols::Id FromFullyQualified(uint32_t rawProtocolId)
{
    VendorId vendorId   = static_cast<VendorId>(rawProtocolId >> 16);
    uint16_t protocolId = static_cast<uint16_t>(rawProtocolId & 0x0000FFFF);
    return chip::Protocols::Id(vendorId, protocolId);
}

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Messaging::ExchangeContext * exchange;

    chip::Callback::Cancelable * successCallback = mQueryImageResponseCallback.Cancel();
    chip::Callback::Cancelable * failureCallback = mOnFailureCallback.Cancel();

    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = chip::bdx::TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize     = 1024;
    char testFileDes[9]          = { "test.txt" };
    initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);

    // NOTE: most of the following Init() calls were just copied from chip-tool code

    if (chip::Platform::MemoryInit() != CHIP_NO_ERROR)
    {

        fprintf(stderr, "FAILED to initialize memory\n");
        return 1;
    }

    if (chip::DeviceLayer::PlatformMgr().InitChipStack() != CHIP_NO_ERROR)
    {
        fprintf(stderr, "FAILED to initialize chip stack\n");
        return 1;
    }

    chip::DeviceLayer::ConfigurationMgr().LogDeviceConfig();
    err = mStorage.Init();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Storage failure: %s", chip::ErrorStr(err)));

    chip::Logging::SetLogFilter(mStorage.GetLoggingLevel());

    initParams.storageDelegate = &mStorage;

    err = mOpCredsIssuer.Initialize(mStorage);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Operational Cred Issuer: %s", chip::ErrorStr(err)));

    initParams.operationalCredentialsDelegate = &mOpCredsIssuer;

    err = mController.SetUdpListenPort(mStorage.GetListenPort());
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Commissioner: %s", chip::ErrorStr(err)));

    err = mController.Init(initParams);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Commissioner: %s", chip::ErrorStr(err)));

    err = mController.ServiceEvents();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Run Loop: %s", chip::ErrorStr(err)));

    // WARNING: In order for this to work, you must first pair to the OTA Provider device using chip-tool.
    // Currently, that pairing action will persist the CASE session in persistent memory, which will then be read by the following
    // call.
    err = mController.GetDevice(chip::kTestDeviceNodeId, &providerDevice);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "No device found: %s", chip::ErrorStr(err)));

    cluster.Associate(providerDevice, kOtaProviderEndpoint);
    cluster.QueryImage(successCallback, failureCallback, testVendorId, testProductId, testImageType, testHWVersion,
                       testCurrentVersion, testProtocolsSupported, location, clientCanConsent, metadata);

    // WARNING: GetNewExchangeContext() was implemented locally as a workaround to get access to an ExchangeContext, but does not
    // exist now. Currently there is GetSecureSession(), but the caller still needs access to the ExchangeManager in order to get a
    // new ExchangeContext.
    exchange = providerDevice->GetNewExchangeContext(&bdxDownloader);
    VerifyOrExit(exchange != nullptr, ChipLogError(BDX, "unable to allocate ec"));

    bdxDownloader.SetInitialExchange(exchange);

    // This will kick of a timer which will regularly check for updates to the bdx::TransferSession state machine.
    bdxDownloader.InitiateTransfer(&chip::DeviceLayer::SystemLayer, chip::bdx::TransferRole::kReceiver, initOptions, 20000);

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

exit:
    ChipLogDetail(BDX, "%s", ErrorStr(err));
    return 0;
}
