/*
 *    Copyright (c) 2023 Project CHIP Authors
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
#pragma once

namespace chip {
namespace Tracing {

/// Trace scopes defined as an enumeration, since various tracing back-ends
/// may need to use constant strings for tracing.
///
/// As a result, tracing scopes in CHIP are from a known list.
enum class Scope
{
    CASESession_EstablishSession,
    CASESession_HandleSigma1,
    CASESession_HandleSigma1_and_SendSigma2,
    CASESession_HandleSigma2,
    CASESession_HandleSigma2_and_SendSigma3,
    CASESession_HandleSigma2Resume,
    CASESession_HandleSigma3,
    CASESession_SendSigma1,
    CASESession_SendSigma2,
    CASESession_SendSigma2Resume,
    CASESession_SendSigma3,
    DeviceCommissioner_Commission,
    DeviceCommissioner_CommissioningStageComplete,
    DeviceCommissioner_continueCommissioningDevice,
    DeviceCommissioner_EstablishPASEConnection,
    DeviceCommissioner_FindCommissioneeDevice,
    DeviceCommissioner_IssueNOCChain,
    DeviceCommissioner_OnAddNOCFailureResponse,
    DeviceCommissioner_OnAttestationFailureResponse,
    DeviceCommissioner_OnAttestationResponse,
    DeviceCommissioner_OnCertificateChainFailureResponse,
    DeviceCommissioner_OnCertificateChainResponse,
    DeviceCommissioner_OnCSRFailureResponse,
    DeviceCommissioner_OnDeviceAttestationInformationVerification,
    DeviceCommissioner_OnDeviceNOCChainGeneration,
    DeviceCommissioner_OnOperationalCertificateAddResponse,
    DeviceCommissioner_OnOperationalCertificateSigningRequest,
    DeviceCommissioner_OnOperationalCredentialsProvisioningCompletion,
    DeviceCommissioner_OnRootCertFailureResponse,
    DeviceCommissioner_OnRootCertSuccessResponse,
    DeviceCommissioner_PairDevice,
    DeviceCommissioner_ProcessOpCSR,
    DeviceCommissioner_SendAttestationRequestCommand,
    DeviceCommissioner_SendCertificateChainRequestCommand,
    DeviceCommissioner_SendOperationalCertificate,
    DeviceCommissioner_SendOperationalCertificateSigningRequestCommand,
    DeviceCommissioner_SendTrustedRootCertificate,
    DeviceCommissioner_UnpairDevice,
    DeviceCommissioner_ValidateAttestationInfo,
    DeviceCommissioner_ValidateCSR,
    GeneralCommissioning_ArmFailSafe,
    GeneralCommissioning_CommissioningComplete,
    GeneralCommissioning_SetRegulatoryConfig,
    NetworkCommissioning_HandleAddOrUpdateThreadNetwork,
    NetworkCommissioning_HandleAddOrUpdateWiFiNetwork,
    NetworkCommissioning_HandleConnectNetwork,
    NetworkCommissioning_HandleRemoveNetwork,
    NetworkCommissioning_HandleReorderNetwork,
    NetworkCommissioning_HandleScanNetwork,
    OperationalCredentials_AddNOC,
    OperationalCredentials_AddTrustedRootCertificate,
    OperationalCredentials_AttestationRequest,
    OperationalCredentials_CertificateChainRequest,
    OperationalCredentials_CSRRequest,
    OperationalCredentials_RemoveFabric,
    OperationalCredentials_UpdateFabricLabel,
    OperationalCredentials_UpdateNOC,
    PASESession_GeneratePASEVerifier,
    PASESession_HandleMsg1_and_SendMsg2,
    PASESession_HandleMsg2_and_SendMsg3,
    PASESession_HandleMsg3,
    PASESession_HandlePBKDFParamRequest,
    PASESession_HandlePBKDFParamResponse,
    PASESession_Pair,
    PASESession_SendMsg1,
    PASESession_SendPBKDFParamRequest,
    PASESession_SendPBKDFParamResponse,
    PASESession_SetupSpake2p,
};


/// An event that happened instant (like a zero sized scope)
enum class Instant {
    // General instant notifications
    Resolve_TxtNotApplicable,
    Resolve_Ipv4NotApplicable,
    Resolve_Ipv6NotApplicable,

    // Used if and only if default "Log*" requests
    // are not implemented in the backend.
    Log_MessageSend,
    Log_MessageReceived,
    Log_NodeLookup,
    Log_NodeDiscovered,
    Log_NodeDiscoveryFailed,
};

} // namespace Tracing
} // namespace chip
