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


private let clusterQueue = DispatchQueue(label: "MTRCluster completion dispatch")

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterIdentify {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterGroups {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterScenes {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterOnOff {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterOnOffSwitchConfiguration {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterLevelControl {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterBinaryInputBasic {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterPulseWidthModulation {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterDescriptor {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterBinding {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterAccessControl {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterActions {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterBasicInformation {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterOTASoftwareUpdateProvider {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterOTASoftwareUpdateRequestor {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterLocalizationConfiguration {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterTimeFormatLocalization {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterUnitLocalization {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterPowerSourceConfiguration {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterPowerSource {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterGeneralCommissioning {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterNetworkCommissioning {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterDiagnosticLogs {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterGeneralDiagnostics {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterSoftwareDiagnostics {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterThreadNetworkDiagnostics {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterWiFiNetworkDiagnostics {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterEthernetNetworkDiagnostics {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterTimeSynchronization {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterBridgedDeviceBasicInformation {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterSwitch {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterAdministratorCommissioning {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterOperationalCredentials {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterGroupKeyManagement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterFixedLabel {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterUserLabel {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterBooleanState {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterICDManagement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterModeSelect {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterLaundryWasherMode {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterRefrigeratorAndTemperatureControlledCabinetMode {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterLaundryWasherControls {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterRVCRunMode {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterRVCCleanMode {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterTemperatureControl {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterRefrigeratorAlarm {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterDishwasherMode {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterAirQuality {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterSmokeCOAlarm {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterDishwasherAlarm {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterOperationalState {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterRVCOperationalState {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterHEPAFilterMonitoring {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterActivatedCarbonFilterMonitoring {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterDoorLock {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterWindowCovering {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterBarrierControl {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterPumpConfigurationAndControl {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterThermostat {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterFanControl {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterThermostatUserInterfaceConfiguration {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterColorControl {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterBallastConfiguration {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterIlluminanceMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterTemperatureMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterPressureMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterFlowMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterRelativeHumidityMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterOccupancySensing {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterCarbonMonoxideConcentrationMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterCarbonDioxideConcentrationMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterNitrogenDioxideConcentrationMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterOzoneConcentrationMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterPM25ConcentrationMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterFormaldehydeConcentrationMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterPM1ConcentrationMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterPM10ConcentrationMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterTotalVolatileOrganicCompoundsConcentrationMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterRadonConcentrationMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterWakeOnLAN {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterChannel {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterTargetNavigator {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterMediaPlayback {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterMediaInput {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterLowPower {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterKeypadInput {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterContentLauncher {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterAudioOutput {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterApplicationLauncher {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterApplicationBasic {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterAccountLogin {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterElectricalMeasurement {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS 17.2, macOS 14.2, watchOS 10.2, tvOS 17.2, *)
extension MTRClusterUnitTesting {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

@available(iOS, unavailable) @available(macOS, unavailable) @available(tvOS, unavailable) @available(watchOS, unavailable)
extension MTRClusterSampleMEI {
    public convenience init(device : MTRDevice, endpointID: UInt) {
        self.init(device: device, endpointID: endpointID as NSNumber, queue: clusterQueue)!
    }
}

