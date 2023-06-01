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
    UndefinedDoNotUse = 0,

    DeviceCommissioner_FindCommissioneeDevice                          = 1,
    DeviceCommissioner_PairDevice                                      = 2,
    DeviceCommissioner_PairDevice                                      = 3,
    DeviceCommissioner_PairDevice                                      = 4,
    DeviceCommissioner_PairDevice                                      = 5,
    DeviceCommissioner_EstablishPASEConnection                         = 6,
    DeviceCommissioner_EstablishPASEConnection                         = 7,
    DeviceCommissioner_Commission                                      = 8,
    DeviceCommissioner_continueCommissioningDevice                     = 9,
    DeviceCommissioner_UnpairDevice                                    = 10,
    DeviceCommissioner_SendCertificateChainRequestCommand              = 11,
    DeviceCommissioner_OnCertificateChainFailureResponse               = 12,
    DeviceCommissioner_OnCertificateChainResponse                      = 13,
    DeviceCommissioner_SendAttestationRequestCommand                   = 14,
    DeviceCommissioner_OnAttestationFailureResponse                    = 15,
    DeviceCommissioner_OnAttestationResponse                           = 16,
    DeviceCommissioner_OnDeviceAttestationInformationVerification      = 17,
    DeviceCommissioner_ValidateAttestationInfo                         = 18,
    DeviceCommissioner_ValidateCSR                                     = 19,
    DeviceCommissioner_SendOperationalCertificateSigningRequestCommand = 20,
    DeviceCommissioner_OnCSRFailureResponse                            = 21,
    DeviceCommissioner_OnOperationalCertificateSigningRequest          = 22,
    DeviceCommissioner_OnDeviceNOCChainGeneration                      = 23,
    DeviceCommissioner_IssueNOCChain                                   = 24,
    DeviceCommissioner_ProcessOpCSR                                    = 25,
    DeviceCommissioner_SendOperationalCertificate                      = 26,
    DeviceCommissioner_OnAddNOCFailureResponse                         = 27,
    DeviceCommissioner_OnOperationalCertificateAddResponse             = 28,
    DeviceCommissioner_SendTrustedRootCertificate                      = 29,
    DeviceCommissioner_OnRootCertSuccessResponse                       = 30,
    DeviceCommissioner_OnRootCertFailureResponse                       = 31,
    DeviceCommissioner_OnOperationalCredentialsProvisioningCompletion  = 32,
    DeviceCommissioner_CommissioningStageComplete                      = 33,
    CASESession_EstablishSession                                       = 34,
    CASESession_SendSigma1                                             = 35,
    CASESession_HandleSigma1_and_SendSigma2                            = 36,
    CASESession_HandleSigma1                                           = 37,
    CASESession_SendSigma2Resume                                       = 38,
    CASESession_SendSigma2                                             = 39,
    CASESession_HandleSigma2Resume                                     = 40,
    CASESession_HandleSigma2_and_SendSigma3                            = 41,
    CASESession_HandleSigma2                                           = 42,
    CASESession_SendSigma3                                             = 43,
    CASESession_HandleSigma3                                           = 44,
    PASESession_GeneratePASEVerifier                                   = 45,
    PASESession_SetupSpake2p                                           = 46,
    PASESession_Pair                                                   = 47,
    PASESession_SendPBKDFParamRequest                                  = 48,
    PASESession_HandlePBKDFParamRequest                                = 49,
    PASESession_SendPBKDFParamResponse                                 = 50,
    PASESession_HandlePBKDFParamResponse                               = 51,
    PASESession_SendMsg1                                               = 52,
    PASESession_HandleMsg1_and_SendMsg2                                = 53,
    PASESession_HandleMsg2_and_SendMsg3                                = 54,
    PASESession_HandleMsg3                                             = 55,
    GeneralCommissioning_ArmFailSafe                                   = 56,
    GeneralCommissioning_CommissioningComplete                         = 57,
    GeneralCommissioning_SetRegulatoryConfig                           = 58,
    NetworkCommissioning_HandleScanNetwork                             = 59,
    NetworkCommissioning_HandleAddOrUpdateWiFiNetwork                  = 60,
    NetworkCommissioning_HandleAddOrUpdateThreadNetwork                = 61,
    NetworkCommissioning_HandleRemoveNetwork                           = 62,
    NetworkCommissioning_HandleConnectNetwork                          = 63,
    NetworkCommissioning_HandleReorderNetwork                          = 64,
    OperationalCredentials_RemoveFabric                                = 65,
    OperationalCredentials_UpdateFabricLabel                           = 66,
    OperationalCredentials_AddNOC                                      = 67,
    OperationalCredentials_UpdateNOC                                   = 68,
    OperationalCredentials_CertificateChainRequest                     = 69,
    OperationalCredentials_AttestationRequest                          = 70,
    OperationalCredentials_CSRRequest                                  = 71,
    OperationalCredentials_AddTrustedRootCertificate                   = 72,
};

/// An event that happened at an instant (like a zero sized scope)
enum class Instant
{
    UndefinedDoNotUse = 0,

    // General instant notifications
    Resolve_TxtNotApplicable  = 1,
    Resolve_Ipv4NotApplicable = 2,
    Resolve_Ipv6NotApplicable = 3,

    // Used if and only if default "Log*" requests
    // are not implemented in the backend.
    Log_MessageSend         = 1000,
    Log_MessageReceived     = 1001,
    Log_NodeLookup          = 1002,
    Log_NodeDiscovered      = 1003,
    Log_NodeDiscoveryFailed = 1004,
};

} // namespace Tracing
} // namespace chip
