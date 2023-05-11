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

#include <zap-generated/CHIPClusters.h>

@interface MTRClusterIdentify ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterGroups ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterScenes ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOnOff ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOnOffSwitchConfiguration ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterLevelControl ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBinaryInputBasic ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterDescriptor ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBinding ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterAccessControl ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterActions ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBasicInformation ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOTASoftwareUpdateProvider ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOTASoftwareUpdateRequestor ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterLocalizationConfiguration ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterTimeFormatLocalization ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterUnitLocalization ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPowerSourceConfiguration ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPowerSource ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterGeneralCommissioning ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterNetworkCommissioning ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterDiagnosticLogs ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterGeneralDiagnostics ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterSoftwareDiagnostics ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterThreadNetworkDiagnostics ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterWiFiNetworkDiagnostics ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterEthernetNetworkDiagnostics ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBridgedDeviceBasicInformation ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterSwitch ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterAdministratorCommissioning ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOperationalCredentials ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterGroupKeyManagement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterFixedLabel ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterUserLabel ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBooleanState ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterModeSelect ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterDoorLock ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterWindowCovering ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBarrierControl ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPumpConfigurationAndControl ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterThermostat ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterFanControl ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterThermostatUserInterfaceConfiguration ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterColorControl ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBallastConfiguration ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterIlluminanceMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterTemperatureMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPressureMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterFlowMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterRelativeHumidityMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOccupancySensing ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterCarbonMonoxideMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterCarbonDioxideMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterEthyleneMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterEthyleneOxideMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterHydrogenMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterHydrogenSulfideMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterNitricOxideMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterNitrogenDioxideMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOxygenMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterOzoneMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterSulfurDioxideMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterDissolvedOxygenMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBromateMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterChloraminesMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterChlorineMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterFecalColiformEColiMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterFluorideMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterHaloaceticAcidsMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterTotalTrihalomethanesMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterTotalColiformBacteriaMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterTurbidityMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterCopperMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterLeadMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterManganeseMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterSulfateMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBromodichloromethaneMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterBromoformMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterChlorodibromomethaneMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterChloroformMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterSodiumMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPM25Measurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterFormaldehydeMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPM1Measurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterPM10Measurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterTotalVolatileOrganicCompoundsMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterRadonMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterWakeOnLAN ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterChannel ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterTargetNavigator ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterMediaPlayback ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterMediaInput ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterLowPower ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterKeypadInput ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterContentLauncher ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterAudioOutput ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterApplicationLauncher ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterApplicationBasic ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterAccountLogin ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterElectricalMeasurement ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end

@interface MTRClusterUnitTesting ()
@property (nonatomic, readonly) uint16_t endpoint;
@property (nonatomic, readonly) MTRDevice * device;
@end
