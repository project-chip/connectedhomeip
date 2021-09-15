/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/util/util.h>
#include <controller/CHIPDevice.h>
#include <controller/CHIPDeviceController.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/RandUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
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
using chip::ArgParser::HelpOptions;
using chip::ArgParser::OptionDef;
using chip::ArgParser::OptionSet;
using chip::ArgParser::PrintArgError;
using chip::bdx::TransferSession;
using chip::Callback::Callback;
using chip::Controller::Device;
using chip::Controller::OnDeviceConnected;
using chip::Controller::OnDeviceConnectionFailure;

// TODO: would be nicer to encapsulate these globals and the callbacks in some sort of class
chip::Messaging::ExchangeContext * exchangeCtx = nullptr;
Device * providerDevice                        = nullptr;
BdxDownloader bdxDownloader;
void OnQueryImageResponse(void * context, uint8_t status, uint32_t delayedActionTime, uint8_t * imageURI, uint32_t softwareVersion,
                          chip::ByteSpan updateToken, bool userConsentNeeded, chip::ByteSpan metadataForRequestor)
{
    ChipLogDetail(SoftwareUpdate, "%s", __FUNCTION__);

    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = chip::bdx::TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize     = 1024;
    char testFileDes[9]          = { "test.txt" };
    initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);

    {
        chip::Messaging::ExchangeManager * exchangeMgr = providerDevice->GetExchangeManager();
        chip::Optional<chip::SessionHandle> session    = providerDevice->GetSecureSession();
        if (exchangeMgr != nullptr && session.HasValue())
        {
            exchangeCtx = exchangeMgr->NewContext(session.Value(), &bdxDownloader);
        }

        if (exchangeCtx == nullptr)
        {
            ChipLogError(BDX, "unable to allocate ec: exchangeMgr=%p sessionExists? %u", exchangeMgr, session.HasValue());
            return;
        }
    }

    bdxDownloader.SetInitialExchange(exchangeCtx);

    // This will kick of a timer which will regularly check for updates to the bdx::TransferSession state machine.
    bdxDownloader.InitiateTransfer(&chip::DeviceLayer::SystemLayer(), chip::bdx::TransferRole::kReceiver, initOptions, 20000);
}

void OnQueryFailure(void * context, uint8_t status)
{
    ChipLogDetail(SoftwareUpdate, "QueryImage failure response %" PRIu8, status);
}

Callback<OtaSoftwareUpdateProviderClusterQueryImageResponseCallback> mQueryImageResponseCallback(OnQueryImageResponse, nullptr);
Callback<DefaultFailureCallback> mOnQueryFailureCallback(OnQueryFailure, nullptr);
void OnConnection(void * context, Device * device)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Controller::OtaSoftwareUpdateProviderCluster cluster;
    constexpr EndpointId kOtaProviderEndpoint = 0;

    chip::Callback::Cancelable * successCallback = mQueryImageResponseCallback.Cancel();
    chip::Callback::Cancelable * failureCallback = mOnQueryFailureCallback.Cancel();

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

    err = cluster.Associate(device, kOtaProviderEndpoint);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Associate() failed: %s", chip::ErrorStr(err));
        return;
    }
    err = cluster.QueryImage(successCallback, failureCallback, testVendorId, testProductId, testImageType, testHWVersion,
                             testCurrentVersion, testProtocolsSupported, location, clientCanConsent, metadata);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "QueryImage() failed: %s", chip::ErrorStr(err));
    }
}

void OnConnectFail(void * context, chip::NodeId deviceId, CHIP_ERROR error)
{
    ChipLogError(SoftwareUpdate, "failed to connect to 0x%" PRIX64 ": %s", deviceId, chip::ErrorStr(error));
}

Callback<OnDeviceConnected> mConnectionCallback(OnConnection, nullptr);
Callback<OnDeviceConnectionFailure> mConnectFailCallback(OnConnectFail, nullptr);

PersistentStorage mStorage;
chip::Controller::ExampleOperationalCredentialsIssuer mOpCredsIssuer;
chip::Controller::DeviceController mController;
chip::Controller::ControllerInitParams initParams;

chip::Protocols::Id FromFullyQualified(uint32_t rawProtocolId)
{
    VendorId vendorId   = static_cast<VendorId>(rawProtocolId >> 16);
    uint16_t protocolId = static_cast<uint16_t>(rawProtocolId & 0x0000FFFF);
    return chip::Protocols::Id(vendorId, protocolId);
}

constexpr uint16_t kOptionPeerId = 'p';
chip::NodeId providerNodeId      = 0x0;

bool HandleOptions(const char * aProgram, OptionSet * aOptions, int aIdentifier, const char * aName, const char * aValue)
{
    bool retval = true;

    switch (aIdentifier)
    {
    case kOptionPeerId:
        if (1 != sscanf(aValue, "%" PRIX64, &providerNodeId))
        {
            PrintArgError("%s: unable to parse Node ID: %s\n", aProgram, aValue);
        }
        break;
    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        retval = false;
        break;
    }

    return (retval);
}

OptionDef cmdLineOptionsDef[] = {
    { "peerId", chip::ArgParser::kArgumentRequired, kOptionPeerId },
    {},
};

OptionSet cmdLineOptions = { HandleOptions, cmdLineOptionsDef, "PROGRAM OPTIONS",
                             "  -p <peer ID>\n"
                             "  --peerId <peer ID>\n"
                             "        Node ID of the OTA Provider to connect to (hex)\n" };

HelpOptions helpOptions("ota-requestor-app", "Usage: ota-requestor-app [options]", "1.0");

OptionSet * allOptions[] = { &cmdLineOptions, &helpOptions, nullptr };

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;

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

    if (!chip::ArgParser::ParseArgs(argv[0], argc, argv, allOptions))
    {
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

    VerifyOrExit(rcac.Alloc(chip::Controller::kMaxCHIPDERCertLength), err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(noc.Alloc(chip::Controller::kMaxCHIPDERCertLength), err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(icac.Alloc(chip::Controller::kMaxCHIPDERCertLength), err = CHIP_ERROR_NO_MEMORY);

    {
        chip::MutableByteSpan nocSpan(noc.Get(), chip::Controller::kMaxCHIPDERCertLength);
        chip::MutableByteSpan icacSpan(icac.Get(), chip::Controller::kMaxCHIPDERCertLength);
        chip::MutableByteSpan rcacSpan(rcac.Get(), chip::Controller::kMaxCHIPDERCertLength);

        chip::Crypto::P256Keypair ephemeralKey;
        SuccessOrExit(err = ephemeralKey.Initialize());

        // TODO - OpCreds should only be generated for pairing command
        //        store the credentials in persistent storage, and
        //        generate when not available in the storage.
        err = mOpCredsIssuer.GenerateNOCChainAfterValidation(mStorage.GetLocalNodeId(), 0, ephemeralKey.Pubkey(), rcacSpan,
                                                             icacSpan, nocSpan);
        SuccessOrExit(err);

        initParams.ephemeralKeypair = &ephemeralKey;
        initParams.controllerRCAC   = rcacSpan;
        initParams.controllerICAC   = icacSpan;
        initParams.controllerNOC    = nocSpan;

        err = mController.Init(initParams);
        VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Commissioner: %s", chip::ErrorStr(err)));
    }

    err = mController.ServiceEvents();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Run Loop: %s", chip::ErrorStr(err)));

    ChipLogProgress(SoftwareUpdate, "Attempting to connect to device 0x%" PRIX64, providerNodeId);

    // WARNING: In order for this to work, you must first pair to the OTA Provider device using chip-tool.
    // Currently, that pairing action will persist the CASE session in persistent memory, which will then be read by the following
    // call.
    err = mController.GetDevice(providerNodeId, &providerDevice);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "No device found: %s", chip::ErrorStr(err)));

    err = providerDevice->EstablishConnectivity(&mConnectionCallback, &mConnectFailCallback);

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

exit:
    ChipLogDetail(SoftwareUpdate, "%s", ErrorStr(err));
    return 0;
}
