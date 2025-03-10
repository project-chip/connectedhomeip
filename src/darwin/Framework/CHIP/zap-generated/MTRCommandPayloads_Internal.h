/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#import <Matter/MTRCommandPayloadsObjc.h>
#import <Matter/MTRDefines.h>

#include <app-common/zap-generated/cluster-objects.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRIdentifyClusterIdentifyParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRIdentifyClusterTriggerEffectParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGroupsClusterAddGroupParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGroupsClusterAddGroupResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Groups::Commands::AddGroupResponse::DecodableType &)decodableStruct;

@end

@interface MTRGroupsClusterViewGroupParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGroupsClusterViewGroupResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Groups::Commands::ViewGroupResponse::DecodableType &)decodableStruct;

@end

@interface MTRGroupsClusterGetGroupMembershipParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGroupsClusterGetGroupMembershipResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Groups::Commands::GetGroupMembershipResponse::DecodableType &)decodableStruct;

@end

@interface MTRGroupsClusterRemoveGroupParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGroupsClusterRemoveGroupResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Groups::Commands::RemoveGroupResponse::DecodableType &)decodableStruct;

@end

@interface MTRGroupsClusterRemoveAllGroupsParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGroupsClusterAddGroupIfIdentifyingParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROnOffClusterOffParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROnOffClusterOnParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROnOffClusterToggleParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROnOffClusterOffWithEffectParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROnOffClusterOnWithRecallGlobalSceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROnOffClusterOnWithTimedOffParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRLevelControlClusterMoveToLevelParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRLevelControlClusterMoveParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRLevelControlClusterStepParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRLevelControlClusterStopParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRLevelControlClusterMoveToLevelWithOnOffParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRLevelControlClusterMoveWithOnOffParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRLevelControlClusterStepWithOnOffParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRLevelControlClusterStopWithOnOffParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRLevelControlClusterMoveToClosestFrequencyParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRAccessControlClusterReviewFabricRestrictionsParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRAccessControlClusterReviewFabricRestrictionsResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::AccessControl::Commands::ReviewFabricRestrictionsResponse::DecodableType &)decodableStruct;

@end

@interface MTRActionsClusterInstantActionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRActionsClusterInstantActionWithTransitionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRActionsClusterStartActionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRActionsClusterStartActionWithDurationParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRActionsClusterStopActionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRActionsClusterPauseActionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRActionsClusterPauseActionWithDurationParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRActionsClusterResumeActionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRActionsClusterEnableActionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRActionsClusterEnableActionWithDurationParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRActionsClusterDisableActionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRActionsClusterDisableActionWithDurationParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROTASoftwareUpdateProviderClusterQueryImageParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROTASoftwareUpdateProviderClusterQueryImageResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::DecodableType &)decodableStruct;

@end

@interface MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::DecodableType &)decodableStruct;

@end

@interface MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROTASoftwareUpdateRequestorClusterAnnounceOTAProviderParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGeneralCommissioningClusterArmFailSafeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGeneralCommissioningClusterArmFailSafeResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType &)decodableStruct;

@end

@interface MTRGeneralCommissioningClusterSetRegulatoryConfigParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::GeneralCommissioning::Commands::SetRegulatoryConfigResponse::DecodableType &)decodableStruct;

@end

@interface MTRGeneralCommissioningClusterCommissioningCompleteParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGeneralCommissioningClusterCommissioningCompleteResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType &)decodableStruct;

@end

@interface MTRGeneralCommissioningClusterSetTCAcknowledgementsParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGeneralCommissioningClusterSetTCAcknowledgementsResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::GeneralCommissioning::Commands::SetTCAcknowledgementsResponse::DecodableType &)decodableStruct;

@end

@interface MTRNetworkCommissioningClusterScanNetworksParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRNetworkCommissioningClusterScanNetworksResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType &)decodableStruct;

@end

@interface MTRNetworkCommissioningClusterAddOrUpdateWiFiNetworkParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRNetworkCommissioningClusterAddOrUpdateThreadNetworkParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRNetworkCommissioningClusterRemoveNetworkParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRNetworkCommissioningClusterNetworkConfigResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType &)decodableStruct;

@end

@interface MTRNetworkCommissioningClusterConnectNetworkParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRNetworkCommissioningClusterConnectNetworkResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType &)decodableStruct;

@end

@interface MTRNetworkCommissioningClusterReorderNetworkParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRNetworkCommissioningClusterQueryIdentityParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRNetworkCommissioningClusterQueryIdentityResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::NetworkCommissioning::Commands::QueryIdentityResponse::DecodableType &)decodableStruct;

@end

@interface MTRDiagnosticLogsClusterRetrieveLogsRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDiagnosticLogsClusterRetrieveLogsResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsResponse::DecodableType &)decodableStruct;

@end

@interface MTRGeneralDiagnosticsClusterTestEventTriggerParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGeneralDiagnosticsClusterTimeSnapshotParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGeneralDiagnosticsClusterTimeSnapshotResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::GeneralDiagnostics::Commands::TimeSnapshotResponse::DecodableType &)decodableStruct;

@end

@interface MTRGeneralDiagnosticsClusterPayloadTestRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGeneralDiagnosticsClusterPayloadTestResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::GeneralDiagnostics::Commands::PayloadTestResponse::DecodableType &)decodableStruct;

@end

@interface MTRSoftwareDiagnosticsClusterResetWatermarksParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThreadNetworkDiagnosticsClusterResetCountsParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWiFiNetworkDiagnosticsClusterResetCountsParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTREthernetNetworkDiagnosticsClusterResetCountsParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTimeSynchronizationClusterSetUTCTimeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTimeSynchronizationClusterSetTrustedTimeSourceParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTimeSynchronizationClusterSetTimeZoneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTimeSynchronizationClusterSetTimeZoneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::TimeSynchronization::Commands::SetTimeZoneResponse::DecodableType &)decodableStruct;

@end

@interface MTRTimeSynchronizationClusterSetDSTOffsetParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTimeSynchronizationClusterSetDefaultNTPParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRBridgedDeviceBasicInformationClusterKeepActiveParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRAdministratorCommissioningClusterOpenCommissioningWindowParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRAdministratorCommissioningClusterRevokeCommissioningParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalCredentialsClusterAttestationRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalCredentialsClusterAttestationResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::OperationalCredentials::Commands::AttestationResponse::DecodableType &)decodableStruct;

@end

@interface MTROperationalCredentialsClusterCertificateChainRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalCredentialsClusterCertificateChainResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::OperationalCredentials::Commands::CertificateChainResponse::DecodableType &)decodableStruct;

@end

@interface MTROperationalCredentialsClusterCSRRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalCredentialsClusterCSRResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::OperationalCredentials::Commands::CSRResponse::DecodableType &)decodableStruct;

@end

@interface MTROperationalCredentialsClusterAddNOCParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalCredentialsClusterUpdateNOCParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalCredentialsClusterNOCResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::OperationalCredentials::Commands::NOCResponse::DecodableType &)decodableStruct;

@end

@interface MTROperationalCredentialsClusterUpdateFabricLabelParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalCredentialsClusterRemoveFabricParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalCredentialsClusterAddTrustedRootCertificateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalCredentialsClusterSetVidVerificationStatementParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalCredentialsClusterSignVidVerificationRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalCredentialsClusterSignVidVerificationResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::OperationalCredentials::Commands::SignVidVerificationResponse::DecodableType &)decodableStruct;

@end

@interface MTRGroupKeyManagementClusterKeySetWriteParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGroupKeyManagementClusterKeySetReadParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGroupKeyManagementClusterKeySetReadResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadResponse::DecodableType &)decodableStruct;

@end

@interface MTRGroupKeyManagementClusterKeySetRemoveParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGroupKeyManagementClusterKeySetReadAllIndicesParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::DecodableType &)decodableStruct;

@end

@interface MTRICDManagementClusterRegisterClientParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRICDManagementClusterRegisterClientResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::IcdManagement::Commands::RegisterClientResponse::DecodableType &)decodableStruct;

@end

@interface MTRICDManagementClusterUnregisterClientParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRICDManagementClusterStayActiveRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRICDManagementClusterStayActiveResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::IcdManagement::Commands::StayActiveResponse::DecodableType &)decodableStruct;

@end

@interface MTRTimerClusterSetTimerParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTimerClusterResetTimerParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTimerClusterAddTimeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTimerClusterReduceTimeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROvenCavityOperationalStateClusterStopParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROvenCavityOperationalStateClusterStartParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROvenCavityOperationalStateClusterOperationalCommandResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::OvenCavityOperationalState::Commands::OperationalCommandResponse::DecodableType &)decodableStruct;

@end

@interface MTROvenModeClusterChangeToModeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROvenModeClusterChangeToModeResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::OvenMode::Commands::ChangeToModeResponse::DecodableType &)decodableStruct;

@end

@interface MTRModeSelectClusterChangeToModeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRLaundryWasherModeClusterChangeToModeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRLaundryWasherModeClusterChangeToModeResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::LaundryWasherMode::Commands::ChangeToModeResponse::DecodableType &)decodableStruct;

@end

@interface MTRRefrigeratorAndTemperatureControlledCabinetModeClusterChangeToModeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRRefrigeratorAndTemperatureControlledCabinetModeClusterChangeToModeResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Commands::ChangeToModeResponse::DecodableType &)decodableStruct;

@end

@interface MTRRVCRunModeClusterChangeToModeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRRVCRunModeClusterChangeToModeResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::RvcRunMode::Commands::ChangeToModeResponse::DecodableType &)decodableStruct;

@end

@interface MTRRVCCleanModeClusterChangeToModeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRRVCCleanModeClusterChangeToModeResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::RvcCleanMode::Commands::ChangeToModeResponse::DecodableType &)decodableStruct;

@end

@interface MTRTemperatureControlClusterSetTemperatureParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDishwasherModeClusterChangeToModeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDishwasherModeClusterChangeToModeResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::DishwasherMode::Commands::ChangeToModeResponse::DecodableType &)decodableStruct;

@end

@interface MTRSmokeCOAlarmClusterSelfTestRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDishwasherAlarmClusterResetParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDishwasherAlarmClusterModifyEnabledAlarmsParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMicrowaveOvenControlClusterSetCookingParametersParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMicrowaveOvenControlClusterAddMoreTimeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalStateClusterPauseParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalStateClusterStopParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalStateClusterStartParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalStateClusterResumeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTROperationalStateClusterOperationalCommandResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::OperationalState::Commands::OperationalCommandResponse::DecodableType &)decodableStruct;

@end

@interface MTRRVCOperationalStateClusterPauseParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRRVCOperationalStateClusterResumeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRRVCOperationalStateClusterOperationalCommandResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::RvcOperationalState::Commands::OperationalCommandResponse::DecodableType &)decodableStruct;

@end

@interface MTRRVCOperationalStateClusterGoHomeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesManagementClusterAddSceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesManagementClusterAddSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ScenesManagement::Commands::AddSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesManagementClusterViewSceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesManagementClusterViewSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ScenesManagement::Commands::ViewSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesManagementClusterRemoveSceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesManagementClusterRemoveSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ScenesManagement::Commands::RemoveSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesManagementClusterRemoveAllScenesParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesManagementClusterRemoveAllScenesResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ScenesManagement::Commands::RemoveAllScenesResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesManagementClusterStoreSceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesManagementClusterStoreSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ScenesManagement::Commands::StoreSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesManagementClusterRecallSceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesManagementClusterGetSceneMembershipParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesManagementClusterGetSceneMembershipResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ScenesManagement::Commands::GetSceneMembershipResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesManagementClusterCopySceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesManagementClusterCopySceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ScenesManagement::Commands::CopySceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRHEPAFilterMonitoringClusterResetConditionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRActivatedCarbonFilterMonitoringClusterResetConditionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRBooleanStateConfigurationClusterSuppressAlarmParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRBooleanStateConfigurationClusterEnableDisableAlarmParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRValveConfigurationAndControlClusterOpenParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRValveConfigurationAndControlClusterCloseParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWaterHeaterManagementClusterBoostParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWaterHeaterManagementClusterCancelBoostParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDemandResponseLoadControlClusterRegisterLoadControlProgramRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDemandResponseLoadControlClusterUnregisterLoadControlProgramRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDemandResponseLoadControlClusterAddLoadControlEventRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDemandResponseLoadControlClusterRemoveLoadControlEventRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDemandResponseLoadControlClusterClearLoadControlEventsRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMessagesClusterPresentMessagesRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMessagesClusterCancelMessagesRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDeviceEnergyManagementClusterPowerAdjustRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDeviceEnergyManagementClusterCancelPowerAdjustRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDeviceEnergyManagementClusterStartTimeAdjustRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDeviceEnergyManagementClusterPauseRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDeviceEnergyManagementClusterResumeRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDeviceEnergyManagementClusterModifyForecastRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDeviceEnergyManagementClusterRequestConstraintBasedForecastParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDeviceEnergyManagementClusterCancelRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTREnergyEVSEClusterGetTargetsResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::EnergyEvse::Commands::GetTargetsResponse::DecodableType &)decodableStruct;

@end

@interface MTREnergyEVSEClusterDisableParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTREnergyEVSEClusterEnableChargingParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTREnergyEVSEClusterEnableDischargingParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTREnergyEVSEClusterStartDiagnosticsParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTREnergyEVSEClusterSetTargetsParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTREnergyEVSEClusterGetTargetsParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTREnergyEVSEClusterClearTargetsParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTREnergyEVSEModeClusterChangeToModeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTREnergyEVSEModeClusterChangeToModeResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::EnergyEvseMode::Commands::ChangeToModeResponse::DecodableType &)decodableStruct;

@end

@interface MTRWaterHeaterModeClusterChangeToModeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWaterHeaterModeClusterChangeToModeResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::WaterHeaterMode::Commands::ChangeToModeResponse::DecodableType &)decodableStruct;

@end

@interface MTRDeviceEnergyManagementModeClusterChangeToModeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDeviceEnergyManagementModeClusterChangeToModeResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::DeviceEnergyManagementMode::Commands::ChangeToModeResponse::DecodableType &)decodableStruct;

@end

@interface MTRDoorLockClusterLockDoorParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterUnlockDoorParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterUnlockWithTimeoutParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterSetWeekDayScheduleParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterGetWeekDayScheduleParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterGetWeekDayScheduleResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::DoorLock::Commands::GetWeekDayScheduleResponse::DecodableType &)decodableStruct;

@end

@interface MTRDoorLockClusterClearWeekDayScheduleParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterSetYearDayScheduleParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterGetYearDayScheduleParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterGetYearDayScheduleResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::DoorLock::Commands::GetYearDayScheduleResponse::DecodableType &)decodableStruct;

@end

@interface MTRDoorLockClusterClearYearDayScheduleParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterSetHolidayScheduleParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterGetHolidayScheduleParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterGetHolidayScheduleResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::DoorLock::Commands::GetHolidayScheduleResponse::DecodableType &)decodableStruct;

@end

@interface MTRDoorLockClusterClearHolidayScheduleParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterSetUserParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterGetUserParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterGetUserResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::DoorLock::Commands::GetUserResponse::DecodableType &)decodableStruct;

@end

@interface MTRDoorLockClusterClearUserParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterSetCredentialParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterSetCredentialResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::DoorLock::Commands::SetCredentialResponse::DecodableType &)decodableStruct;

@end

@interface MTRDoorLockClusterGetCredentialStatusParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterGetCredentialStatusResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::DoorLock::Commands::GetCredentialStatusResponse::DecodableType &)decodableStruct;

@end

@interface MTRDoorLockClusterClearCredentialParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterUnboltDoorParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterSetAliroReaderConfigParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRDoorLockClusterClearAliroReaderConfigParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWindowCoveringClusterUpOrOpenParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWindowCoveringClusterDownOrCloseParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWindowCoveringClusterStopMotionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWindowCoveringClusterGoToLiftValueParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWindowCoveringClusterGoToLiftPercentageParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWindowCoveringClusterGoToTiltValueParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWindowCoveringClusterGoToTiltPercentageParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRClosureControlClusterStopParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRClosureControlClusterMoveToParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRClosureControlClusterCalibrateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRClosureControlClusterConfigureFallbackParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRClosureControlClusterCancelFallbackParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRServiceAreaClusterSelectAreasParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRServiceAreaClusterSelectAreasResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ServiceArea::Commands::SelectAreasResponse::DecodableType &)decodableStruct;

@end

@interface MTRServiceAreaClusterSkipAreaParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRServiceAreaClusterSkipAreaResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ServiceArea::Commands::SkipAreaResponse::DecodableType &)decodableStruct;

@end

@interface MTRThermostatClusterSetpointRaiseLowerParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThermostatClusterGetWeeklyScheduleResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Thermostat::Commands::GetWeeklyScheduleResponse::DecodableType &)decodableStruct;

@end

@interface MTRThermostatClusterSetWeeklyScheduleParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThermostatClusterGetWeeklyScheduleParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThermostatClusterClearWeeklyScheduleParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThermostatClusterSetActiveScheduleRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThermostatClusterSetActivePresetRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThermostatClusterAtomicResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Thermostat::Commands::AtomicResponse::DecodableType &)decodableStruct;

@end

@interface MTRThermostatClusterAtomicRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRFanControlClusterStepParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterMoveToHueParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterMoveHueParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterStepHueParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterMoveToSaturationParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterMoveSaturationParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterStepSaturationParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterMoveToHueAndSaturationParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterMoveToColorParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterMoveColorParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterStepColorParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterMoveToColorTemperatureParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterEnhancedMoveToHueParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterEnhancedMoveHueParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterEnhancedStepHueParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterEnhancedMoveToHueAndSaturationParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterColorLoopSetParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterStopMoveStepParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterMoveColorTemperatureParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRColorControlClusterStepColorTemperatureParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWiFiNetworkManagementClusterNetworkPassphraseRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWiFiNetworkManagementClusterNetworkPassphraseResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::WiFiNetworkManagement::Commands::NetworkPassphraseResponse::DecodableType &)decodableStruct;

@end

@interface MTRThreadBorderRouterManagementClusterGetActiveDatasetRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThreadBorderRouterManagementClusterGetPendingDatasetRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThreadBorderRouterManagementClusterDatasetResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ThreadBorderRouterManagement::Commands::DatasetResponse::DecodableType &)decodableStruct;

@end

@interface MTRThreadBorderRouterManagementClusterSetActiveDatasetRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThreadBorderRouterManagementClusterSetPendingDatasetRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThreadNetworkDirectoryClusterAddNetworkParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThreadNetworkDirectoryClusterRemoveNetworkParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThreadNetworkDirectoryClusterGetOperationalDatasetParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRThreadNetworkDirectoryClusterOperationalDatasetResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ThreadNetworkDirectory::Commands::OperationalDatasetResponse::DecodableType &)decodableStruct;

@end

@interface MTRChannelClusterChangeChannelParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRChannelClusterChangeChannelResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Channel::Commands::ChangeChannelResponse::DecodableType &)decodableStruct;

@end

@interface MTRChannelClusterChangeChannelByNumberParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRChannelClusterSkipChannelParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRChannelClusterGetProgramGuideParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRChannelClusterProgramGuideResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Channel::Commands::ProgramGuideResponse::DecodableType &)decodableStruct;

@end

@interface MTRChannelClusterRecordProgramParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRChannelClusterCancelRecordProgramParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTargetNavigatorClusterNavigateTargetParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTargetNavigatorClusterNavigateTargetResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::DecodableType &)decodableStruct;

@end

@interface MTRMediaPlaybackClusterPlayParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaPlaybackClusterPauseParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaPlaybackClusterStopParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaPlaybackClusterStartOverParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaPlaybackClusterPreviousParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaPlaybackClusterNextParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaPlaybackClusterRewindParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaPlaybackClusterFastForwardParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaPlaybackClusterSkipForwardParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaPlaybackClusterSkipBackwardParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaPlaybackClusterPlaybackResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType &)decodableStruct;

@end

@interface MTRMediaPlaybackClusterSeekParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaPlaybackClusterActivateAudioTrackParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaPlaybackClusterActivateTextTrackParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaPlaybackClusterDeactivateTextTrackParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaInputClusterSelectInputParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaInputClusterShowInputStatusParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaInputClusterHideInputStatusParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRMediaInputClusterRenameInputParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRLowPowerClusterSleepParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRKeypadInputClusterSendKeyParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRKeypadInputClusterSendKeyResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::KeypadInput::Commands::SendKeyResponse::DecodableType &)decodableStruct;

@end

@interface MTRContentLauncherClusterLaunchContentParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentLauncherClusterLaunchURLParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentLauncherClusterLauncherResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ContentLauncher::Commands::LauncherResponse::DecodableType &)decodableStruct;

@end

@interface MTRAudioOutputClusterSelectOutputParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRAudioOutputClusterRenameOutputParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRApplicationLauncherClusterLaunchAppParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRApplicationLauncherClusterStopAppParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRApplicationLauncherClusterHideAppParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRApplicationLauncherClusterLauncherResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::DecodableType &)decodableStruct;

@end

@interface MTRAccountLoginClusterGetSetupPINParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRAccountLoginClusterGetSetupPINResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::AccountLogin::Commands::GetSetupPINResponse::DecodableType &)decodableStruct;

@end

@interface MTRAccountLoginClusterLoginParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRAccountLoginClusterLogoutParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentControlClusterUpdatePINParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentControlClusterResetPINParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentControlClusterResetPINResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ContentControl::Commands::ResetPINResponse::DecodableType &)decodableStruct;

@end

@interface MTRContentControlClusterEnableParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentControlClusterDisableParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentControlClusterAddBonusTimeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentControlClusterSetScreenDailyTimeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentControlClusterBlockUnratedContentParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentControlClusterUnblockUnratedContentParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentControlClusterSetOnDemandRatingThresholdParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentControlClusterSetScheduledContentRatingThresholdParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentAppObserverClusterContentAppMessageParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRContentAppObserverClusterContentAppMessageResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ContentAppObserver::Commands::ContentAppMessageResponse::DecodableType &)decodableStruct;

@end

@interface MTRZoneManagementClusterCreateTwoDCartesianZoneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRZoneManagementClusterCreateTwoDCartesianZoneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ZoneManagement::Commands::CreateTwoDCartesianZoneResponse::DecodableType &)decodableStruct;

@end

@interface MTRZoneManagementClusterUpdateTwoDCartesianZoneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRZoneManagementClusterGetTwoDCartesianZoneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRZoneManagementClusterGetTwoDCartesianZoneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ZoneManagement::Commands::GetTwoDCartesianZoneResponse::DecodableType &)decodableStruct;

@end

@interface MTRZoneManagementClusterRemoveZoneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVStreamManagementClusterAudioStreamAllocateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVStreamManagementClusterAudioStreamAllocateResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::CameraAvStreamManagement::Commands::AudioStreamAllocateResponse::DecodableType &)decodableStruct;

@end

@interface MTRCameraAVStreamManagementClusterAudioStreamDeallocateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVStreamManagementClusterVideoStreamAllocateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVStreamManagementClusterVideoStreamAllocateResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::CameraAvStreamManagement::Commands::VideoStreamAllocateResponse::DecodableType &)decodableStruct;

@end

@interface MTRCameraAVStreamManagementClusterVideoStreamModifyParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVStreamManagementClusterVideoStreamDeallocateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVStreamManagementClusterSnapshotStreamAllocateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVStreamManagementClusterSnapshotStreamAllocateResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::CameraAvStreamManagement::Commands::SnapshotStreamAllocateResponse::DecodableType &)decodableStruct;

@end

@interface MTRCameraAVStreamManagementClusterSnapshotStreamModifyParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVStreamManagementClusterSnapshotStreamDeallocateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVStreamManagementClusterSetStreamPrioritiesParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVStreamManagementClusterCaptureSnapshotParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVStreamManagementClusterCaptureSnapshotResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::CameraAvStreamManagement::Commands::CaptureSnapshotResponse::DecodableType &)decodableStruct;

@end

@interface MTRCameraAVSettingsUserLevelManagementClusterMPTZSetPositionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVSettingsUserLevelManagementClusterMPTZRelativeMoveParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVSettingsUserLevelManagementClusterMPTZMoveToPresetParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVSettingsUserLevelManagementClusterMPTZSavePresetParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVSettingsUserLevelManagementClusterMPTZRemovePresetParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVSettingsUserLevelManagementClusterDPTZSetViewportParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCameraAVSettingsUserLevelManagementClusterDPTZRelativeMoveParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWebRTCTransportProviderClusterSolicitOfferParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWebRTCTransportProviderClusterSolicitOfferResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::WebRTCTransportProvider::Commands::SolicitOfferResponse::DecodableType &)decodableStruct;

@end

@interface MTRWebRTCTransportProviderClusterProvideOfferParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWebRTCTransportProviderClusterProvideOfferResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::WebRTCTransportProvider::Commands::ProvideOfferResponse::DecodableType &)decodableStruct;

@end

@interface MTRWebRTCTransportProviderClusterProvideAnswerParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWebRTCTransportProviderClusterProvideICECandidatesParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWebRTCTransportProviderClusterEndSessionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWebRTCTransportRequestorClusterOfferParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWebRTCTransportRequestorClusterAnswerParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWebRTCTransportRequestorClusterICECandidatesParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRWebRTCTransportRequestorClusterEndParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRPushAVStreamTransportClusterAllocatePushTransportParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRPushAVStreamTransportClusterAllocatePushTransportResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::PushAvStreamTransport::Commands::AllocatePushTransportResponse::DecodableType &)decodableStruct;

@end

@interface MTRPushAVStreamTransportClusterDeallocatePushTransportParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRPushAVStreamTransportClusterModifyPushTransportParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRPushAVStreamTransportClusterSetTransportStatusParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRPushAVStreamTransportClusterManuallyTriggerTransportParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRPushAVStreamTransportClusterFindTransportParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRPushAVStreamTransportClusterFindTransportResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::PushAvStreamTransport::Commands::FindTransportResponse::DecodableType &)decodableStruct;

@end

@interface MTRChimeClusterPlayChimeSoundParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCommissionerControlClusterRequestCommissioningApprovalParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCommissionerControlClusterCommissionNodeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRCommissionerControlClusterReverseOpenCommissioningWindowParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::CommissionerControl::Commands::ReverseOpenCommissioningWindow::DecodableType &)decodableStruct;

@end

@interface MTRTLSCertificateManagementClusterProvisionRootCertificateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTLSCertificateManagementClusterProvisionRootCertificateResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::TlsCertificateManagement::Commands::ProvisionRootCertificateResponse::DecodableType &)decodableStruct;

@end

@interface MTRTLSCertificateManagementClusterFindRootCertificateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTLSCertificateManagementClusterFindRootCertificateResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::TlsCertificateManagement::Commands::FindRootCertificateResponse::DecodableType &)decodableStruct;

@end

@interface MTRTLSCertificateManagementClusterLookupRootCertificateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTLSCertificateManagementClusterLookupRootCertificateResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::TlsCertificateManagement::Commands::LookupRootCertificateResponse::DecodableType &)decodableStruct;

@end

@interface MTRTLSCertificateManagementClusterRemoveRootCertificateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTLSCertificateManagementClusterTLSClientCSRParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTLSCertificateManagementClusterTLSClientCSRResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::TlsCertificateManagement::Commands::TLSClientCSRResponse::DecodableType &)decodableStruct;

@end

@interface MTRTLSCertificateManagementClusterProvisionClientCertificateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTLSCertificateManagementClusterFindClientCertificateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTLSCertificateManagementClusterFindClientCertificateResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::TlsCertificateManagement::Commands::FindClientCertificateResponse::DecodableType &)decodableStruct;

@end

@interface MTRTLSCertificateManagementClusterLookupClientCertificateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTLSCertificateManagementClusterLookupClientCertificateResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::TlsCertificateManagement::Commands::LookupClientCertificateResponse::DecodableType &)decodableStruct;

@end

@interface MTRTLSCertificateManagementClusterRemoveClientCertificateParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTLSClientManagementClusterProvisionEndpointParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTLSClientManagementClusterProvisionEndpointResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::TlsClientManagement::Commands::ProvisionEndpointResponse::DecodableType &)decodableStruct;

@end

@interface MTRTLSClientManagementClusterFindEndpointParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRTLSClientManagementClusterFindEndpointResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::TlsClientManagement::Commands::FindEndpointResponse::DecodableType &)decodableStruct;

@end

@interface MTRTLSClientManagementClusterRemoveEndpointParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestSpecificResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::TestSpecificResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestNotHandledParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestAddArgumentsResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::TestAddArgumentsResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestSpecificParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestSimpleArgumentResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::TestSimpleArgumentResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestUnknownCommandParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestStructArrayArgumentResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::TestStructArrayArgumentResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestAddArgumentsParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestListInt8UReverseResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::TestListInt8UReverseResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestSimpleArgumentRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestEnumsResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::TestEnumsResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestStructArrayArgumentRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestNullableOptionalResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::TestNullableOptionalResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestStructArgumentRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestComplexNullableOptionalResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::TestComplexNullableOptionalResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestNestedStructArgumentRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterBooleanResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::BooleanResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestListStructArgumentRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterSimpleStructResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::SimpleStructResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestListInt8UArgumentRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestEmitTestEventResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::TestEmitTestEventResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestNestedStructListArgumentRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::TestEmitTestFabricScopedEventResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestListNestedStructListArgumentRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestBatchHelperResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::TestBatchHelperResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestListInt8UReverseRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterStringEchoResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::StringEchoResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestEnumsRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterGlobalEchoResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::GlobalEchoResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestNullableOptionalRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestComplexNullableOptionalRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterSimpleStructEchoRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTimedInvokeRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestSimpleOptionalArgumentRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestEmitTestEventRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestEmitTestFabricScopedEventRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestBatchHelperRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestSecondBatchHelperRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterStringEchoRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterGlobalEchoRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestDifferentVendorMeiRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestDifferentVendorMeiResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::UnitTesting::Commands::TestDifferentVendorMeiResponse::DecodableType &)decodableStruct;

@end

@interface MTRSampleMEIClusterPingParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRSampleMEIClusterAddArgumentsResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::SampleMei::Commands::AddArgumentsResponse::DecodableType &)decodableStruct;

@end

@interface MTRSampleMEIClusterAddArgumentsParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

// Make sure that MTRBasicClusterMfgSpecificPingParams has _encodeAsDataValue just so it compiles.
@interface MTRBasicClusterMfgSpecificPingParams (InternalMethods)
- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;
@end

NS_ASSUME_NONNULL_END
