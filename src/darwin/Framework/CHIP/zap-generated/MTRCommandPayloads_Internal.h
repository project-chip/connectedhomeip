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

@interface MTRScenesClusterAddSceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesClusterAddSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Scenes::Commands::AddSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterViewSceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesClusterViewSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Scenes::Commands::ViewSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterRemoveSceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesClusterRemoveSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Scenes::Commands::RemoveSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterRemoveAllScenesParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesClusterRemoveAllScenesResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Scenes::Commands::RemoveAllScenesResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterStoreSceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesClusterStoreSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Scenes::Commands::StoreSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterRecallSceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesClusterGetSceneMembershipParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesClusterGetSceneMembershipResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Scenes::Commands::GetSceneMembershipResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterEnhancedAddSceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesClusterEnhancedAddSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Scenes::Commands::EnhancedAddSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterEnhancedViewSceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesClusterEnhancedViewSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Scenes::Commands::EnhancedViewSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterCopySceneParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRScenesClusterCopySceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::Scenes::Commands::CopySceneResponse::DecodableType &)decodableStruct;

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

@interface MTRRVCOperationalStateClusterStopParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRRVCOperationalStateClusterStartParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRRVCOperationalStateClusterResumeParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRRVCOperationalStateClusterOperationalCommandResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::RvcOperationalState::Commands::OperationalCommandResponse::DecodableType &)decodableStruct;

@end

@interface MTRHEPAFilterMonitoringClusterResetConditionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRActivatedCarbonFilterMonitoringClusterResetConditionParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRBooleanSensorConfigurationClusterSuppressRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRValveConfigurationAndControlClusterOpenParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRValveConfigurationAndControlClusterCloseParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRValveConfigurationAndControlClusterSetLevelParams (InternalMethods)

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

@interface MTRBarrierControlClusterBarrierControlGoToPercentParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRBarrierControlClusterBarrierControlStopParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

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

@interface MTRElectricalMeasurementClusterGetProfileInfoResponseCommandParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ElectricalMeasurement::Commands::GetProfileInfoResponseCommand::DecodableType &)decodableStruct;

@end

@interface MTRElectricalMeasurementClusterGetProfileInfoCommandParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRElectricalMeasurementClusterGetMeasurementProfileResponseCommandParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:(const chip::app::Clusters::ElectricalMeasurement::Commands::GetMeasurementProfileResponseCommand::DecodableType &)decodableStruct;

@end

@interface MTRElectricalMeasurementClusterGetMeasurementProfileCommandParams (InternalMethods)

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

@interface MTRUnitTestingClusterTestListInt8UReverseRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

@end

@interface MTRUnitTestingClusterTestEnumsRequestParams (InternalMethods)

- (NSDictionary<NSString *, id> * _Nullable)_encodeAsDataValue:(NSError * __autoreleasing *)error;

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