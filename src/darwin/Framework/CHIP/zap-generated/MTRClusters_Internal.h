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

#import <Foundation/Foundation.h>

#import "MTRClusters.h"
#import "MTRDevice.h"
#import "MTRDevice_Internal.h"

@interface MTRClusterIdentify ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterGroups ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterScenes ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOnOff ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOnOffSwitchConfiguration ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterLevelControl ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBinaryInputBasic ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPulseWidthModulation ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterDescriptor ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBinding ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterAccessControl ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterActions ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBasicInformation ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOTASoftwareUpdateProvider ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOTASoftwareUpdateRequestor ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterLocalizationConfiguration ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterTimeFormatLocalization ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterUnitLocalization ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPowerSourceConfiguration ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPowerSource ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterGeneralCommissioning ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterNetworkCommissioning ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterDiagnosticLogs ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterGeneralDiagnostics ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterSoftwareDiagnostics ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterThreadNetworkDiagnostics ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterWiFiNetworkDiagnostics ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterEthernetNetworkDiagnostics ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterTimeSynchronization ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBridgedDeviceBasicInformation ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterSwitch ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterAdministratorCommissioning ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOperationalCredentials ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterGroupKeyManagement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterFixedLabel ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterUserLabel ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBooleanState ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterICDManagement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterModeSelect ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterLaundryWasherMode ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterRefrigeratorAndTemperatureControlledCabinetMode ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterLaundryWasherControls ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterRVCRunMode ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterRVCCleanMode ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterTemperatureControl ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterRefrigeratorAlarm ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterDishwasherMode ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterAirQuality ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterSmokeCOAlarm ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterDishwasherAlarm ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOperationalState ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterRVCOperationalState ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterHEPAFilterMonitoring ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterActivatedCarbonFilterMonitoring ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterDoorLock ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterWindowCovering ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBarrierControl ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPumpConfigurationAndControl ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterThermostat ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterFanControl ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterThermostatUserInterfaceConfiguration ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterColorControl ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBallastConfiguration ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterIlluminanceMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterTemperatureMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPressureMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterFlowMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterRelativeHumidityMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOccupancySensing ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterCarbonMonoxideConcentrationMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterCarbonDioxideConcentrationMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterNitrogenDioxideConcentrationMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOzoneConcentrationMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPM25ConcentrationMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterFormaldehydeConcentrationMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPM1ConcentrationMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPM10ConcentrationMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterTotalVolatileOrganicCompoundsConcentrationMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterRadonConcentrationMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterWakeOnLAN ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterChannel ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterTargetNavigator ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterMediaPlayback ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterMediaInput ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterLowPower ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterKeypadInput ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterContentLauncher ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterAudioOutput ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterApplicationLauncher ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterApplicationBasic ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterAccountLogin ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterElectricalMeasurement ()
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterUnitTesting ()
@property (nonatomic, readonly) MTRDevice * device;
@end
