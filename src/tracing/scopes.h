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

    CASESession_EstablishSession                                       = 1,
    CASESession_HandleSigma1                                           = 2,
    CASESession_HandleSigma1_and_SendSigma2                            = 3,
    CASESession_HandleSigma2                                           = 4,
    CASESession_HandleSigma2_and_SendSigma3                            = 5,
    CASESession_HandleSigma2Resume                                     = 6,
    CASESession_HandleSigma3                                           = 7,
    CASESession_SendSigma1                                             = 8,
    CASESession_SendSigma2                                             = 9,
    CASESession_SendSigma2Resume                                       = 10,
    CASESession_SendSigma3                                             = 11,
    DeviceCommissioner_Commission                                      = 12,
    DeviceCommissioner_CommissioningStageComplete                      = 13,
    DeviceCommissioner_continueCommissioningDevice                     = 14,
    DeviceCommissioner_EstablishPASEConnection                         = 15,
    DeviceCommissioner_FindCommissioneeDevice                          = 16,
    DeviceCommissioner_IssueNOCChain                                   = 17,
    DeviceCommissioner_OnAddNOCFailureResponse                         = 18,
    DeviceCommissioner_OnAttestationFailureResponse                    = 19,
    DeviceCommissioner_OnAttestationResponse                           = 20,
    DeviceCommissioner_OnCertificateChainFailureResponse               = 21,
    DeviceCommissioner_OnCertificateChainResponse                      = 22,
    DeviceCommissioner_OnCSRFailureResponse                            = 23,
    DeviceCommissioner_OnDeviceAttestationInformationVerification      = 24,
    DeviceCommissioner_OnDeviceNOCChainGeneration                      = 25,
    DeviceCommissioner_OnOperationalCertificateAddResponse             = 26,
    DeviceCommissioner_OnOperationalCertificateSigningRequest          = 27,
    DeviceCommissioner_OnOperationalCredentialsProvisioningCompletion  = 28,
    DeviceCommissioner_OnRootCertFailureResponse                       = 29,
    DeviceCommissioner_OnRootCertSuccessResponse                       = 30,
    DeviceCommissioner_PairDevice                                      = 31,
    DeviceCommissioner_ProcessOpCSR                                    = 32,
    DeviceCommissioner_SendAttestationRequestCommand                   = 33,
    DeviceCommissioner_SendCertificateChainRequestCommand              = 34,
    DeviceCommissioner_SendOperationalCertificate                      = 35,
    DeviceCommissioner_SendOperationalCertificateSigningRequestCommand = 36,
    DeviceCommissioner_SendTrustedRootCertificate                      = 37,
    DeviceCommissioner_UnpairDevice                                    = 38,
    DeviceCommissioner_ValidateAttestationInfo                         = 39,
    DeviceCommissioner_ValidateCSR                                     = 40,
    GeneralCommissioning_ArmFailSafe                                   = 41,
    GeneralCommissioning_CommissioningComplete                         = 42,
    GeneralCommissioning_SetRegulatoryConfig                           = 43,
    NetworkCommissioning_HandleAddOrUpdateThreadNetwork                = 44,
    NetworkCommissioning_HandleAddOrUpdateWiFiNetwork                  = 45,
    NetworkCommissioning_HandleConnectNetwork                          = 46,
    NetworkCommissioning_HandleRemoveNetwork                           = 47,
    NetworkCommissioning_HandleReorderNetwork                          = 48,
    NetworkCommissioning_HandleScanNetwork                             = 49,
    OperationalCredentials_AddNOC                                      = 50,
    OperationalCredentials_AddTrustedRootCertificate                   = 51,
    OperationalCredentials_AttestationRequest                          = 52,
    OperationalCredentials_CertificateChainRequest                     = 53,
    OperationalCredentials_CSRRequest                                  = 54,
    OperationalCredentials_RemoveFabric                                = 55,
    OperationalCredentials_UpdateFabricLabel                           = 56,
    OperationalCredentials_UpdateNOC                                   = 57,
    PASESession_GeneratePASEVerifier                                   = 58,
    PASESession_HandleMsg1_and_SendMsg2                                = 59,
    PASESession_HandleMsg2_and_SendMsg3                                = 60,
    PASESession_HandleMsg3                                             = 61,
    PASESession_HandlePBKDFParamRequest                                = 62,
    PASESession_HandlePBKDFParamResponse                               = 63,
    PASESession_Pair                                                   = 64,
    PASESession_SendMsg1                                               = 65,
    PASESession_SendPBKDFParamRequest                                  = 66,
    PASESession_SendPBKDFParamResponse                                 = 67,
    PASESession_SetupSpake2p                                           = 68,
    Resolve_IncrementalRecordParse                                     = 69,
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
