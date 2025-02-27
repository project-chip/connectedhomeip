/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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

#include <type_traits>

#include "ChipDeviceController-ScriptDevicePairingDelegate.h"
#include "ChipDeviceController-StorageDelegate.h"

#include "controller/python/chip/crypto/p256keypair.h"
#include "controller/python/chip/interaction_model/Delegate.h"

#include <app/icd/client/DefaultICDClientStorage.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/logging/CHIPLogging.h>

#include <controller/python/chip/commissioning/PlaceholderOperationalCredentialsIssuer.h>
#include <controller/python/chip/native/PyChipError.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/FileAttestationTrustStore.h>
#include <credentials/attestation_verifier/TestDACRevocationDelegateImpl.h>
#include <cstddef> // Added for size_t

using namespace chip;

using Py_GenerateNOCChainFunc         = void (*)(void * pyContext, const char * csrElements, const char * attestationSignature,
                                         const char * dac, const char * pai, const char * paa,
                                         Controller::OnNOCChainGeneration onNocChainGenerationFunc);
using Py_SetNodeIdForNextNOCRequest   = void (*)(void * pyContext, NodeId nodeId);
using Py_SetFabricIdForNextNOCRequest = void (*)(void * pyContext, FabricId fabricId);

namespace {
const chip::Credentials::AttestationTrustStore * GetTestFileAttestationTrustStore(const char * paaTrustStorePath)
{
    static chip::Credentials::FileAttestationTrustStore attestationTrustStore{ paaTrustStorePath };

    return &attestationTrustStore;
}

Credentials::DeviceAttestationRevocationDelegate * GetTestAttestationRevocationDelegate(const char * dacRevocationSetPath)
{
    VerifyOrReturnValue(dacRevocationSetPath != nullptr, nullptr);

    static Credentials::TestDACRevocationDelegateImpl testDacRevocationDelegate;
    testDacRevocationDelegate.SetDeviceAttestationRevocationSetPath(dacRevocationSetPath);
    return &testDacRevocationDelegate;
}

chip::Python::PlaceholderOperationalCredentialsIssuer sPlaceholderOperationalCredentialsIssuer;
} // namespace

namespace chip {
namespace Controller {
namespace Python {

class OperationalCredentialsAdapter : public OperationalCredentialsDelegate
{
public:
    OperationalCredentialsAdapter(uint32_t fabricCredentialsIndex) : mExampleOpCredsIssuer(fabricCredentialsIndex) {}

    CHIP_ERROR Initialize(PersistentStorageDelegate & storageDelegate) { return mExampleOpCredsIssuer.Initialize(storageDelegate); }

    CHIP_ERROR GenerateNOCChain(NodeId nodeId, FabricId fabricId, const CATValues & cats, const Crypto::P256PublicKey & pubKey,
                                MutableByteSpan & rcac, MutableByteSpan & icac, MutableByteSpan & noc)
    {
        return mExampleOpCredsIssuer.GenerateNOCChainAfterValidation(nodeId, fabricId, cats, pubKey, rcac, icac, noc);
    }

    void SetMaximallyLargeCertsUsed(bool enabled) { mExampleOpCredsIssuer.SetMaximallyLargeCertsUsed(enabled); }

    void SetCertificateValidityPeriod(uint32_t validity) { mExampleOpCredsIssuer.SetCertificateValidityPeriod(validity); }

private:
    CHIP_ERROR GenerateNOCChain(const ByteSpan & csrElements, const ByteSpan & csrNonce, const ByteSpan & attestationSignature,
                                const ByteSpan & attestationChallenge, const ByteSpan & DAC, const ByteSpan & PAI,
                                Callback::Callback<OnNOCChainGeneration> * onCompletion) override
    {
        return mExampleOpCredsIssuer.GenerateNOCChain(csrElements, csrNonce, attestationSignature, attestationChallenge, DAC, PAI,
                                                      onCompletion);
    }

    void SetNodeIdForNextNOCRequest(NodeId nodeId) override { mExampleOpCredsIssuer.SetNodeIdForNextNOCRequest(nodeId); }

    void SetFabricIdForNextNOCRequest(FabricId fabricId) override { mExampleOpCredsIssuer.SetFabricIdForNextNOCRequest(fabricId); }

    ExampleOperationalCredentialsIssuer mExampleOpCredsIssuer;
};

} // namespace Python
} // namespace Controller
} // namespace chip

extern chip::Credentials::GroupDataProviderImpl sGroupDataProvider;
extern chip::Controller::ScriptDevicePairingDelegate sPairingDelegate;
extern chip::app::DefaultICDClientStorage sICDClientStorage;

class TestCommissioner : public chip::Controller::AutoCommissioner
{
public:
    TestCommissioner() { Reset(); }
    ~TestCommissioner() {}

    CHIP_ERROR SetCommissioningParameters(const chip::Controller::CommissioningParameters & params) override
    {
        mIsWifi   = false;
        mIsThread = false;
        if (params.GetWiFiCredentials().HasValue())
        {
            mIsWifi = true;
        }
        else if (params.GetThreadOperationalDataset().HasValue())
        {
            mIsThread = true;
        }
        return chip::Controller::AutoCommissioner::SetCommissioningParameters(params);
    }
    CHIP_ERROR CommissioningStepFinished(CHIP_ERROR err,
                                         chip::Controller::CommissioningDelegate::CommissioningReport report) override
    {
        mTestCommissionerUsed = true;
        if (mFailOnReportAfterStage == report.stageCompleted)
        {
            return CHIP_ERROR_INTERNAL;
        }
        if (mSimulateFailureOnStage == report.stageCompleted)
        {
            // Pretend we received an error from the device during this stage
            err = CHIP_ERROR_INTERNAL;
        }
        if (mPrematureCompleteAfter == report.stageCompleted)
        {
            auto commissioner = chip::Controller::AutoCommissioner::GetCommissioner();
            auto proxy        = chip::Controller::AutoCommissioner::GetCommissioneeDeviceProxy();
            auto stage        = chip::Controller::CommissioningStage::kSendComplete;
            auto params       = chip::Controller::CommissioningParameters();
            commissioner->PerformCommissioningStep(proxy, stage, params, this, 0, GetCommandTimeout(proxy, stage));
            return CHIP_NO_ERROR;
        }

        if (report.stageCompleted == chip::Controller::CommissioningStage::kGenerateNOCChain)
        {
            if (report.Is<chip::Controller::NocChain>())
            {

                auto nocChain = report.Get<chip::Controller::NocChain>().rcac;

                // Convert RCAC to CHIP cert format to be deciphered by TLV later in python3
                std::vector<uint8_t> chipRcac(Credentials::kMaxCHIPCertLength);
                MutableByteSpan chipRcacSpan(chipRcac.data(), chipRcac.size());
                chip::Credentials::ConvertX509CertToChipCert(nocChain, chipRcacSpan);

                mCHIPRCACData.assign(chipRcacSpan.data(), chipRcacSpan.data() + chipRcacSpan.size());

                if (!mCHIPRCACData.empty())
                {
                    ChipLogProgress(Controller, "RCAC data converted and stored.");
                }
                else
                {
                    ChipLogError(Controller, "RCAC data is empty. No data to log.");
                }
            }
        }

        if (mPrematureCompleteAfter != chip::Controller::CommissioningStage::kError &&
            report.stageCompleted == chip::Controller::CommissioningStage::kSendComplete)
        {
            if (report.Is<chip::Controller::CommissioningErrorInfo>())
            {
                uint8_t code     = chip::to_underlying(report.Get<chip::Controller::CommissioningErrorInfo>().commissioningError);
                mCompletionError = chip::ChipError(chip::ChipError::SdkPart::kIMClusterStatus, code);
            }
            else
            {
                mCompletionError = err;
            }
        }
        if (report.stageCompleted == chip::Controller::CommissioningStage::kReadCommissioningInfo)
        {
            mReadCommissioningInfo = report.Get<chip::Controller::ReadCommissioningInfo>();
        }
        if (report.stageCompleted == chip::Controller::CommissioningStage::kConfigureTimeZone)
        {
            mNeedsDST = report.Get<chip::Controller::TimeZoneResponseInfo>().requiresDSTOffsets;
        }

        return chip::Controller::AutoCommissioner::CommissioningStepFinished(err, report);
    }
    // This will cause the COMMISSIONER to fail after the given stage. Setting this to kSecurePairing will cause the
    // StartCommissioning call to fail.
    bool SimulateFailAfter(chip::Controller::CommissioningStage stage)
    {
        ChipLogProgress(Controller, "setting simulate fail after stage %s", chip::Controller::StageToString(stage));
        if (!ValidStage(stage) && stage != chip::Controller::CommissioningStage::kError)
        {
            return false;
        }
        mSimulateFailureOnStage = stage;
        return true;
    }
    bool SimulateFailOnReport(chip::Controller::CommissioningStage stage)
    {
        if (!ValidStage(stage) && stage != chip::Controller::CommissioningStage::kError)
        {
            return false;
        }
        mFailOnReportAfterStage = stage;
        return true;
    }
    bool PrematureCompleteAfter(chip::Controller::CommissioningStage stage)
    {
        if (!ValidStage(stage) && stage != chip::Controller::CommissioningStage::kError)
        {
            return false;
        }
        mPrematureCompleteAfter = stage;
        return true;
    }
    bool CheckCallbacks()
    {
        bool successFailureOk;
        bool updatesOk;
        if (mFailOnReportAfterStage != chip::Controller::CommissioningStage::kError)
        {
            successFailureOk = mReceivedCommissioningFailureStage == mFailOnReportAfterStage && !mReceivedCommissioningSuccess;
            updatesOk        = StatusUpdatesOk(mFailOnReportAfterStage);
        }
        else if (mSimulateFailureOnStage != chip::Controller::CommissioningStage::kError)
        {
            successFailureOk = mReceivedCommissioningFailureStage == mSimulateFailureOnStage && !mReceivedCommissioningSuccess;
            updatesOk        = StatusUpdatesOk(mSimulateFailureOnStage);
        }
        else
        {
            successFailureOk = mReceivedCommissioningSuccess;
            updatesOk        = StatusUpdatesOk(chip::Controller::CommissioningStage::kError);
        }
        ChipLogProgress(Controller, "Checking callbacks: success failure ok? %d updates ok? %d", successFailureOk, updatesOk);
        return successFailureOk && updatesOk;
    }
    bool CheckPaseConnection(NodeId nodeId)
    {
        bool paseShouldBeOpen = false;
        if (chip::to_underlying(mFailOnReportAfterStage) >=
                chip::to_underlying(chip::Controller::CommissioningStage::kWiFiNetworkSetup) ||
            chip::to_underlying(mSimulateFailureOnStage) >=
                chip::to_underlying(chip::Controller::CommissioningStage::kWiFiNetworkSetup))
        {
            // Pase should be open still
            paseShouldBeOpen = true;
        }
        CommissioneeDeviceProxy * proxy;
        bool paseIsOpen =
            (chip::Controller::AutoCommissioner::GetCommissioner()->GetDeviceBeingCommissioned(nodeId, &proxy) == CHIP_NO_ERROR);
        ChipLogProgress(Controller, "Checking pase connection state: Should be open? %d is open? %d", paseShouldBeOpen, paseIsOpen);

        return paseShouldBeOpen == paseIsOpen;
    }
    bool CheckStageSuccessful(uint8_t stage) { return mReceivedStageSuccess[stage]; }
    void Reset()
    {
        mTestCommissionerUsed              = false;
        mReceivedCommissioningSuccess      = false;
        mReceivedCommissioningFailureStage = chip::Controller::CommissioningStage::kError;
        for (size_t i = 0; i < kNumCommissioningStages; ++i)
        {
            mReceivedStageSuccess[i] = false;
            mReceivedStageFailure[i] = false;
        }
        mSimulateFailureOnStage = chip::Controller::CommissioningStage::kError;
        mFailOnReportAfterStage = chip::Controller::CommissioningStage::kError;
        mPrematureCompleteAfter = chip::Controller::CommissioningStage::kError;
        mReadCommissioningInfo  = chip::Controller::ReadCommissioningInfo();
        mNeedsDST               = false;
        mCompletionError        = CHIP_NO_ERROR;
    }
    bool GetTestCommissionerUsed() { return mTestCommissionerUsed; }

    void OnCommissioningSuccess(chip::PeerId peerId) { mReceivedCommissioningSuccess = true; }

    void OnCommissioningFailure(chip::PeerId peerId, CHIP_ERROR error, chip::Controller::CommissioningStage stageFailed,
                                chip::Optional<chip::Credentials::AttestationVerificationResult> additionalErrorInfo)
    {
        mReceivedCommissioningFailureStage = stageFailed;
    }
    void OnCommissioningStatusUpdate(chip::PeerId peerId, chip::Controller::CommissioningStage stageCompleted, CHIP_ERROR error)
    {
        if (error == CHIP_NO_ERROR)
        {
            mReceivedStageSuccess[chip::to_underlying(stageCompleted)] = true;
        }
        else
        {
            mReceivedStageFailure[chip::to_underlying(stageCompleted)] = true;
        }
        if (stageCompleted == chip::Controller::CommissioningStage::kCleanup &&
            mPrematureCompleteAfter != chip::Controller::CommissioningStage::kError)
        {
            // We need to manually clean up the proxy here because we're doing bad things in the name of testing
            ChipLogProgress(Controller, "Cleaning up dangling proxies");
            auto commissioner = chip::Controller::AutoCommissioner::GetCommissioner();
            auto proxy        = chip::Controller::AutoCommissioner::GetCommissioneeDeviceProxy();
            if (proxy != nullptr)
            {
                commissioner->StopPairing(proxy->GetDeviceId());
            }
        }
    }

    CHIP_ERROR GetCompletionError() { return mCompletionError; }

    const std::vector<uint8_t> & GetCHIPRCACData() const { return mCHIPRCACData; }

private:
    std::vector<uint8_t> mCHIPRCACData;
    static constexpr uint8_t kNumCommissioningStages = chip::to_underlying(chip::Controller::CommissioningStage::kCleanup) + 1;
    chip::Controller::CommissioningStage mSimulateFailureOnStage            = chip::Controller::CommissioningStage::kError;
    chip::Controller::CommissioningStage mFailOnReportAfterStage            = chip::Controller::CommissioningStage::kError;
    chip::Controller::CommissioningStage mPrematureCompleteAfter            = chip::Controller::CommissioningStage::kError;
    bool mTestCommissionerUsed                                              = false;
    bool mReceivedCommissioningSuccess                                      = false;
    chip::Controller::CommissioningStage mReceivedCommissioningFailureStage = chip::Controller::CommissioningStage::kError;
    bool mReceivedStageSuccess[kNumCommissioningStages];
    bool mReceivedStageFailure[kNumCommissioningStages];
    bool mIsWifi                = false;
    bool mIsThread              = false;
    CHIP_ERROR mCompletionError = CHIP_NO_ERROR;
    // Contains information about whether the device needs time sync
    chip::Controller::ReadCommissioningInfo mReadCommissioningInfo;
    bool mNeedsDST = false;
    bool ValidStage(chip::Controller::CommissioningStage stage)
    {
        switch (stage)
        {
        case chip::Controller::CommissioningStage::kWiFiNetworkEnable:
        case chip::Controller::CommissioningStage::kFailsafeBeforeWiFiEnable:
        case chip::Controller::CommissioningStage::kWiFiNetworkSetup:
            return mIsWifi;
        case chip::Controller::CommissioningStage::kThreadNetworkEnable:
        case chip::Controller::CommissioningStage::kFailsafeBeforeThreadEnable:
        case chip::Controller::CommissioningStage::kThreadNetworkSetup:
            return mIsThread;
        case chip::Controller::CommissioningStage::kConfigureUTCTime:
            return mReadCommissioningInfo.requiresUTC;
        case chip::Controller::CommissioningStage::kConfigureTimeZone:
            return mReadCommissioningInfo.requiresTimeZone && mParams.GetTimeZone().HasValue();
        case chip::Controller::CommissioningStage::kConfigureTrustedTimeSource:
            return mReadCommissioningInfo.requiresTrustedTimeSource && mParams.GetTrustedTimeSource().HasValue();
        case chip::Controller::CommissioningStage::kConfigureDefaultNTP:
            return mReadCommissioningInfo.requiresDefaultNTP && mParams.GetDefaultNTP().HasValue();
        case chip::Controller::CommissioningStage::kConfigureDSTOffset:
            return mNeedsDST && mParams.GetDSTOffsets().HasValue();
        case chip::Controller::CommissioningStage::kError:
        case chip::Controller::CommissioningStage::kSecurePairing:
        // "not valid" because attestation verification always fails after entering revocation check step
        case chip::Controller::CommissioningStage::kAttestationVerification:
            return false;
        default:
            return true;
        }
    }
    bool StatusUpdatesOk(chip::Controller::CommissioningStage failedStage)
    {
        // Because we're just simulating a failure here, we will have received a success callback even for the failed stage.
        for (uint8_t i = 0; i < kNumCommissioningStages; ++i)
        {
            if (mReceivedStageFailure[i])
            {
                return false;
            }
            if (!ValidStage(static_cast<chip::Controller::CommissioningStage>(i)))
            {
                continue;
            }
            // Anything above our current stage we won't have received a callback for. We also expect that the "failed" stage will
            // have a success callback because we're just faking the failure here and overwriting error.
            if (i == chip::to_underlying(failedStage))
            {
                break;
            }
        }
        return true;
    }
};
TestCommissioner sTestCommissioner;

extern "C" {
struct OpCredsContext
{
    Platform::UniquePtr<Controller::Python::OperationalCredentialsAdapter> mAdapter;
    void * mPyContext;
};

void * pychip_OpCreds_InitializeDelegate(void * pyContext, uint32_t fabricCredentialsIndex,
                                         Controller::Python::StorageAdapter * storageAdapter)
{
    auto context      = Platform::MakeUnique<OpCredsContext>();
    context->mAdapter = Platform::MakeUnique<Controller::Python::OperationalCredentialsAdapter>(fabricCredentialsIndex);

    if (context->mAdapter->Initialize(*storageAdapter) != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    return context.release();
}

void pychip_OnCommissioningSuccess(PeerId peerId)
{
    sTestCommissioner.OnCommissioningSuccess(peerId);
}

void pychip_OnCommissioningFailure(chip::PeerId peerId, CHIP_ERROR error, chip::Controller::CommissioningStage stageFailed,
                                   chip::Optional<chip::Credentials::AttestationVerificationResult> additionalErrorInfo)
{
    sTestCommissioner.OnCommissioningFailure(peerId, error, stageFailed, additionalErrorInfo);
}
void pychip_OnCommissioningStatusUpdate(chip::PeerId peerId, chip::Controller::CommissioningStage stageCompleted, CHIP_ERROR err)
{
    return sTestCommissioner.OnCommissioningStatusUpdate(peerId, stageCompleted, err);
}

/**
 * Allocates a controller that does not use auto-commisioning.
 *
 * TODO(#25214): Need clean up API
 *
 */
PyChipError pychip_OpCreds_AllocateControllerForPythonCommissioningFLow(
    chip::Controller::DeviceCommissioner ** outDevCtrl, chip::Controller::ScriptDevicePairingDelegate ** outPairingDelegate,
    chip::python::pychip_P256Keypair * operationalKey, uint8_t * noc, uint32_t nocLen, uint8_t * icac, uint32_t icacLen,
    uint8_t * rcac, uint32_t rcacLen, const uint8_t * ipk, uint32_t ipkLen, chip::VendorId adminVendorId,
    bool enableServerInteractions)
{
    VerifyOrReturnError(nocLen <= Controller::kMaxCHIPDERCertLength, ToPyChipError(CHIP_ERROR_NO_MEMORY));
    VerifyOrReturnError(icacLen <= Controller::kMaxCHIPDERCertLength, ToPyChipError(CHIP_ERROR_NO_MEMORY));
    VerifyOrReturnError(rcacLen <= Controller::kMaxCHIPDERCertLength, ToPyChipError(CHIP_ERROR_NO_MEMORY));

    ChipLogDetail(Controller, "Creating New Device Controller");

    auto pairingDelegate = std::make_unique<chip::Controller::ScriptDevicePairingDelegate>();
    VerifyOrReturnError(pairingDelegate != nullptr, ToPyChipError(CHIP_ERROR_NO_MEMORY));
    auto devCtrl = std::make_unique<chip::Controller::DeviceCommissioner>();
    VerifyOrReturnError(devCtrl != nullptr, ToPyChipError(CHIP_ERROR_NO_MEMORY));

    Controller::SetupParams initParams;
    initParams.pairingDelegate                      = pairingDelegate.get();
    initParams.operationalCredentialsDelegate       = &sPlaceholderOperationalCredentialsIssuer;
    initParams.operationalKeypair                   = operationalKey;
    initParams.controllerRCAC                       = ByteSpan(rcac, rcacLen);
    initParams.controllerICAC                       = ByteSpan(icac, icacLen);
    initParams.controllerNOC                        = ByteSpan(noc, nocLen);
    initParams.enableServerInteractions             = enableServerInteractions;
    initParams.controllerVendorId                   = adminVendorId;
    initParams.permitMultiControllerFabrics         = true;
    initParams.hasExternallyOwnedOperationalKeypair = true;

    CHIP_ERROR err = Controller::DeviceControllerFactory::GetInstance().SetupCommissioner(initParams, *devCtrl);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    // Setup IPK in Group Data Provider for controller after Commissioner init which sets-up the fabric table entry
    uint8_t compressedFabricId[sizeof(uint64_t)] = { 0 };
    chip::MutableByteSpan compressedFabricIdSpan(compressedFabricId);

    err = devCtrl->GetCompressedFabricIdBytes(compressedFabricIdSpan);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    ChipLogProgress(Support, "Setting up group data for Fabric Index %u with Compressed Fabric ID:",
                    static_cast<unsigned>(devCtrl->GetFabricIndex()));
    ChipLogByteSpan(Support, compressedFabricIdSpan);

    chip::ByteSpan fabricIpk =
        (ipk == nullptr) ? chip::GroupTesting::DefaultIpkValue::GetDefaultIpk() : chip::ByteSpan(ipk, ipkLen);
    err =
        chip::Credentials::SetSingleIpkEpochKey(&sGroupDataProvider, devCtrl->GetFabricIndex(), fabricIpk, compressedFabricIdSpan);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    *outDevCtrl         = devCtrl.release();
    *outPairingDelegate = pairingDelegate.release();

    return ToPyChipError(CHIP_NO_ERROR);
}

// TODO(#25214): Need clean up API
PyChipError pychip_OpCreds_AllocateController(OpCredsContext * context, chip::Controller::DeviceCommissioner ** outDevCtrl,
                                              chip::Controller::ScriptDevicePairingDelegate ** outPairingDelegate,
                                              FabricId fabricId, chip::NodeId nodeId, chip::VendorId adminVendorId,
                                              const char * paaTrustStorePath, bool useTestCommissioner,
                                              bool enableServerInteractions, CASEAuthTag * caseAuthTags, uint32_t caseAuthTagLen,
                                              chip::python::pychip_P256Keypair * operationalKey)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(Controller, "Creating New Device Controller");

    VerifyOrReturnError(context != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    auto pairingDelegate = std::make_unique<chip::Controller::ScriptDevicePairingDelegate>();
    VerifyOrReturnError(pairingDelegate != nullptr, ToPyChipError(CHIP_ERROR_NO_MEMORY));
    auto devCtrl = std::make_unique<chip::Controller::DeviceCommissioner>();
    VerifyOrReturnError(devCtrl != nullptr, ToPyChipError(CHIP_ERROR_NO_MEMORY));

    if (paaTrustStorePath == nullptr)
    {
        paaTrustStorePath = "./credentials/development/paa-root-certs";
    }

    ChipLogProgress(Support, "Using device attestation PAA trust store path %s.", paaTrustStorePath);

    // Initialize device attestation verifier
    const chip::Credentials::AttestationTrustStore * testingRootStore = GetTestFileAttestationTrustStore(paaTrustStorePath);
    chip::Credentials::DeviceAttestationVerifier * dacVerifier        = chip::Credentials::GetDefaultDACVerifier(testingRootStore);
    SetDeviceAttestationVerifier(dacVerifier);

    chip::Crypto::P256Keypair ephemeralKey;
    chip::Crypto::P256Keypair * controllerKeyPair;

    if (operationalKey == nullptr)
    {
        err = ephemeralKey.Initialize(chip::Crypto::ECPKeyTarget::ECDSA);
        VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));
        controllerKeyPair = &ephemeralKey;
    }
    else
    {
        controllerKeyPair = operationalKey;
    }

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    VerifyOrReturnError(noc.Alloc(Controller::kMaxCHIPDERCertLength), ToPyChipError(CHIP_ERROR_NO_MEMORY));
    MutableByteSpan nocSpan(noc.Get(), Controller::kMaxCHIPDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    VerifyOrReturnError(icac.Alloc(Controller::kMaxCHIPDERCertLength), ToPyChipError(CHIP_ERROR_NO_MEMORY));
    MutableByteSpan icacSpan(icac.Get(), Controller::kMaxCHIPDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;
    VerifyOrReturnError(rcac.Alloc(Controller::kMaxCHIPDERCertLength), ToPyChipError(CHIP_ERROR_NO_MEMORY));
    MutableByteSpan rcacSpan(rcac.Get(), Controller::kMaxCHIPDERCertLength);

    CATValues catValues;

    if (caseAuthTagLen > kMaxSubjectCATAttributeCount)
    {
        ChipLogError(Controller, "Too many of CASE Tags (%u) exceeds kMaxSubjectCATAttributeCount",
                     static_cast<unsigned>(caseAuthTagLen));
        return ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT);
    }

    memcpy(catValues.values.data(), caseAuthTags, caseAuthTagLen * sizeof(CASEAuthTag));

    err =
        context->mAdapter->GenerateNOCChain(nodeId, fabricId, catValues, controllerKeyPair->Pubkey(), rcacSpan, icacSpan, nocSpan);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    Controller::SetupParams initParams;
    initParams.pairingDelegate                      = pairingDelegate.get();
    initParams.operationalCredentialsDelegate       = context->mAdapter.get();
    initParams.operationalKeypair                   = controllerKeyPair;
    initParams.controllerRCAC                       = rcacSpan;
    initParams.controllerICAC                       = icacSpan;
    initParams.controllerNOC                        = nocSpan;
    initParams.enableServerInteractions             = enableServerInteractions;
    initParams.controllerVendorId                   = adminVendorId;
    initParams.permitMultiControllerFabrics         = true;
    initParams.hasExternallyOwnedOperationalKeypair = operationalKey != nullptr;
    initParams.deviceAttestationVerifier            = dacVerifier;

    if (useTestCommissioner)
    {
        initParams.defaultCommissioner = &sTestCommissioner;
        pairingDelegate->SetCommissioningSuccessCallback(pychip_OnCommissioningSuccess);
        pairingDelegate->SetCommissioningFailureCallback(pychip_OnCommissioningFailure);
        pairingDelegate->SetCommissioningStatusUpdateCallback(pychip_OnCommissioningStatusUpdate);
    }

    err = Controller::DeviceControllerFactory::GetInstance().SetupCommissioner(initParams, *devCtrl);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    // Setup IPK in Group Data Provider for controller after Commissioner init which sets-up the fabric table entry
    uint8_t compressedFabricId[sizeof(uint64_t)] = { 0 };
    chip::MutableByteSpan compressedFabricIdSpan(compressedFabricId);

    err = devCtrl->GetCompressedFabricIdBytes(compressedFabricIdSpan);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    ChipLogProgress(Support, "Setting up group data for Fabric Index %u with Compressed Fabric ID:",
                    static_cast<unsigned>(devCtrl->GetFabricIndex()));
    ChipLogByteSpan(Support, compressedFabricIdSpan);

    chip::ByteSpan defaultIpk = chip::GroupTesting::DefaultIpkValue::GetDefaultIpk();
    err =
        chip::Credentials::SetSingleIpkEpochKey(&sGroupDataProvider, devCtrl->GetFabricIndex(), defaultIpk, compressedFabricIdSpan);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    sICDClientStorage.UpdateFabricList(devCtrl->GetFabricIndex());
    pairingDelegate->SetFabricIndex(devCtrl->GetFabricIndex());

    *outDevCtrl         = devCtrl.release();
    *outPairingDelegate = pairingDelegate.release();

    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_OpCreds_InitGroupTestingData(chip::Controller::DeviceCommissioner * devCtrl)
{
    VerifyOrReturnError(devCtrl != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    uint8_t compressedFabricId[sizeof(uint64_t)] = { 0 };
    chip::MutableByteSpan compressedFabricIdSpan(compressedFabricId);

    CHIP_ERROR err = devCtrl->GetCompressedFabricIdBytes(compressedFabricIdSpan);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    err = chip::GroupTesting::InitData(&sGroupDataProvider, devCtrl->GetFabricIndex(), compressedFabricIdSpan);

    return ToPyChipError(err);
}

PyChipError pychip_OpCreds_SetMaximallyLargeCertsUsed(OpCredsContext * context, bool enabled)
{
    VerifyOrReturnError(context != nullptr && context->mAdapter != nullptr, ToPyChipError(CHIP_ERROR_INCORRECT_STATE));

    context->mAdapter->SetMaximallyLargeCertsUsed(enabled);

    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_OpCreds_SetCertificateValidityPeriod(OpCredsContext * context, uint32_t validity)
{
    VerifyOrReturnError(context != nullptr && context->mAdapter != nullptr, ToPyChipError(CHIP_ERROR_INCORRECT_STATE));

    context->mAdapter->SetCertificateValidityPeriod(validity);

    return ToPyChipError(CHIP_NO_ERROR);
}

void pychip_OpCreds_FreeDelegate(OpCredsContext * context)
{
    Platform::Delete(context);
}

PyChipError pychip_DeviceController_DeleteDeviceController(chip::Controller::DeviceCommissioner * devCtrl,
                                                           chip::Controller::ScriptDevicePairingDelegate * pairingDelegate)
{
    if (devCtrl != nullptr)
    {
        devCtrl->Shutdown();
        delete devCtrl;
    }

    if (pairingDelegate != nullptr)
    {
        delete pairingDelegate;
    }

    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_DeviceController_SetIpk(chip::Controller::DeviceCommissioner * devCtrl, const uint8_t * ipk, size_t ipkLen)
{
    VerifyOrReturnError(ipk != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    uint8_t compressedFabricId[sizeof(uint64_t)] = { 0 };
    chip::MutableByteSpan compressedFabricIdSpan(compressedFabricId);

    CHIP_ERROR err = devCtrl->GetCompressedFabricIdBytes(compressedFabricIdSpan);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ToPyChipError(err));

    err = chip::Credentials::SetSingleIpkEpochKey(&sGroupDataProvider, devCtrl->GetFabricIndex(), ByteSpan(ipk, ipkLen),
                                                  compressedFabricIdSpan);

    return ToPyChipError(err);
}

bool pychip_TestCommissionerUsed()
{
    return sTestCommissioner.GetTestCommissionerUsed();
}

bool pychip_TestCommissioningCallbacks()
{
    return sTestCommissioner.CheckCallbacks();
}

bool pychip_TestCommissioningStageSuccessful(uint8_t stage)
{
    return sTestCommissioner.CheckStageSuccessful(stage);
}

bool pychip_TestPaseConnection(NodeId nodeId)
{
    return sTestCommissioner.CheckPaseConnection(nodeId);
}

void pychip_ResetCommissioningTests()
{
    sTestCommissioner.Reset();
}

// Returns True if this is a valid test, false otherwise
bool pychip_SetTestCommissionerSimulateFailureOnStage(uint8_t failStage)
{
    return sTestCommissioner.SimulateFailAfter(static_cast<chip::Controller::CommissioningStage>(failStage));
}
bool pychip_SetTestCommissionerSimulateFailureOnReport(uint8_t failStage)
{
    return sTestCommissioner.SimulateFailOnReport(static_cast<chip::Controller::CommissioningStage>(failStage));
}
bool pychip_SetTestCommissionerPrematureCompleteAfter(uint8_t stage)
{
    return sTestCommissioner.PrematureCompleteAfter(static_cast<chip::Controller::CommissioningStage>(stage));
}

PyChipError pychip_GetCompletionError()
{
    return ToPyChipError(sTestCommissioner.GetCompletionError());
}

PyChipError pychip_DeviceController_SetDACRevocationSetPath(const char * dacRevocationSetPath)
{
    Credentials::DeviceAttestationRevocationDelegate * dacRevocationDelegate =
        GetTestAttestationRevocationDelegate(dacRevocationSetPath);
    VerifyOrReturnError(dacRevocationDelegate != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    Credentials::DeviceAttestationVerifier * dacVerifier = Credentials::GetDeviceAttestationVerifier();
    VerifyOrReturnError(dacVerifier != nullptr, ToPyChipError(CHIP_ERROR_INCORRECT_STATE));

    return ToPyChipError(dacVerifier->SetRevocationDelegate(dacRevocationDelegate));
}

extern "C" {
// Function to get the RCAC data from the sTestCommissioner
void pychip_GetCommissioningRCACData(uint8_t * rcacDataPtr, size_t * rcacSize, size_t bufferSize)
{
    // Attempting to get Python RCAC data in C++
    const auto & rcacData = sTestCommissioner.GetCHIPRCACData();

    if (rcacData.empty())
    {
        ChipLogError(Controller, "RCAC data is empty in C++. Nothing to return.");
        *rcacSize = 0;
        return;
    }

    // Check if the provided buffer is too small
    if (bufferSize < rcacData.size())
    {
        ChipLogError(Controller, "Allocated buffer size (%lu) is too small. RCAC data was size (%lu). Returning zero-sized buffer.",
                     static_cast<unsigned long>(bufferSize), static_cast<unsigned long>(rcacData.size()));
        *rcacSize = 0;
        return;
    }

    // Copy the data from C++ to Python's allocated memory
    std::memcpy(rcacDataPtr, rcacData.data(), rcacData.size());
    *rcacSize = rcacData.size();
}
}
} // extern "C"
