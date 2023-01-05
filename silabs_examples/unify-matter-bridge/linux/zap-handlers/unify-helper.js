
// Match supportedMappedCluster but with codes instead of names
function supportedMappedClusterCode(code) {
  switch (code) {
    case 0: return true // Cluster Basic
    // case 1: return true // Cluster PowerConfiguration
    // case 2: return true // Cluster DeviceTemperatureConfiguration
    case 3: return true // Cluster Identify
    case 4: return true // Cluster Groups
    case 5: return true // Cluster Scenes
    case 6: return true // Cluster OnOff
    case 8: return true // Cluster Level
    case 9: return true // Cluster Alarms
    case 10: return true // Cluster Time
    // case 21: return true // Cluster Commissioning
    // case 25: return true // Cluster OTAUpgrade
    // case 32: return true // Cluster PollControl
    case 57: return true // Cluster BridgeBasic
    // case 256: return true // Cluster ShadeConfiguration
    case 257: return true // Cluster DoorLock
    // case 258: return true // Cluster WindowCovering
    case 259: return true // Cluster BarrierControl
    // case 512: return true // Cluster PumpConfigurationAndControl
    case 513: return true // Cluster Thermostat
    case 514: return true // Cluster FanControl
    // case 515: return true // Cluster DehumidificationControl
    case 516: return true // Cluster ThermostatUserInterfaceConfiguration
    case 768: return true // Cluster ColorControl
    // case 769: return true // Cluster BallastConfiguration
    case 1024: return true // Cluster IlluminanceMeasurement
    // case 1025: return true // Cluster IlluminanceLevelSensing
    case 1026: return true // Cluster TemperatureMeasurement
    case 1027: return true // Cluster PressureMeasurement
    // case 1028: return true // Cluster FlowMeasurement
    case 1029: return true // Cluster RelativityHumidity
    case 1030: return true // Cluster OccupancySensing
    // case 1033: return true // Cluster PhMeasurement
    // case 1034: return true // Cluster ElectricalConductivityMeasurement
    // case 1035: return true // Cluster WindSpeedMeasurement
    // case 1036: return true // Cluster CarbonMonoxide
    // case 1280: return true // Cluster IASZone
    // case 1281: return true // Cluster IASACE
    // case 1282: return true // Cluster IASWD
    case 2820: return true // Cluster ElectricalMeasurement
    // case 2821: return true // Cluster Diagnostics
    // case 4096: return true // Cluster TouchlinkCommissioning

    default: return false
  }
}

exports.supportedMappedClusterCode = supportedMappedClusterCode