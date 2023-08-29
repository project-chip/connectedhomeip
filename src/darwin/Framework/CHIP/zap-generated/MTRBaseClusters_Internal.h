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

#import "MTRBaseClusters.h"
#import "MTRBaseDevice.h"

@interface MTRBaseClusterIdentify ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterGroups ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterScenes ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterOnOff ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterOnOffSwitchConfiguration ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterLevelControl ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterBinaryInputBasic ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterPulseWidthModulation ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterDescriptor ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterBinding ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterAccessControl ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterActions ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterBasicInformation ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterOTASoftwareUpdateProvider ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterOTASoftwareUpdateRequestor ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterLocalizationConfiguration ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterTimeFormatLocalization ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterUnitLocalization ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterPowerSourceConfiguration ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterPowerSource ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterGeneralCommissioning ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterNetworkCommissioning ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterDiagnosticLogs ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterGeneralDiagnostics ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterSoftwareDiagnostics ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterThreadNetworkDiagnostics ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterWiFiNetworkDiagnostics ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterEthernetNetworkDiagnostics ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterTimeSynchronization ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterBridgedDeviceBasicInformation ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterSwitch ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterAdministratorCommissioning ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterOperationalCredentials ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterGroupKeyManagement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterFixedLabel ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterUserLabel ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterBooleanState ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterICDManagement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterModeSelect ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterLaundryWasherMode ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterRefrigeratorAndTemperatureControlledCabinetMode ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterLaundryWasherControls ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterRVCRunMode ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterRVCCleanMode ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterTemperatureControl ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterRefrigeratorAlarm ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterDishwasherMode ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterAirQuality ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterSmokeCOAlarm ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterDishwasherAlarm ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterOperationalState ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterRVCOperationalState ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterHEPAFilterMonitoring ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterActivatedCarbonFilterMonitoring ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterDoorLock ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterWindowCovering ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterBarrierControl ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterPumpConfigurationAndControl ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterThermostat ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterFanControl ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterThermostatUserInterfaceConfiguration ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterColorControl ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterBallastConfiguration ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterIlluminanceMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterTemperatureMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterPressureMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterFlowMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterRelativeHumidityMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterOccupancySensing ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterCarbonMonoxideConcentrationMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterCarbonDioxideConcentrationMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterNitrogenDioxideConcentrationMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterOzoneConcentrationMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterPM25ConcentrationMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterFormaldehydeConcentrationMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterPM1ConcentrationMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterPM10ConcentrationMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterTotalVolatileOrganicCompoundsConcentrationMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterRadonConcentrationMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterWakeOnLAN ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterChannel ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterTargetNavigator ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterMediaPlayback ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterMediaInput ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterLowPower ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterKeypadInput ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterContentLauncher ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterAudioOutput ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterApplicationLauncher ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterApplicationBasic ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterAccountLogin ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterElectricalMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterUnitTesting ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end

@interface MTRBaseClusterSampleMEI ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@end
