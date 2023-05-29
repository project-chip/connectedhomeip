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

#import <Matter/MTRDefines.h>

#include <app-common/zap-generated/cluster-objects.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRGroupsClusterAddGroupResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Groups::Commands::AddGroupResponse::DecodableType &)decodableStruct;

@end

@interface MTRGroupsClusterViewGroupResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Groups::Commands::ViewGroupResponse::DecodableType &)decodableStruct;

@end

@interface MTRGroupsClusterGetGroupMembershipResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Groups::Commands::GetGroupMembershipResponse::DecodableType &)decodableStruct;

@end

@interface MTRGroupsClusterRemoveGroupResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Groups::Commands::RemoveGroupResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterAddSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::AddSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterViewSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::ViewSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterRemoveSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::RemoveSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterRemoveAllScenesResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::RemoveAllScenesResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterStoreSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::StoreSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterGetSceneMembershipResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::GetSceneMembershipResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterEnhancedAddSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::EnhancedAddSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterEnhancedViewSceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::EnhancedViewSceneResponse::DecodableType &)decodableStruct;

@end

@interface MTRScenesClusterCopySceneResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Scenes::Commands::CopySceneResponse::DecodableType &)decodableStruct;

@end

@interface MTROTASoftwareUpdateProviderClusterQueryImageResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::DecodableType &)decodableStruct;

@end

@interface MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::DecodableType &)decodableStruct;

@end

@interface MTRGeneralCommissioningClusterArmFailSafeResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType &)decodableStruct;

@end

@interface MTRGeneralCommissioningClusterSetRegulatoryConfigResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::GeneralCommissioning::Commands::SetRegulatoryConfigResponse::DecodableType &)decodableStruct;

@end

@interface MTRGeneralCommissioningClusterCommissioningCompleteResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType &)decodableStruct;

@end

@interface MTRNetworkCommissioningClusterScanNetworksResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType &)decodableStruct;

@end

@interface MTRNetworkCommissioningClusterNetworkConfigResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType &)decodableStruct;

@end

@interface MTRNetworkCommissioningClusterConnectNetworkResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType &)decodableStruct;

@end

@interface MTRDiagnosticLogsClusterRetrieveLogsResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DiagnosticLogs::Commands::RetrieveLogsResponse::DecodableType &)decodableStruct;

@end

@interface MTROperationalCredentialsClusterAttestationResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OperationalCredentials::Commands::AttestationResponse::DecodableType &)decodableStruct;

@end

@interface MTROperationalCredentialsClusterCertificateChainResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OperationalCredentials::Commands::CertificateChainResponse::DecodableType &)decodableStruct;

@end

@interface MTROperationalCredentialsClusterCSRResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OperationalCredentials::Commands::CSRResponse::DecodableType &)decodableStruct;

@end

@interface MTROperationalCredentialsClusterNOCResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OperationalCredentials::Commands::NOCResponse::DecodableType &)decodableStruct;

@end

@interface MTRGroupKeyManagementClusterKeySetReadResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadResponse::DecodableType &)decodableStruct;

@end

@interface MTRGroupKeyManagementClusterKeySetReadAllIndicesResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::DecodableType &)decodableStruct;

@end

@interface MTROperationalStateClusterOperationalCommandResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::OperationalState::Commands::OperationalCommandResponse::DecodableType &)decodableStruct;

@end

@interface MTRDoorLockClusterGetWeekDayScheduleResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DoorLock::Commands::GetWeekDayScheduleResponse::DecodableType &)decodableStruct;

@end

@interface MTRDoorLockClusterGetYearDayScheduleResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DoorLock::Commands::GetYearDayScheduleResponse::DecodableType &)decodableStruct;

@end

@interface MTRDoorLockClusterGetHolidayScheduleResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DoorLock::Commands::GetHolidayScheduleResponse::DecodableType &)decodableStruct;

@end

@interface MTRDoorLockClusterGetUserResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DoorLock::Commands::GetUserResponse::DecodableType &)decodableStruct;

@end

@interface MTRDoorLockClusterSetCredentialResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DoorLock::Commands::SetCredentialResponse::DecodableType &)decodableStruct;

@end

@interface MTRDoorLockClusterGetCredentialStatusResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::DoorLock::Commands::GetCredentialStatusResponse::DecodableType &)decodableStruct;

@end

@interface MTRThermostatClusterGetWeeklyScheduleResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Thermostat::Commands::GetWeeklyScheduleResponse::DecodableType &)decodableStruct;

@end

@interface MTRChannelClusterChangeChannelResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::Channel::Commands::ChangeChannelResponse::DecodableType &)decodableStruct;

@end

@interface MTRTargetNavigatorClusterNavigateTargetResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::DecodableType &)decodableStruct;

@end

@interface MTRMediaPlaybackClusterPlaybackResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::DecodableType &)decodableStruct;

@end

@interface MTRKeypadInputClusterSendKeyResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::KeypadInput::Commands::SendKeyResponse::DecodableType &)decodableStruct;

@end

@interface MTRContentLauncherClusterLauncherResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::ContentLauncher::Commands::LauncherResponse::DecodableType &)decodableStruct;

@end

@interface MTRApplicationLauncherClusterLauncherResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::ApplicationLauncher::Commands::LauncherResponse::DecodableType &)decodableStruct;

@end

@interface MTRAccountLoginClusterGetSetupPINResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::AccountLogin::Commands::GetSetupPINResponse::DecodableType &)decodableStruct;

@end

@interface MTRElectricalMeasurementClusterGetProfileInfoResponseCommandParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::ElectricalMeasurement::Commands::GetProfileInfoResponseCommand::DecodableType &)decodableStruct;

@end

@interface MTRElectricalMeasurementClusterGetMeasurementProfileResponseCommandParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::ElectricalMeasurement::Commands::GetMeasurementProfileResponseCommand::DecodableType &)
        decodableStruct;

@end

@interface MTRUnitTestingClusterTestSpecificResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestSpecificResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestAddArgumentsResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestAddArgumentsResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestSimpleArgumentResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestSimpleArgumentResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestStructArrayArgumentResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestStructArrayArgumentResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestListInt8UReverseResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestListInt8UReverseResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestEnumsResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestEnumsResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestNullableOptionalResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestNullableOptionalResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestComplexNullableOptionalResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestComplexNullableOptionalResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterBooleanResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::BooleanResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterSimpleStructResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::SimpleStructResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestEmitTestEventResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestEmitTestEventResponse::DecodableType &)decodableStruct;

@end

@interface MTRUnitTestingClusterTestEmitTestFabricScopedEventResponseParams (InternalMethods)

- (CHIP_ERROR)_setFieldsFromDecodableStruct:
    (const chip::app::Clusters::UnitTesting::Commands::TestEmitTestFabricScopedEventResponse::DecodableType &)decodableStruct;

@end

NS_ASSUME_NONNULL_END