/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <matter/tracing/build_config.h>

namespace chip {
namespace Tracing {

/**
 * Defines the key type use to identity a specific group
 */
typedef const char * LabelKey;

/**
 * List of supported label keys
 */
constexpr LabelKey kA = "A";

constexpr LabelKey kAbortPendingEstablish = "AbortPendingEstablish";

constexpr LabelKey kActive_commissioning_delegate_call = "Active commissioning delegate call";

constexpr LabelKey kActive_operational_delegate_call = "Active operational delegate call";

constexpr LabelKey kAddGroup = "AddGroup";

constexpr LabelKey kAddGroupIfIdentifying = "AddGroupIfIdentifying";

constexpr LabelKey kAddICAC = "AddICAC";

constexpr LabelKey kAddNOC = "AddNOC";

constexpr LabelKey kAddNewPendingFabricCommon = "AddNewPendingFabricCommon";

constexpr LabelKey kAddScene = "AddScene";

constexpr LabelKey kAddTrustedRootCertificate = "AddTrustedRootCertificate";

constexpr LabelKey kAdvance_pending_resolve_states = "Advance pending resolve states";

constexpr LabelKey kAnnounceJointFabricAdministrator = "AnnounceJointFabricAdministrator";

constexpr LabelKey kArmFailSafe = "ArmFailSafe";

constexpr LabelKey kAttestationRequest = "AttestationRequest";

constexpr LabelKey kB = "B";

constexpr LabelKey kC = "C";

constexpr LabelKey kCASEFail = "CASEFail";

constexpr LabelKey kCASETimeout = "CASETimeout";

constexpr LabelKey kCSRRequest = "CSRRequest";

constexpr LabelKey kCertificateChainRequest = "CertificateChainRequest";

constexpr LabelKey kChangeToMode = "ChangeToMode";

constexpr LabelKey kCheckForRevokedDACChain = "CheckForRevokedDACChain";

constexpr LabelKey kClear = "Clear";

constexpr LabelKey kCommission = "Commission";

constexpr LabelKey kCommissioningComplete = "CommissioningComplete";

constexpr LabelKey kCommissioningStageComplete = "CommissioningStageComplete";

constexpr LabelKey kCopyScene = "CopyScene";

constexpr LabelKey kD = "D";

constexpr LabelKey kDelete = "Delete";

constexpr LabelKey kE = "E";

constexpr LabelKey kEncodeSigma1 = "EncodeSigma1";

constexpr LabelKey kEncodeSigma2Resume = "EncodeSigma2Resume";

constexpr LabelKey kEstablishPASEConnection = "EstablishPASEConnection";

constexpr LabelKey kEstablishSession = "EstablishSession";

constexpr LabelKey kFOO = "FOO";

constexpr LabelKey kFetchICACert = "FetchICACert";

constexpr LabelKey kFetchNOCCert = "FetchNOCCert";

constexpr LabelKey kFetchPendingNonFabricAssociatedRootCert = "FetchPendingNonFabricAssociatedRootCert";

constexpr LabelKey kFetchRootCert = "FetchRootCert";

constexpr LabelKey kFetchRootPubKey = "FetchRootPubKey";

constexpr LabelKey kFetchRootPubkey = "FetchRootPubkey";

constexpr LabelKey kFindCommissioneeDevice = "FindCommissioneeDevice";

constexpr LabelKey kFindExistingFabricByNocChaining = "FindExistingFabricByNocChaining";

constexpr LabelKey kFindLocalNodeFromDestinationId = "FindLocalNodeFromDestinationId";

constexpr LabelKey kGeneratePASEVerifier = "GeneratePASEVerifier";

constexpr LabelKey kGetGroupMembership = "GetGroupMembership";

constexpr LabelKey kGetSceneMembership = "GetSceneMembership";

constexpr LabelKey kGroup_Message_Dispatch = "Group Message Dispatch";

constexpr LabelKey kHandleAddOrUpdateThreadNetwork = "HandleAddOrUpdateThreadNetwork";

constexpr LabelKey kHandleAddOrUpdateWiFiNetwork = "HandleAddOrUpdateWiFiNetwork";

constexpr LabelKey kHandleConnectNetwork = "HandleConnectNetwork";

constexpr LabelKey kHandleMsg1_and_SendMsg2 = "HandleMsg1_and_SendMsg2";

constexpr LabelKey kHandleMsg2_and_SendMsg3 = "HandleMsg2_and_SendMsg3";

constexpr LabelKey kHandleMsg3 = "HandleMsg3";

constexpr LabelKey kHandlePBKDFParamRequest = "HandlePBKDFParamRequest";

constexpr LabelKey kHandlePBKDFParamResponse = "HandlePBKDFParamResponse";

constexpr LabelKey kHandleQueryIdentity = "HandleQueryIdentity";

constexpr LabelKey kHandleRemoveNetwork = "HandleRemoveNetwork";

constexpr LabelKey kHandleReorderNetwork = "HandleReorderNetwork";

constexpr LabelKey kHandleScanNetwork = "HandleScanNetwork";

constexpr LabelKey kHandleSigma1 = "HandleSigma1";

constexpr LabelKey kHandleSigma1_and_SendSigma2 = "HandleSigma1_and_SendSigma2";

constexpr LabelKey kHandleSigma2 = "HandleSigma2";

constexpr LabelKey kHandleSigma2Resume = "HandleSigma2Resume";

constexpr LabelKey kHandleSigma2_and_SendSigma3 = "HandleSigma2_and_SendSigma3";

constexpr LabelKey kHandleSigma3 = "HandleSigma3";

constexpr LabelKey kICACCSRRequest = "ICACCSRRequest";

constexpr LabelKey kIPv4_not_applicable = "IPv4 not applicable";

constexpr LabelKey kIPv6_not_applicable = "IPv6 not applicable";

constexpr LabelKey kIdentifyCommand = "IdentifyCommand";

constexpr LabelKey kInit = "Init";

constexpr LabelKey kInitCASEHandshake = "InitCASEHandshake";

constexpr LabelKey kIssueNOCChain = "IssueNOCChain";

constexpr LabelKey kMove = "Move";

constexpr LabelKey kMoveToLevel = "MoveToLevel";

constexpr LabelKey kMoveToLevelWithOnOff = "MoveToLevelWithOnOff";

constexpr LabelKey kMoveWithOnOff = "MoveWithOnOff";

constexpr LabelKey kNotifyFabricCommitted = "NotifyFabricCommitted";

constexpr LabelKey kNotifyFabricUpdated = "NotifyFabricUpdated";

constexpr LabelKey kOffCommand = "OffCommand";

constexpr LabelKey kOnAddNOCFailureResponse = "OnAddNOCFailureResponse";

constexpr LabelKey kOnAssociationFailureDetected = "OnAssociationFailureDetected";

constexpr LabelKey kOnAttestationFailureResponse = "OnAttestationFailureResponse";

constexpr LabelKey kOnAttestationResponse = "OnAttestationResponse";

constexpr LabelKey kOnCSRFailureResponse = "OnCSRFailureResponse";

constexpr LabelKey kOnCertificateChainFailureResponse = "OnCertificateChainFailureResponse";

constexpr LabelKey kOnCertificateChainResponse = "OnCertificateChainResponse";

constexpr LabelKey kOnCommand = "OnCommand";

constexpr LabelKey kOnConnectionStatusChanged = "OnConnectionStatusChanged";

constexpr LabelKey kOnDeviceAttestationInformationVerification = "OnDeviceAttestationInformationVerification";

constexpr LabelKey kOnDeviceNOCChainGeneration = "OnDeviceNOCChainGeneration";

constexpr LabelKey kOnDisconnectionDetected = "OnDisconnectionDetected";

constexpr LabelKey kOnMessageReceived = "OnMessageReceived";

constexpr LabelKey kOnOperationalCertificateAddResponse = "OnOperationalCertificateAddResponse";

constexpr LabelKey kOnOperationalCertificateSigningRequest = "OnOperationalCertificateSigningRequest";

constexpr LabelKey kOnOperationalCredentialsProvisioningCompletion = "OnOperationalCredentialsProvisioningCompletion";

constexpr LabelKey kOnResponseTimeout = "OnResponseTimeout";

constexpr LabelKey kOnRootCertFailureResponse = "OnRootCertFailureResponse";

constexpr LabelKey kOnRootCertSuccessResponse = "OnRootCertSuccessResponse";

constexpr LabelKey kOnSessionEstablished = "OnSessionEstablished";

constexpr LabelKey kOnSessionEstablishmentError = "OnSessionEstablishmentError";

constexpr LabelKey kOnShutDown = "OnShutDown";

constexpr LabelKey kOnStartUp = "OnStartUp";

constexpr LabelKey kOnWithRecallGlobalSceneCommand = "OnWithRecallGlobalSceneCommand";

constexpr LabelKey kOnWithTimedOffCommand = "OnWithTimedOffCommand";

constexpr LabelKey kOpenBasicCommissioningWindow = "OpenBasicCommissioningWindow";

constexpr LabelKey kOpenCommissioningWindow = "OpenCommissioningWindow";

constexpr LabelKey kOpenJointCommissioningWindow = "OpenJointCommissioningWindow";

constexpr LabelKey kPASEFail = "PASEFail";

constexpr LabelKey kPASETimeout = "PASETimeout";

constexpr LabelKey kPair = "Pair";

constexpr LabelKey kPairDevice = "PairDevice";

constexpr LabelKey kPake1 = "Pake1";

constexpr LabelKey kPake2 = "Pake2";

constexpr LabelKey kPake3 = "Pake3";

constexpr LabelKey kPrepareForSessionEstablishment = "PrepareForSessionEstablishment";

constexpr LabelKey kPrepareMessage = "PrepareMessage";

constexpr LabelKey kPrepareSigma2 = "PrepareSigma2";

constexpr LabelKey kPrepareSigma2Resume = "PrepareSigma2Resume";

constexpr LabelKey kProcessOpCSR = "ProcessOpCSR";

constexpr LabelKey kReachableChanged = "ReachableChanged";

constexpr LabelKey kRecallScene = "RecallScene";

constexpr LabelKey kReceived_MDNS_Packet = "Received MDNS Packet";

constexpr LabelKey kRemoveAllGroups = "RemoveAllGroups";

constexpr LabelKey kRemoveAllScenes = "RemoveAllScenes";

constexpr LabelKey kRemoveFabric = "RemoveFabric";

constexpr LabelKey kRemoveGroup = "RemoveGroup";

constexpr LabelKey kRemoveScene = "RemoveScene";

constexpr LabelKey kRevertPendingFabricData = "RevertPendingFabricData";

constexpr LabelKey kRevertPendingOpCertsExceptRoot = "RevertPendingOpCertsExceptRoot";

constexpr LabelKey kRevokeCommissioning = "RevokeCommissioning";

constexpr LabelKey kSchedule_retries = "Schedule retries";

constexpr LabelKey kSearching_NON_SRV_Records = "Searching NON-SRV Records";

constexpr LabelKey kSearching_SRV_Records = "Searching SRV Records";

constexpr LabelKey kSecure_Unicast_Message_Dispatch = "Secure Unicast Message Dispatch";

constexpr LabelKey kSendAttestationRequestCommand = "SendAttestationRequestCommand";

constexpr LabelKey kSendBusyStatusReport = "SendBusyStatusReport";

constexpr LabelKey kSendCertificateChainRequestCommand = "SendCertificateChainRequestCommand";

constexpr LabelKey kSendMsg1 = "SendMsg1";

constexpr LabelKey kSendOperationalCertificate = "SendOperationalCertificate";

constexpr LabelKey kSendOperationalCertificateSigningRequestCommand = "SendOperationalCertificateSigningRequestCommand";

constexpr LabelKey kSendPBKDFParamRequest = "SendPBKDFParamRequest";

constexpr LabelKey kSendPBKDFParamResponse = "SendPBKDFParamResponse";

constexpr LabelKey kSendSigma1 = "SendSigma1";

constexpr LabelKey kSendSigma2 = "SendSigma2";

constexpr LabelKey kSendSigma3 = "SendSigma3";

constexpr LabelKey kSendTrustedRootCertificate = "SendTrustedRootCertificate";

constexpr LabelKey kSetRegulatoryConfig = "SetRegulatoryConfig";

constexpr LabelKey kSetTCAcknowledgements = "SetTCAcknowledgements";

constexpr LabelKey kSetupSpake2p = "SetupSpake2p";

constexpr LabelKey kSigma1 = "Sigma1";

constexpr LabelKey kSigma2 = "Sigma2";

constexpr LabelKey kSigma2Resume = "Sigma2Resume";

constexpr LabelKey kSigma3 = "Sigma3";

constexpr LabelKey kSignWithOpKeypair = "SignWithOpKeypair";

constexpr LabelKey kSleep = "Sleep";

constexpr LabelKey kStep = "Step";

constexpr LabelKey kStepWithOnOff = "StepWithOnOff";

constexpr LabelKey kStop = "Stop";

constexpr LabelKey kStopWithOnOff = "StopWithOnOff";

constexpr LabelKey kStoreScene = "StoreScene";

constexpr LabelKey kTXT_not_applicable = "TXT not applicable";

constexpr LabelKey kToggleCommand = "ToggleCommand";

constexpr LabelKey kTransferAnchorComplete = "TransferAnchorComplete";

constexpr LabelKey kTransferAnchorRequest = "TransferAnchorRequest";

constexpr LabelKey kTriggerEffect = "TriggerEffect";

constexpr LabelKey kTryResumeSession = "TryResumeSession";

constexpr LabelKey kUnauthenticated_Message_Dispatch = "Unauthenticated Message Dispatch";

constexpr LabelKey kUnpairDevice = "UnpairDevice";

constexpr LabelKey kUpdateFabricLabel = "UpdateFabricLabel";

constexpr LabelKey kUpdateNOC = "UpdateNOC";

constexpr LabelKey kUpdatePendingFabricCommon = "UpdatePendingFabricCommon";

constexpr LabelKey kValidateAttestationInfo = "ValidateAttestationInfo";

constexpr LabelKey kValidateCSR = "ValidateCSR";

constexpr LabelKey kValidateIncomingNOCChain = "ValidateIncomingNOCChain";

constexpr LabelKey kVerifyCredentials = "VerifyCredentials";

constexpr LabelKey kViewGroup = "ViewGroup";

constexpr LabelKey kViewScene = "ViewScene";

constexpr LabelKey k_1 = "1";

constexpr LabelKey k_2 = "2";

constexpr LabelKey k_3 = "3";

constexpr LabelKey k_4 = "4";

constexpr LabelKey kcolorLoop = "colorLoop";

constexpr LabelKey kcontinueCommissioningAfterConnectNetworkRequest = "continueCommissioningAfterConnectNetworkRequest";

constexpr LabelKey kcontinueCommissioningDevice = "continueCommissioningDevice";

constexpr LabelKey kmoveHue = "moveHue";

constexpr LabelKey kmoveSaturation = "moveSaturation";

constexpr LabelKey kmoveToHue = "moveToHue";

constexpr LabelKey kmoveToHueAndSaturation = "moveToHueAndSaturation";

constexpr LabelKey kmoveToSaturation = "moveToSaturation";

constexpr LabelKey koffWithEffectCommand = "offWithEffectCommand";

constexpr LabelKey ksetOnOffValue = "setOnOffValue";

constexpr LabelKey kstepHue = "stepHue";

constexpr LabelKey kstepSaturation = "stepSaturation";

constexpr LabelKey kupdateHueSat = "updateHueSat";

/**
 * Array of all label keys
 */
constexpr LabelKey kAllLabelKeys[] = {
    kA,
    kAbortPendingEstablish,
    kActive_commissioning_delegate_call,
    kActive_operational_delegate_call,
    kAddGroup,
    kAddGroupIfIdentifying,
    kAddICAC,
    kAddNOC,
    kAddNewPendingFabricCommon,
    kAddScene,
    kAddTrustedRootCertificate,
    kAdvance_pending_resolve_states,
    kAnnounceJointFabricAdministrator,
    kArmFailSafe,
    kAttestationRequest,
    kB,
    kC,
    kCASEFail,
    kCASETimeout,
    kCSRRequest,
    kCertificateChainRequest,
    kChangeToMode,
    kCheckForRevokedDACChain,
    kClear,
    kCommission,
    kCommissioningComplete,
    kCommissioningStageComplete,
    kCopyScene,
    kD,
    kDelete,
    kE,
    kEncodeSigma1,
    kEncodeSigma2Resume,
    kEstablishPASEConnection,
    kEstablishSession,
    kFOO,
    kFetchICACert,
    kFetchNOCCert,
    kFetchPendingNonFabricAssociatedRootCert,
    kFetchRootCert,
    kFetchRootPubKey,
    kFetchRootPubkey,
    kFindCommissioneeDevice,
    kFindExistingFabricByNocChaining,
    kFindLocalNodeFromDestinationId,
    kGeneratePASEVerifier,
    kGetGroupMembership,
    kGetSceneMembership,
    kGroup_Message_Dispatch,
    kHandleAddOrUpdateThreadNetwork,
    kHandleAddOrUpdateWiFiNetwork,
    kHandleConnectNetwork,
    kHandleMsg1_and_SendMsg2,
    kHandleMsg2_and_SendMsg3,
    kHandleMsg3,
    kHandlePBKDFParamRequest,
    kHandlePBKDFParamResponse,
    kHandleQueryIdentity,
    kHandleRemoveNetwork,
    kHandleReorderNetwork,
    kHandleScanNetwork,
    kHandleSigma1,
    kHandleSigma1_and_SendSigma2,
    kHandleSigma2,
    kHandleSigma2Resume,
    kHandleSigma2_and_SendSigma3,
    kHandleSigma3,
    kICACCSRRequest,
    kIPv4_not_applicable,
    kIPv6_not_applicable,
    kIdentifyCommand,
    kInit,
    kInitCASEHandshake,
    kIssueNOCChain,
    kMove,
    kMoveToLevel,
    kMoveToLevelWithOnOff,
    kMoveWithOnOff,
    kNotifyFabricCommitted,
    kNotifyFabricUpdated,
    kOffCommand,
    kOnAddNOCFailureResponse,
    kOnAssociationFailureDetected,
    kOnAttestationFailureResponse,
    kOnAttestationResponse,
    kOnCSRFailureResponse,
    kOnCertificateChainFailureResponse,
    kOnCertificateChainResponse,
    kOnCommand,
    kOnConnectionStatusChanged,
    kOnDeviceAttestationInformationVerification,
    kOnDeviceNOCChainGeneration,
    kOnDisconnectionDetected,
    kOnMessageReceived,
    kOnOperationalCertificateAddResponse,
    kOnOperationalCertificateSigningRequest,
    kOnOperationalCredentialsProvisioningCompletion,
    kOnResponseTimeout,
    kOnRootCertFailureResponse,
    kOnRootCertSuccessResponse,
    kOnSessionEstablished,
    kOnSessionEstablishmentError,
    kOnShutDown,
    kOnStartUp,
    kOnWithRecallGlobalSceneCommand,
    kOnWithTimedOffCommand,
    kOpenBasicCommissioningWindow,
    kOpenCommissioningWindow,
    kOpenJointCommissioningWindow,
    kPASEFail,
    kPASETimeout,
    kPair,
    kPairDevice,
    kPake1,
    kPake2,
    kPake3,
    kPrepareForSessionEstablishment,
    kPrepareMessage,
    kPrepareSigma2,
    kPrepareSigma2Resume,
    kProcessOpCSR,
    kReachableChanged,
    kRecallScene,
    kReceived_MDNS_Packet,
    kRemoveAllGroups,
    kRemoveAllScenes,
    kRemoveFabric,
    kRemoveGroup,
    kRemoveScene,
    kRevertPendingFabricData,
    kRevertPendingOpCertsExceptRoot,
    kRevokeCommissioning,
    kSchedule_retries,
    kSearching_NON_SRV_Records,
    kSearching_SRV_Records,
    kSecure_Unicast_Message_Dispatch,
    kSendAttestationRequestCommand,
    kSendBusyStatusReport,
    kSendCertificateChainRequestCommand,
    kSendMsg1,
    kSendOperationalCertificate,
    kSendOperationalCertificateSigningRequestCommand,
    kSendPBKDFParamRequest,
    kSendPBKDFParamResponse,
    kSendSigma1,
    kSendSigma2,
    kSendSigma3,
    kSendTrustedRootCertificate,
    kSetRegulatoryConfig,
    kSetTCAcknowledgements,
    kSetupSpake2p,
    kSigma1,
    kSigma2,
    kSigma2Resume,
    kSigma3,
    kSignWithOpKeypair,
    kSleep,
    kStep,
    kStepWithOnOff,
    kStop,
    kStopWithOnOff,
    kStoreScene,
    kTXT_not_applicable,
    kToggleCommand,
    kTransferAnchorComplete,
    kTransferAnchorRequest,
    kTriggerEffect,
    kTryResumeSession,
    kUnauthenticated_Message_Dispatch,
    kUnpairDevice,
    kUpdateFabricLabel,
    kUpdateNOC,
    kUpdatePendingFabricCommon,
    kValidateAttestationInfo,
    kValidateCSR,
    kValidateIncomingNOCChain,
    kVerifyCredentials,
    kViewGroup,
    kViewScene,
    k_1,
    k_2,
    k_3,
    k_4,
    kcolorLoop,
    kcontinueCommissioningAfterConnectNetworkRequest,
    kcontinueCommissioningDevice,
    kmoveHue,
    kmoveSaturation,
    kmoveToHue,
    kmoveToHueAndSaturation,
    kmoveToSaturation,
    koffWithEffectCommand,
    ksetOnOffValue,
    kstepHue,
    kstepSaturation,
    kupdateHueSat,
}; // kAllLabelKeys

} // namespace Tracing
} // namespace chip
