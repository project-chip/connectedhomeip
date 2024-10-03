/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "PairingManager.h"

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <commands/common/CHIPCommand.h>
#include <device_manager/DeviceSynchronization.h>
#include <lib/support/logging/CHIPLogging.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

#if defined(PW_RPC_ENABLED)
#include <rpc/RpcClient.h>
#endif

using namespace ::chip;
using namespace ::chip::Controller;

namespace {

CHIP_ERROR GetPayload(const char * setUpCode, SetupPayload & payload)
{
    VerifyOrReturnValue(setUpCode, CHIP_ERROR_INVALID_ARGUMENT);
    bool isQRCode = strncmp(setUpCode, kQRCodePrefix, strlen(kQRCodePrefix)) == 0;
    if (isQRCode)
    {
        ReturnErrorOnFailure(QRCodeSetupPayloadParser(setUpCode).populatePayload(payload));
        VerifyOrReturnError(payload.isValidQRCodePayload(), CHIP_ERROR_INVALID_ARGUMENT);
    }
    else
    {
        ReturnErrorOnFailure(ManualSetupPayloadParser(setUpCode).populatePayload(payload));
        VerifyOrReturnError(payload.isValidManualCode(), CHIP_ERROR_INVALID_ARGUMENT);
    }

    return CHIP_NO_ERROR;
}

bool ParseAddressWithInterface(const char * addressString, Inet::IPAddress & address, Inet::InterfaceId & interfaceId)
{
    struct addrinfo hints;
    struct addrinfo * result;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    ret               = getaddrinfo(addressString, nullptr, &hints, &result);
    if (ret < 0)
    {
        ChipLogError(NotSpecified, "Invalid address: %s", addressString);
        return false;
    }

    if (result->ai_family == AF_INET6)
    {
        struct sockaddr_in6 * addr = reinterpret_cast<struct sockaddr_in6 *>(result->ai_addr);
        address                    = Inet::IPAddress::FromSockAddr(*addr);
        interfaceId                = Inet::InterfaceId(addr->sin6_scope_id);
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (result->ai_family == AF_INET)
    {
        address     = Inet::IPAddress::FromSockAddr(*reinterpret_cast<struct sockaddr_in *>(result->ai_addr));
        interfaceId = Inet::InterfaceId::Null();
    }
#endif // INET_CONFIG_ENABLE_IPV4
    else
    {
        ChipLogError(NotSpecified, "Unsupported address: %s", addressString);
        freeaddrinfo(result);
        return false;
    }

    freeaddrinfo(result);
    return true;
}

} // namespace

PairingManager::PairingManager() :
    mOnOpenCommissioningWindowCallback(OnOpenCommissioningWindowResponse, this),
    mOnOpenCommissioningWindowVerifierCallback(OnOpenCommissioningWindowVerifierResponse, this),
    mCurrentFabricRemoveCallback(OnCurrentFabricRemove, this)
{}

CHIP_ERROR PairingManager::Init(Controller::DeviceCommissioner * commissioner, CredentialIssuerCommands * credIssuerCmds)
{
    VerifyOrReturnError(commissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(credIssuerCmds != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mCommissioner   = commissioner;
    mCredIssuerCmds = credIssuerCmds;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PairingManager::OpenCommissioningWindow(NodeId nodeId, EndpointId endpointId, uint16_t commissioningTimeoutSec,
                                                   uint32_t iterations, uint16_t discriminator, const ByteSpan & salt,
                                                   const ByteSpan & verifier)
{
    if (mCommissioner == nullptr)
    {
        ChipLogError(NotSpecified, "Commissioner is null, cannot open commissioning window");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Check if a window is already open
    if (mWindowOpener != nullptr)
    {
        ChipLogError(NotSpecified, "A commissioning window is already open");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto params                        = Platform::MakeUnique<CommissioningWindowParams>();
    params->nodeId                     = nodeId;
    params->endpointId                 = endpointId;
    params->commissioningWindowTimeout = commissioningTimeoutSec;
    params->iteration                  = iterations;
    params->discriminator              = discriminator;

    if (!salt.empty())
    {
        if (salt.size() > sizeof(params->saltBuffer))
        {
            ChipLogError(NotSpecified, "Salt size exceeds buffer capacity");
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        memcpy(params->saltBuffer, salt.data(), salt.size());
        params->salt = ByteSpan(params->saltBuffer, salt.size());
    }

    if (!verifier.empty())
    {
        if (verifier.size() > sizeof(params->verifierBuffer))
        {
            ChipLogError(NotSpecified, "Verifier size exceeds buffer capacity");
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        memcpy(params->verifierBuffer, verifier.data(), verifier.size());
        params->verifier = ByteSpan(params->verifierBuffer, verifier.size());
    }

    // Schedule work on the Matter thread
    return DeviceLayer::PlatformMgr().ScheduleWork(OnOpenCommissioningWindow, reinterpret_cast<intptr_t>(params.release()));
}

void PairingManager::OnOpenCommissioningWindow(intptr_t context)
{
    Platform::UniquePtr<CommissioningWindowParams> params(reinterpret_cast<CommissioningWindowParams *>(context));
    PairingManager & self = PairingManager::Instance();

    if (self.mCommissioner == nullptr)
    {
        ChipLogError(NotSpecified, "Commissioner is null, cannot open commissioning window");
        return;
    }

    self.mWindowOpener = Platform::MakeUnique<Controller::CommissioningWindowOpener>(self.mCommissioner);

    if (!params->verifier.empty())
    {
        if (params->salt.empty())
        {
            ChipLogError(NotSpecified, "Salt is required when verifier is set");
            self.mWindowOpener.reset();
            return;
        }

        CHIP_ERROR err =
            self.mWindowOpener->OpenCommissioningWindow(Controller::CommissioningWindowVerifierParams()
                                                            .SetNodeId(params->nodeId)
                                                            .SetEndpointId(params->endpointId)
                                                            .SetTimeout(params->commissioningWindowTimeout)
                                                            .SetIteration(params->iteration)
                                                            .SetDiscriminator(params->discriminator)
                                                            .SetVerifier(params->verifier)
                                                            .SetSalt(params->salt)
                                                            .SetCallback(&self.mOnOpenCommissioningWindowVerifierCallback));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed to open commissioning window with verifier: %s", ErrorStr(err));
            self.mWindowOpener.reset();
        }
    }
    else
    {
        SetupPayload ignored;
        CHIP_ERROR err = self.mWindowOpener->OpenCommissioningWindow(Controller::CommissioningWindowPasscodeParams()
                                                                         .SetNodeId(params->nodeId)
                                                                         .SetEndpointId(params->endpointId)
                                                                         .SetTimeout(params->commissioningWindowTimeout)
                                                                         .SetIteration(params->iteration)
                                                                         .SetDiscriminator(params->discriminator)
                                                                         .SetSetupPIN(NullOptional)
                                                                         .SetSalt(NullOptional)
                                                                         .SetCallback(&self.mOnOpenCommissioningWindowCallback),
                                                                     ignored);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed to open commissioning window with passcode: %s", ErrorStr(err));
            self.mWindowOpener.reset();
        }
    }
}

void PairingManager::OnOpenCommissioningWindowResponse(void * context, NodeId remoteId, CHIP_ERROR err, SetupPayload payload)
{
    VerifyOrDie(context != nullptr);
    PairingManager * self = static_cast<PairingManager *>(context);
    if (self->mCommissioningWindowDelegate)
    {
        self->mCommissioningWindowDelegate->OnCommissioningWindowOpened(remoteId, err, payload);
        self->SetOpenCommissioningWindowDelegate(nullptr);
    }

    OnOpenCommissioningWindowVerifierResponse(context, remoteId, err);
}

void PairingManager::OnOpenCommissioningWindowVerifierResponse(void * context, NodeId remoteId, CHIP_ERROR err)
{
    VerifyOrDie(context != nullptr);
    PairingManager * self = static_cast<PairingManager *>(context);
    LogErrorOnFailure(err);

    // Reset the window opener once the window operation is complete
    self->mWindowOpener.reset();
}

void PairingManager::OnStatusUpdate(DevicePairingDelegate::Status status)
{
    switch (status)
    {
    case DevicePairingDelegate::Status::SecurePairingSuccess:
        ChipLogProgress(NotSpecified, "CASE establishment successful");
        break;
    case DevicePairingDelegate::Status::SecurePairingFailed:
        ChipLogError(NotSpecified, "Secure Pairing Failed");
        break;
    }
}

void PairingManager::OnPairingComplete(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "PASE establishment successful");
    }
    else
    {
        ChipLogProgress(NotSpecified, "Pairing Failure: %s", ErrorStr(err));
    }
}

void PairingManager::OnPairingDeleted(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "Pairing Deleted Success");
    }
    else
    {
        ChipLogProgress(NotSpecified, "Pairing Deleted Failure: %s", ErrorStr(err));
    }
}

void PairingManager::OnCommissioningComplete(NodeId nodeId, CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        // print to console
        fprintf(stderr, "New device with Node ID: " ChipLogFormatX64 "has been successfully added.\n", ChipLogValueX64(nodeId));

        // mCommissioner has a lifetime that is the entire life of the application itself
        // so it is safe to provide to StartDeviceSynchronization.
        DeviceSynchronizer::Instance().StartDeviceSynchronization(mCommissioner, nodeId, mDeviceIsICD);
    }
    else
    {
        // When ICD device commissioning fails, the ICDClientInfo stored in OnICDRegistrationComplete needs to be removed.
        if (mDeviceIsICD)
        {
            CHIP_ERROR deleteEntryError =
                CHIPCommand::sICDClientStorage.DeleteEntry(ScopedNodeId(nodeId, mCommissioner->GetFabricIndex()));
            if (deleteEntryError != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified, "Failed to delete ICD entry: %s", ErrorStr(err));
            }
        }
        ChipLogProgress(NotSpecified, "Device commissioning Failure: %s", ErrorStr(err));
    }

    if (mCommissioningDelegate)
    {
        mCommissioningDelegate->OnCommissioningComplete(nodeId, err);
        SetCommissioningDelegate(nullptr);
    }
}

void PairingManager::OnReadCommissioningInfo(const Controller::ReadCommissioningInfo & info)
{
    ChipLogProgress(AppServer, "OnReadCommissioningInfo - vendorId=0x%04X productId=0x%04X", info.basic.vendorId,
                    info.basic.productId);

    // The string in CharSpan received from the device is not null-terminated, we use std::string here for coping and
    // appending a null-terminator at the end of the string.
    std::string userActiveModeTriggerInstruction;

    // Note: the callback doesn't own the buffer, should make a copy if it will be used it later.
    if (info.icd.userActiveModeTriggerInstruction.size() != 0)
    {
        userActiveModeTriggerInstruction =
            std::string(info.icd.userActiveModeTriggerInstruction.data(), info.icd.userActiveModeTriggerInstruction.size());
    }

    if (info.icd.userActiveModeTriggerHint.HasAny())
    {
        ChipLogProgress(AppServer, "OnReadCommissioningInfo - LIT UserActiveModeTriggerHint=0x%08x",
                        info.icd.userActiveModeTriggerHint.Raw());
        ChipLogProgress(AppServer, "OnReadCommissioningInfo - LIT UserActiveModeTriggerInstruction=%s",
                        userActiveModeTriggerInstruction.c_str());
    }
    ChipLogProgress(AppServer, "OnReadCommissioningInfo ICD - IdleModeDuration=%u activeModeDuration=%u activeModeThreshold=%u",
                    info.icd.idleModeDuration, info.icd.activeModeDuration, info.icd.activeModeThreshold);
}

void PairingManager::OnICDRegistrationComplete(ScopedNodeId nodeId, uint32_t icdCounter)
{
    char icdSymmetricKeyHex[Crypto::kAES_CCM128_Key_Length * 2 + 1];

    Encoding::BytesToHex(mICDSymmetricKey.Value().data(), mICDSymmetricKey.Value().size(), icdSymmetricKeyHex,
                         sizeof(icdSymmetricKeyHex), Encoding::HexFlags::kNullTerminate);

    app::ICDClientInfo clientInfo;
    clientInfo.peer_node         = nodeId;
    clientInfo.monitored_subject = mICDMonitoredSubject.Value();
    clientInfo.start_icd_counter = icdCounter;

    CHIP_ERROR err = CHIPCommand::sICDClientStorage.SetKey(clientInfo, mICDSymmetricKey.Value());
    if (err == CHIP_NO_ERROR)
    {
        err = CHIPCommand::sICDClientStorage.StoreEntry(clientInfo);
    }

    if (err != CHIP_NO_ERROR)
    {
        CHIPCommand::sICDClientStorage.RemoveKey(clientInfo);
        ChipLogError(NotSpecified, "Failed to persist symmetric key for " ChipLogFormatX64 ": %s",
                     ChipLogValueX64(nodeId.GetNodeId()), err.AsString());
        return;
    }

    mDeviceIsICD = true;

    ChipLogProgress(NotSpecified, "Saved ICD Symmetric key for " ChipLogFormatX64, ChipLogValueX64(nodeId.GetNodeId()));
    ChipLogProgress(NotSpecified,
                    "ICD Registration Complete for device " ChipLogFormatX64 " / Check-In NodeID: " ChipLogFormatX64
                    " / Monitored Subject: " ChipLogFormatX64 " / Symmetric Key: %s / ICDCounter %u",
                    ChipLogValueX64(nodeId.GetNodeId()), ChipLogValueX64(mICDCheckInNodeId.Value()),
                    ChipLogValueX64(mICDMonitoredSubject.Value()), icdSymmetricKeyHex, icdCounter);
}

void PairingManager::OnICDStayActiveComplete(ScopedNodeId deviceId, uint32_t promisedActiveDuration)
{
    ChipLogProgress(NotSpecified, "ICD Stay Active Complete for device " ChipLogFormatX64 " / promisedActiveDuration: %u",
                    ChipLogValueX64(deviceId.GetNodeId()), promisedActiveDuration);
}

void PairingManager::OnDiscoveredDevice(const Dnssd::CommissionNodeData & nodeData)
{
    // Ignore nodes with closed commissioning window
    VerifyOrReturn(nodeData.commissioningMode != 0);

    auto & resolutionData = nodeData;

    const uint16_t port = resolutionData.port;
    char buf[Inet::IPAddress::kMaxStringLength];
    resolutionData.ipAddress[0].ToString(buf);
    ChipLogProgress(NotSpecified, "Discovered Device: %s:%u", buf, port);

    // Stop Mdns discovery.
    auto err = mCommissioner->StopCommissionableDiscovery();

    // Some platforms does not implement a mechanism to stop mdns browse, so
    // we just ignore CHIP_ERROR_NOT_IMPLEMENTED instead of bailing out.
    if (CHIP_NO_ERROR != err && CHIP_ERROR_NOT_IMPLEMENTED != err)
    {
        return;
    }

    mCommissioner->RegisterDeviceDiscoveryDelegate(nullptr);

    auto interfaceId = resolutionData.ipAddress[0].IsIPv6LinkLocal() ? resolutionData.interfaceId : Inet::InterfaceId::Null();
    auto peerAddress = Transport::PeerAddress::UDP(resolutionData.ipAddress[0], port, interfaceId);
    err              = Pair(mNodeId, peerAddress);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogProgress(NotSpecified, "Failed to pair device: " ChipLogFormatX64 " %s", ChipLogValueX64(mNodeId), ErrorStr(err));
    }
}

Optional<uint16_t> PairingManager::FailSafeExpiryTimeoutSecs() const
{
    // No manual input, so do not need to extend.
    return Optional<uint16_t>();
}

bool PairingManager::ShouldWaitAfterDeviceAttestation()
{
    // If there is a vendor ID and product ID, request OnDeviceAttestationCompleted().
    // Currently this is added in the case that the example is performing reverse commissioning,
    // but it would be an improvement to store that explicitly.
    // TODO: Issue #35297 - [Fabric Sync] Improve where we get VID and PID when validating CCTRL CommissionNode command
    SetupPayload payload;
    CHIP_ERROR err = GetPayload(mOnboardingPayload, payload);
    return err == CHIP_NO_ERROR && (payload.vendorID != 0 || payload.productID != 0);
}

void PairingManager::OnDeviceAttestationCompleted(Controller::DeviceCommissioner * deviceCommissioner, DeviceProxy * device,
                                                  const Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
                                                  Credentials::AttestationVerificationResult attestationResult)
{
    SetupPayload payload;
    CHIP_ERROR parse_error = GetPayload(mOnboardingPayload, payload);
    if (parse_error == CHIP_NO_ERROR && (payload.vendorID != 0 || payload.productID != 0))
    {
        if (payload.vendorID == 0 || payload.productID == 0)
        {
            ChipLogProgress(NotSpecified,
                            "Failed validation: vendorID or productID must not be 0."
                            "Requested VID: %u, Requested PID: %u.",
                            payload.vendorID, payload.productID);
            deviceCommissioner->ContinueCommissioningAfterDeviceAttestation(
                device, Credentials::AttestationVerificationResult::kInvalidArgument);
            return;
        }

        if (payload.vendorID != info.BasicInformationVendorId() || payload.productID != info.BasicInformationProductId())
        {
            ChipLogProgress(NotSpecified,
                            "Failed validation of vendorID or productID."
                            "Requested VID: %u, Requested PID: %u,"
                            "Detected VID: %u, Detected PID %u.",
                            payload.vendorID, payload.productID, info.BasicInformationVendorId(), info.BasicInformationProductId());
            deviceCommissioner->ContinueCommissioningAfterDeviceAttestation(
                device,
                payload.vendorID == info.BasicInformationVendorId()
                    ? Credentials::AttestationVerificationResult::kDacProductIdMismatch
                    : Credentials::AttestationVerificationResult::kDacVendorIdMismatch);
            return;
        }

        // NOTE: This will log errors even if the attestion was successful.
        CHIP_ERROR err = deviceCommissioner->ContinueCommissioningAfterDeviceAttestation(device, attestationResult);
        if (CHIP_NO_ERROR != err)
        {
            ChipLogError(NotSpecified, "Failed to continue commissioning after device attestation, error: %s", ErrorStr(err));
        }
        return;
    }

    // Don't bypass attestation, continue with error.
    CHIP_ERROR err = deviceCommissioner->ContinueCommissioningAfterDeviceAttestation(device, attestationResult);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(NotSpecified, "Failed to continue commissioning after device attestation, error: %s", ErrorStr(err));
    }
}

CommissioningParameters PairingManager::GetCommissioningParameters()
{
    auto params = CommissioningParameters();
    params.SetSkipCommissioningComplete(false);
    params.SetDeviceAttestationDelegate(this);

    if (mICDRegistration.ValueOr(false))
    {
        params.SetICDRegistrationStrategy(ICDRegistrationStrategy::kBeforeComplete);

        if (!mICDSymmetricKey.HasValue())
        {
            Crypto::DRBG_get_bytes(mRandomGeneratedICDSymmetricKey, sizeof(mRandomGeneratedICDSymmetricKey));
            mICDSymmetricKey.SetValue(ByteSpan(mRandomGeneratedICDSymmetricKey));
        }
        if (!mICDCheckInNodeId.HasValue())
        {
            mICDCheckInNodeId.SetValue(mCommissioner->GetNodeId());
        }
        if (!mICDMonitoredSubject.HasValue())
        {
            mICDMonitoredSubject.SetValue(mICDCheckInNodeId.Value());
        }
        if (!mICDClientType.HasValue())
        {
            mICDClientType.SetValue(app::Clusters::IcdManagement::ClientTypeEnum::kPermanent);
        }
        // These Optionals must have values now.
        // The commissioner will verify these values.
        params.SetICDSymmetricKey(mICDSymmetricKey.Value());
        if (mICDStayActiveDurationMsec.HasValue())
        {
            params.SetICDStayActiveDurationMsec(mICDStayActiveDurationMsec.Value());
        }
        params.SetICDCheckInNodeId(mICDCheckInNodeId.Value());
        params.SetICDMonitoredSubject(mICDMonitoredSubject.Value());
        params.SetICDClientType(mICDClientType.Value());
    }

    return params;
}

CHIP_ERROR PairingManager::Pair(NodeId remoteId, Transport::PeerAddress address)
{
    auto params = RendezvousParameters().SetSetupPINCode(mSetupPINCode).SetDiscriminator(mDiscriminator).SetPeerAddress(address);

    CHIP_ERROR err           = CHIP_NO_ERROR;
    auto commissioningParams = GetCommissioningParameters();
    err                      = CurrentCommissioner().PairDevice(remoteId, params, commissioningParams);

    return err;
}

void PairingManager::OnCurrentFabricRemove(void * context, NodeId nodeId, CHIP_ERROR err)
{
    PairingManager * self = reinterpret_cast<PairingManager *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(NotSpecified, "OnCurrentFabricRemove: context is null"));

    if (err == CHIP_NO_ERROR)
    {
        // print to console
        fprintf(stderr, "Device with Node ID: " ChipLogFormatX64 "has been successfully removed.\n", ChipLogValueX64(nodeId));

#if defined(PW_RPC_ENABLED)
        FabricIndex fabricIndex = self->CurrentCommissioner().GetFabricIndex();
        app::InteractionModelEngine::GetInstance()->ShutdownSubscriptions(fabricIndex, nodeId);
        ScopedNodeId scopedNodeId(nodeId, fabricIndex);
        RemoveSynchronizedDevice(scopedNodeId);
#endif
    }
    else
    {
        ChipLogProgress(NotSpecified, "Device unpair Failure: " ChipLogFormatX64 " %s", ChipLogValueX64(nodeId), ErrorStr(err));
    }
}

void PairingManager::InitPairingCommand()
{
    mCommissioner->RegisterPairingDelegate(this);
    // Clear the CATs in OperationalCredentialsIssuer
    mCredIssuerCmds->SetCredentialIssuerCATValues(kUndefinedCATs);
    mDeviceIsICD = false;
}

CHIP_ERROR PairingManager::PairDeviceWithCode(NodeId nodeId, const char * payload)
{
    if (payload == nullptr || strlen(payload) > kMaxManualCodeLength + 1)
    {
        ChipLogError(NotSpecified, "PairDeviceWithCode failed: Invalid pairing payload");
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    auto params = Platform::MakeUnique<PairDeviceWithCodeParams>();
    VerifyOrReturnError(params != nullptr, CHIP_ERROR_NO_MEMORY);

    params->nodeId = nodeId;
    Platform::CopyString(params->payloadBuffer, sizeof(params->payloadBuffer), payload);

    // Schedule work on the Matter thread
    return DeviceLayer::PlatformMgr().ScheduleWork(OnPairDeviceWithCode, reinterpret_cast<intptr_t>(params.release()));
}

void PairingManager::OnPairDeviceWithCode(intptr_t context)
{
    Platform::UniquePtr<PairDeviceWithCodeParams> params(reinterpret_cast<PairDeviceWithCodeParams *>(context));
    PairingManager & self = PairingManager::Instance();

    self.InitPairingCommand();

    CommissioningParameters commissioningParams = self.GetCommissioningParameters();
    auto discoveryType                          = DiscoveryType::kDiscoveryNetworkOnly;

    self.mNodeId            = params->nodeId;
    self.mOnboardingPayload = params->payloadBuffer;

    CHIP_ERROR err = self.mCommissioner->PairDevice(params->nodeId, params->payloadBuffer, commissioningParams, discoveryType);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to pair device with code, error: %s", ErrorStr(err));
    }
}

CHIP_ERROR PairingManager::PairDevice(chip::NodeId nodeId, uint32_t setupPINCode, const char * deviceRemoteIp,
                                      uint16_t deviceRemotePort)
{
    if (deviceRemoteIp == nullptr || strlen(deviceRemoteIp) > Inet::IPAddress::kMaxStringLength)
    {
        ChipLogError(NotSpecified, "PairDevice failed: Invalid device remote IP address");
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    auto params = Platform::MakeUnique<PairDeviceParams>();
    VerifyOrReturnError(params != nullptr, CHIP_ERROR_NO_MEMORY);

    params->nodeId           = nodeId;
    params->setupPINCode     = setupPINCode;
    params->deviceRemotePort = deviceRemotePort;

    Platform::CopyString(params->ipAddrBuffer, sizeof(params->ipAddrBuffer), deviceRemoteIp);

    // Schedule work on the Matter thread
    return DeviceLayer::PlatformMgr().ScheduleWork(OnPairDevice, reinterpret_cast<intptr_t>(params.release()));
}

void PairingManager::OnPairDevice(intptr_t context)
{
    Platform::UniquePtr<PairDeviceParams> params(reinterpret_cast<PairDeviceParams *>(context));
    PairingManager & self = PairingManager::Instance();

    self.InitPairingCommand();
    self.mSetupPINCode = params->setupPINCode;

    Inet::IPAddress address;
    Inet::InterfaceId interfaceId;

    if (!ParseAddressWithInterface(params->ipAddrBuffer, address, interfaceId))
    {
        ChipLogError(NotSpecified, "Invalid IP address: %s", params->ipAddrBuffer);
        return;
    }

    CHIP_ERROR err = self.Pair(params->nodeId, Transport::PeerAddress::UDP(address, params->deviceRemotePort, interfaceId));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to pair device, error: %s", ErrorStr(err));
    }
}

CHIP_ERROR PairingManager::UnpairDevice(NodeId nodeId)
{
    auto params = Platform::MakeUnique<UnpairDeviceParams>();
    VerifyOrReturnError(params != nullptr, CHIP_ERROR_NO_MEMORY);

    params->nodeId = nodeId;

    // Schedule work on the Matter thread
    return DeviceLayer::PlatformMgr().ScheduleWork(OnUnpairDevice, reinterpret_cast<intptr_t>(params.release()));
}

void PairingManager::OnUnpairDevice(intptr_t context)
{
    Platform::UniquePtr<PairDeviceParams> params(reinterpret_cast<PairDeviceParams *>(context));
    PairingManager & self = PairingManager::Instance();

    self.InitPairingCommand();

    self.mCurrentFabricRemover = Platform::MakeUnique<Controller::CurrentFabricRemover>(self.mCommissioner);

    if (!self.mCurrentFabricRemover)
    {
        ChipLogError(NotSpecified, "Failed to unpair device, mCurrentFabricRemover is null");
        return;
    }

    CHIP_ERROR err = self.mCurrentFabricRemover->RemoveCurrentFabric(params->nodeId, &self.mCurrentFabricRemoveCallback);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to unpair device, error: %s", ErrorStr(err));
    }
}
