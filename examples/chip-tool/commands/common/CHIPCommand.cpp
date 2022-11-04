/*
 *   Copyright (c) 2021-2022 Project CHIP Authors
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

#include "CHIPCommand.h"

#include <controller/CHIPDeviceControllerFactory.h>
#include <core/CHIPBuildConfig.h>
#include <credentials/attestation_verifier/FileAttestationTrustStore.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#include "TraceDecoder.h"
#include "TraceHandlers.h"
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

std::map<CHIPCommand::CommissionerIdentity, std::unique_ptr<chip::Controller::DeviceCommissioner>> CHIPCommand::mCommissioners;
std::set<CHIPCommand *> CHIPCommand::sDeferredCleanups;

using DeviceControllerFactory = chip::Controller::DeviceControllerFactory;

constexpr chip::FabricId kIdentityNullFabricId    = chip::kUndefinedFabricId;
constexpr chip::FabricId kIdentityAlphaFabricId   = 1;
constexpr chip::FabricId kIdentityBetaFabricId    = 2;
constexpr chip::FabricId kIdentityGammaFabricId   = 3;
constexpr chip::FabricId kIdentityOtherFabricId   = 4;
constexpr const char * kPAATrustStorePathVariable = "CHIPTOOL_PAA_TRUST_STORE_PATH";
constexpr const char * kCDTrustStorePathVariable  = "CHIPTOOL_CD_TRUST_STORE_PATH";

const chip::Credentials::AttestationTrustStore * CHIPCommand::sTrustStore = nullptr;
chip::Credentials::GroupDataProviderImpl CHIPCommand::sGroupDataProvider{ kMaxGroupsPerFabric, kMaxGroupKeysPerFabric };

namespace {
CHIP_ERROR GetAttestationTrustStore(const char * paaTrustStorePath, const chip::Credentials::AttestationTrustStore ** trustStore)
{
    if (paaTrustStorePath == nullptr)
    {
        paaTrustStorePath = getenv(kPAATrustStorePathVariable);
    }

    if (paaTrustStorePath == nullptr)
    {
        *trustStore = chip::Credentials::GetTestAttestationTrustStore();
        return CHIP_NO_ERROR;
    }

    static chip::Credentials::FileAttestationTrustStore attestationTrustStore{ paaTrustStorePath };

    if (paaTrustStorePath != nullptr && attestationTrustStore.paaCount() == 0)
    {
        ChipLogError(chipTool, "No PAAs found in path: %s", paaTrustStorePath);
        ChipLogError(chipTool,
                     "Please specify a valid path containing trusted PAA certificates using "
                     "the argument [--paa-trust-store-path paa/file/path] "
                     "or environment variable [%s=paa/file/path]",
                     kPAATrustStorePathVariable);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    *trustStore = &attestationTrustStore;
    return CHIP_NO_ERROR;
}
} // namespace

CHIP_ERROR CHIPCommand::MaybeSetUpStack()
{
    if (IsInteractive())
    {
        return CHIP_NO_ERROR;
    }

    StartTracing();

#if (CHIP_DEVICE_LAYER_TARGET_LINUX || CHIP_DEVICE_LAYER_TARGET_TIZEN) && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    // By default, Linux device is configured as a BLE peripheral while the controller needs a BLE central.
    ReturnLogErrorOnFailure(chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(mBleAdapterId.ValueOr(0), true));
#endif

    ReturnLogErrorOnFailure(mDefaultStorage.Init());
    ReturnLogErrorOnFailure(mOperationalKeystore.Init(&mDefaultStorage));
    ReturnLogErrorOnFailure(mOpCertStore.Init(&mDefaultStorage));

    chip::Controller::FactoryInitParams factoryInitParams;

    factoryInitParams.fabricIndependentStorage = &mDefaultStorage;
    factoryInitParams.operationalKeystore      = &mOperationalKeystore;
    factoryInitParams.opCertStore              = &mOpCertStore;

    // Init group data provider that will be used for all group keys and IPKs for the
    // chip-tool-configured fabrics. This is OK to do once since the fabric tables
    // and the DeviceControllerFactory all "share" in the same underlying data.
    // Different commissioner implementations may want to use alternate implementations
    // of GroupDataProvider for injection through factoryInitParams.
    sGroupDataProvider.SetStorageDelegate(&mDefaultStorage);
    ReturnLogErrorOnFailure(sGroupDataProvider.Init());
    chip::Credentials::SetGroupDataProvider(&sGroupDataProvider);
    factoryInitParams.groupDataProvider = &sGroupDataProvider;

    uint16_t port = mDefaultStorage.GetListenPort();
    if (port != 0)
    {
        // Make sure different commissioners run on different ports.
        port = static_cast<uint16_t>(port + CurrentCommissionerId());
    }
    factoryInitParams.listenPort = port;
    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().Init(factoryInitParams));

    ReturnErrorOnFailure(GetAttestationTrustStore(mPaaTrustStorePath.ValueOr(nullptr), &sTrustStore));

    CommissionerIdentity nullIdentity{ kIdentityNull, chip::kUndefinedNodeId };
    ReturnLogErrorOnFailure(InitializeCommissioner(nullIdentity, kIdentityNullFabricId));

    // After initializing first commissioner, add the additional CD certs once
    {
        const char * cdTrustStorePath = mCDTrustStorePath.ValueOr(nullptr);
        if (cdTrustStorePath == nullptr)
        {
            cdTrustStorePath = getenv(kCDTrustStorePathVariable);
        }

        auto additionalCdCerts = chip::Credentials::LoadAllX509DerCerts(cdTrustStorePath);
        if (cdTrustStorePath != nullptr && additionalCdCerts.size() == 0)
        {
            ChipLogError(chipTool, "Warning: no CD signing certs found in path: %s, only defaults will be used", cdTrustStorePath);
            ChipLogError(chipTool,
                         "Please specify a path containing trusted CD verifying key certificates using "
                         "the argument [--cd-trust-store-path cd/file/path] "
                         "or environment variable [%s=cd/file/path]",
                         kCDTrustStorePathVariable);
        }
        ReturnErrorOnFailure(mCredIssuerCmds->AddAdditionalCDVerifyingCerts(additionalCdCerts));
    }
    bool allowTestCdSigningKey = !mOnlyAllowTrustedCdKeys.ValueOr(false);
    mCredIssuerCmds->SetCredentialIssuerOption(CredentialIssuerCommands::CredentialIssuerOptions::kAllowTestCdSigningKey,
                                               allowTestCdSigningKey);

    return CHIP_NO_ERROR;
}

void CHIPCommand::MaybeTearDownStack()
{
    if (IsInteractive())
    {
        return;
    }

    //
    // We can call DeviceController::Shutdown() safely without grabbing the stack lock
    // since the CHIP thread and event queue have been stopped, preventing any thread
    // races.
    //
    for (auto & commissioner : mCommissioners)
    {
        ShutdownCommissioner(commissioner.first);
    }

    StopTracing();
}

CHIP_ERROR CHIPCommand::EnsureCommissionerForIdentity(std::string identity)
{
    chip::NodeId nodeId;
    ReturnErrorOnFailure(GetCommissionerNodeId(identity, &nodeId));
    CommissionerIdentity lookupKey{ identity, nodeId };
    if (mCommissioners.find(lookupKey) != mCommissioners.end())
    {
        return CHIP_NO_ERROR;
    }

    // Need to initialize the commissioner.
    chip::FabricId fabricId;
    if (identity == kIdentityAlpha)
    {
        fabricId = kIdentityAlphaFabricId;
    }
    else if (identity == kIdentityBeta)
    {
        fabricId = kIdentityBetaFabricId;
    }
    else if (identity == kIdentityGamma)
    {
        fabricId = kIdentityGammaFabricId;
    }
    else
    {
        fabricId = strtoull(identity.c_str(), nullptr, 0);
        if (fabricId < kIdentityOtherFabricId)
        {
            ChipLogError(chipTool, "Invalid identity: %s", identity.c_str());
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    return InitializeCommissioner(lookupKey, fabricId);
}

CHIP_ERROR CHIPCommand::Run()
{
    ReturnErrorOnFailure(MaybeSetUpStack());

    CHIP_ERROR err = StartWaiting(GetWaitDuration());

    bool deferCleanup = (IsInteractive() && DeferInteractiveCleanup());

    Shutdown();

    if (deferCleanup)
    {
        sDeferredCleanups.insert(this);
    }
    else
    {
        Cleanup();
    }

    MaybeTearDownStack();

    return err;
}

void CHIPCommand::StartTracing()
{
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::trace::InitTrace();

    if (mTraceFile.HasValue())
    {
        chip::trace::AddTraceStream(new chip::trace::TraceStreamFile(mTraceFile.Value()));
    }
    else if (mTraceLog.HasValue() && mTraceLog.Value())
    {
        chip::trace::AddTraceStream(new chip::trace::TraceStreamLog());
    }

    if (mTraceDecode.HasValue() && mTraceDecode.Value())
    {
        chip::trace::TraceDecoderOptions options;
        // The interaction model protocol is already logged, so just disable logging those.
        options.mEnableProtocolInteractionModelResponse = false;
        chip::trace::TraceDecoder * decoder             = new chip::trace::TraceDecoder();
        decoder->SetOptions(options);
        chip::trace::AddTraceStream(decoder);
    }
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
}

void CHIPCommand::StopTracing()
{
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::trace::DeInitTrace();
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
}

void CHIPCommand::SetIdentity(const char * identity)
{
    std::string name = std::string(identity);
    if (name.compare(kIdentityAlpha) != 0 && name.compare(kIdentityBeta) != 0 && name.compare(kIdentityGamma) != 0 &&
        name.compare(kIdentityNull) != 0 && strtoull(name.c_str(), nullptr, 0) < kIdentityOtherFabricId)
    {
        ChipLogError(chipTool, "Unknown commissioner name: %s. Supported names are [%s, %s, %s, 4, 5...]", name.c_str(),
                     kIdentityAlpha, kIdentityBeta, kIdentityGamma);
        chipDie();
    }

    mCommissionerName.SetValue(const_cast<char *>(identity));
}

std::string CHIPCommand::GetIdentity()
{
    std::string name = mCommissionerName.HasValue() ? mCommissionerName.Value() : kIdentityAlpha;
    if (name.compare(kIdentityAlpha) != 0 && name.compare(kIdentityBeta) != 0 && name.compare(kIdentityGamma) != 0 &&
        name.compare(kIdentityNull) != 0)
    {
        chip::FabricId fabricId = strtoull(name.c_str(), nullptr, 0);
        if (fabricId >= kIdentityOtherFabricId)
        {
            // normalize name since it is used in persistent storage

            char s[24];
            sprintf(s, "%" PRIu64, fabricId);

            name = s;
        }
        else
        {
            ChipLogError(chipTool, "Unknown commissioner name: %s. Supported names are [%s, %s, %s, 4, 5...]", name.c_str(),
                         kIdentityAlpha, kIdentityBeta, kIdentityGamma);
            chipDie();
        }
    }

    return name;
}

CHIP_ERROR CHIPCommand::GetCommissionerNodeId(std::string identity, chip::NodeId * nodeId)
{
    if (mCommissionerNodeId.HasValue())
    {
        *nodeId = mCommissionerNodeId.Value();
        return CHIP_NO_ERROR;
    }

    if (identity == kIdentityNull)
    {
        *nodeId = chip::kUndefinedNodeId;
        return CHIP_NO_ERROR;
    }

    ReturnLogErrorOnFailure(mCommissionerStorage.Init(identity.c_str()));

    *nodeId = mCommissionerStorage.GetLocalNodeId();

    return CHIP_NO_ERROR;
}

chip::FabricId CHIPCommand::CurrentCommissionerId()
{
    chip::FabricId id;

    std::string name = GetIdentity();
    if (name.compare(kIdentityAlpha) == 0)
    {
        id = kIdentityAlphaFabricId;
    }
    else if (name.compare(kIdentityBeta) == 0)
    {
        id = kIdentityBetaFabricId;
    }
    else if (name.compare(kIdentityGamma) == 0)
    {
        id = kIdentityGammaFabricId;
    }
    else if (name.compare(kIdentityNull) == 0)
    {
        id = kIdentityNullFabricId;
    }
    else if ((id = strtoull(name.c_str(), nullptr, 0)) < kIdentityOtherFabricId)
    {
        VerifyOrDieWithMsg(false, chipTool, "Unknown commissioner name: %s. Supported names are [%s, %s, %s, 4, 5...]",
                           name.c_str(), kIdentityAlpha, kIdentityBeta, kIdentityGamma);
    }

    return id;
}

chip::Controller::DeviceCommissioner & CHIPCommand::CurrentCommissioner()
{
    return GetCommissioner(GetIdentity());
}

chip::Controller::DeviceCommissioner & CHIPCommand::GetCommissioner(std::string identity)
{
    // We don't have a great way to handle commissioner setup failures here.
    // This only matters for commands (like TestCommand) that involve multiple
    // identities.
    VerifyOrDie(EnsureCommissionerForIdentity(identity) == CHIP_NO_ERROR);

    chip::NodeId nodeId;
    VerifyOrDie(GetCommissionerNodeId(identity, &nodeId) == CHIP_NO_ERROR);
    CommissionerIdentity lookupKey{ identity, nodeId };
    auto item = mCommissioners.find(lookupKey);
    VerifyOrDie(item != mCommissioners.end());
    return *item->second;
}

void CHIPCommand::ShutdownCommissioner(const CommissionerIdentity & key)
{
    mCommissioners[key].get()->Shutdown();
}

CHIP_ERROR CHIPCommand::InitializeCommissioner(const CommissionerIdentity & identity, chip::FabricId fabricId)
{
    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;

    std::unique_ptr<ChipDeviceCommissioner> commissioner = std::make_unique<ChipDeviceCommissioner>();
    chip::Controller::SetupParams commissionerParams;

    ReturnLogErrorOnFailure(mCredIssuerCmds->SetupDeviceAttestation(commissionerParams, sTrustStore));

    VerifyOrReturnError(noc.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(icac.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(rcac.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);

    chip::Crypto::P256Keypair ephemeralKey;

    if (fabricId != chip::kUndefinedFabricId)
    {

        // TODO - OpCreds should only be generated for pairing command
        //        store the credentials in persistent storage, and
        //        generate when not available in the storage.
        ReturnLogErrorOnFailure(mCommissionerStorage.Init(identity.mName.c_str()));
        if (mUseMaxSizedCerts.HasValue())
        {
            auto option = CredentialIssuerCommands::CredentialIssuerOptions::kMaximizeCertificateSizes;
            mCredIssuerCmds->SetCredentialIssuerOption(option, mUseMaxSizedCerts.Value());
        }

        ReturnLogErrorOnFailure(mCredIssuerCmds->InitializeCredentialsIssuer(mCommissionerStorage));

        chip::MutableByteSpan nocSpan(noc.Get(), chip::Controller::kMaxCHIPDERCertLength);
        chip::MutableByteSpan icacSpan(icac.Get(), chip::Controller::kMaxCHIPDERCertLength);
        chip::MutableByteSpan rcacSpan(rcac.Get(), chip::Controller::kMaxCHIPDERCertLength);

        ReturnLogErrorOnFailure(ephemeralKey.Initialize());

        ReturnLogErrorOnFailure(mCredIssuerCmds->GenerateControllerNOCChain(identity.mLocalNodeId, fabricId,
                                                                            mCommissionerStorage.GetCommissionerCATs(),
                                                                            ephemeralKey, rcacSpan, icacSpan, nocSpan));
        commissionerParams.operationalKeypair           = &ephemeralKey;
        commissionerParams.controllerRCAC               = rcacSpan;
        commissionerParams.controllerICAC               = icacSpan;
        commissionerParams.controllerNOC                = nocSpan;
        commissionerParams.permitMultiControllerFabrics = true;
    }

    // TODO: Initialize IPK epoch key in ExampleOperationalCredentials issuer rather than relying on DefaultIpkValue
    commissionerParams.operationalCredentialsDelegate = mCredIssuerCmds->GetCredentialIssuer();
    commissionerParams.controllerVendorId             = chip::VendorId::TestVendor1;

    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().SetupCommissioner(commissionerParams, *(commissioner.get())));

    if (identity.mName != kIdentityNull)
    {
        // Initialize Group Data, including IPK
        chip::FabricIndex fabricIndex = commissioner->GetFabricIndex();
        uint8_t compressed_fabric_id[sizeof(uint64_t)];
        chip::MutableByteSpan compressed_fabric_id_span(compressed_fabric_id);
        ReturnLogErrorOnFailure(commissioner->GetCompressedFabricIdBytes(compressed_fabric_id_span));

        ReturnLogErrorOnFailure(chip::GroupTesting::InitData(&sGroupDataProvider, fabricIndex, compressed_fabric_id_span));

        // Configure the default IPK for all fabrics used by CHIP-tool. The epoch
        // key is the same, but the derived keys will be different for each fabric.
        chip::ByteSpan defaultIpk = chip::GroupTesting::DefaultIpkValue::GetDefaultIpk();
        ReturnLogErrorOnFailure(
            chip::Credentials::SetSingleIpkEpochKey(&sGroupDataProvider, fabricIndex, defaultIpk, compressed_fabric_id_span));
    }

    mCommissioners[identity] = std::move(commissioner);

    return CHIP_NO_ERROR;
}

void CHIPCommand::RunQueuedCommand(intptr_t commandArg)
{
    auto * command = reinterpret_cast<CHIPCommand *>(commandArg);
    CHIP_ERROR err = command->EnsureCommissionerForIdentity(command->GetIdentity());
    if (err == CHIP_NO_ERROR)
    {
        err = command->RunCommand();
    }

    if (err != CHIP_NO_ERROR)
    {
        command->SetCommandExitStatus(err);
    }
}

#if !CONFIG_USE_SEPARATE_EVENTLOOP
static void OnResponseTimeout(chip::System::Layer *, void * appState)
{
    (reinterpret_cast<CHIPCommand *>(appState))->SetCommandExitStatus(CHIP_ERROR_TIMEOUT);
}
#endif // !CONFIG_USE_SEPARATE_EVENTLOOP

CHIP_ERROR CHIPCommand::StartWaiting(chip::System::Clock::Timeout duration)
{
#if CONFIG_USE_SEPARATE_EVENTLOOP
    // ServiceEvents() calls StartEventLoopTask(), which is paired with the StopEventLoopTask() below.
    if (!IsInteractive())
    {
        ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().ServiceEvents());
    }

    if (duration.count() == 0)
    {
        mCommandExitStatus = RunCommand();
    }
    else
    {
        {
            std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
            mWaitingForResponse = true;
        }

        chip::DeviceLayer::PlatformMgr().ScheduleWork(RunQueuedCommand, reinterpret_cast<intptr_t>(this));
        auto waitingUntil = std::chrono::system_clock::now() + std::chrono::duration_cast<std::chrono::seconds>(duration);
        {
            std::unique_lock<std::mutex> lk(cvWaitingForResponseMutex);
            if (!cvWaitingForResponse.wait_until(lk, waitingUntil, [this]() { return !this->mWaitingForResponse; }))
            {
                mCommandExitStatus = CHIP_ERROR_TIMEOUT;
            }
        }
    }
    if (!IsInteractive())
    {
        LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().StopEventLoopTask());
    }
#else
    chip::DeviceLayer::PlatformMgr().ScheduleWork(RunQueuedCommand, reinterpret_cast<intptr_t>(this));
    ReturnLogErrorOnFailure(chip::DeviceLayer::SystemLayer().StartTimer(duration, OnResponseTimeout, this));
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
#endif // CONFIG_USE_SEPARATE_EVENTLOOP

    return mCommandExitStatus;
}

void CHIPCommand::StopWaiting()
{
#if CONFIG_USE_SEPARATE_EVENTLOOP
    {
        std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
        mWaitingForResponse = false;
    }
    cvWaitingForResponse.notify_all();
#else  // CONFIG_USE_SEPARATE_EVENTLOOP
    LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().StopEventLoopTask());
#endif // CONFIG_USE_SEPARATE_EVENTLOOP
}

void CHIPCommand::ExecuteDeferredCleanups(intptr_t ignored)
{
    for (auto * cmd : sDeferredCleanups)
    {
        cmd->Cleanup();
    }
    sDeferredCleanups.clear();
}
