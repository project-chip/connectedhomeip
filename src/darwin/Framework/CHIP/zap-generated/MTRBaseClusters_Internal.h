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

#include <zap-generated/CHIPClusters.h>

@interface MTRBaseClusterIdentify ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterGroups ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterScenes ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterOnOff ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterOnOffSwitchConfiguration ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterLevelControl ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterBinaryInputBasic ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterPulseWidthModulation ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterDescriptor ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterBinding ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterAccessControl ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterActions ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterBasicInformation ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterOTASoftwareUpdateProvider ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterOTASoftwareUpdateRequestor ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterLocalizationConfiguration ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterTimeFormatLocalization ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterUnitLocalization ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterPowerSourceConfiguration ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterPowerSource ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterGeneralCommissioning ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterNetworkCommissioning ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterDiagnosticLogs ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterGeneralDiagnostics ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterSoftwareDiagnostics ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterThreadNetworkDiagnostics ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterWiFiNetworkDiagnostics ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterEthernetNetworkDiagnostics ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterTimeSynchronization ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterBridgedDeviceBasicInformation ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterSwitch ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterAdministratorCommissioning ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterOperationalCredentials ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterGroupKeyManagement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterFixedLabel ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterUserLabel ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterProxyConfiguration ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterProxyDiscovery ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterProxyValid ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterBooleanState ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterModeSelect ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterDoorLock ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterWindowCovering ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterBarrierControl ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterPumpConfigurationAndControl ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterThermostat ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterFanControl ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterThermostatUserInterfaceConfiguration ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterColorControl ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterBallastConfiguration ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterIlluminanceMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterTemperatureMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterPressureMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterFlowMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterRelativeHumidityMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterOccupancySensing ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterWakeOnLAN ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterChannel ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterTargetNavigator ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterMediaPlayback ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterMediaInput ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterLowPower ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterKeypadInput ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterContentLauncher ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterAudioOutput ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterApplicationLauncher ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterApplicationBasic ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterAccountLogin ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterElectricalMeasurement ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterClientMonitoring ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterUnitTesting ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end

@interface MTRBaseClusterFaultInjection ()
@property (nonatomic, strong, readonly) MTRBaseDevice * device;
@property (nonatomic, assign, readonly) chip::EndpointId endpoint;
@end
