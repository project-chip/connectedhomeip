
let pwd = process.env.PWD
// let zapPath = pwd + '/../third_party/connectedhomeip/third_party/zap/repo/src-electron/'
// const cHelper = require(zapPath + 'generator/helper-c.js')

function toCamelCase(label, firstLower = true) {
  let str = label.replace(/[+()&]/g, '').split(/ |_|-|\//)
  let res = ''
  for (let i = 0; i < str.length; i++) {
    if (i == 0 && firstLower) {
      res += str[i].charAt(0).toLowerCase()
    } else {
      res += str[i].charAt(0).toUpperCase()
    }
    // Acronyms, such as ZLL become lower-case.
    if (str[i].length > 1 && str[i].toUpperCase() == str[i]) {
      res += str[i].substring(1).toLowerCase()
    } else {
      res += str[i].substring(1)
    }
  }
  return res
}

// Only lowercase some acronyms not all, thanks for the consistency matter.
function lowerAcronyms(label) {
  if (['RFID', 'PIN', "LED"].some(element => label.includes(element))) {
    return label
  }

  return label.replace(/(?<=[A-Z])[A-Z]+(?=[A-Z])/g, (c) => c.toLowerCase())
}

function asUpperCamelCase(label)
{
  let str = toCamelCase(label, false)
  str = lowerAcronyms(label);
  return str.replace(/[^A-Za-z0-9_]/g, '');
}

// These clusters are not supported directly translating from Unify -> Matter
function chipSupportedCluster(clusterName) {
  let clusters = {
   "Identify":{}, 
   "Groups":{}, 
   "Scenes":{}, 
   "OnOff":{}, 
   "Level":{}, 
   //"DoorLock":{}, 
   "FanControl":{}, 
   "ColorControl":{}, 
   "IlluminanceMeasurement":{}, 
   "TemperatureMeasurement":{}, 
   "PressureMeasurement":{}, 
   "FlowMeasurement":{}, 
   "OccupancySensing":{}, 
   "IasZone":{}, 
   "IaswD":{},
   "Basic": {}, 
    };
   return clusterName in clusters
}

// These attributes can't be translated from Unify to Matter
function chipSupportedAttribute(cluster, attribute) {
  if(!chipSupportedCluster(cluster)) return false;

  switch (cluster + "_" + attribute) {
    case "DoorLock_SecurityLevel": return false
    case "ColorControl_Options": return false
    case "Basic_ZCLVersion": return false
    case "Basic_StackVersion": return false
    case "Basic_GenericDevice-Class": return false
    case "Basic_GenericDevice-Type": return false
    case "Basic_ProductCode": return false
    case "Basic_ManufacturerVersionDetails": return false
    case "Basic_LocationDescription": return false
    case "Basic_Basic_PhysicalEnvironment": return false
    case "Basic_AlarmMask": return false
    case "Basic_DisableLocalConfig": return false
    case "Basic_ResetToFactoryDefaults": return false
    case "Basic_PhysicalEnvironment": return false
    case "Basic_DeviceEnabled": return false
    default: return true
  }
}

// These commands can't be translated from Unify to Matter
function chipSupportedCommands(cluster, command) {
  if(!chipSupportedCluster(cluster)) return false;
  if (command.includes("Response")) {
    return false
  }

  switch (cluster + "_" + command) {
    case "Level_MoveToClosestFrequency": return false
    case "DoorLock_Toggle": return false
    case "Identify_IdentifyQuery": return false
    case "Basic_ResetToFactoryDefaults": return false
    default: return true
  }
}

function chipCommandsConversion(command) {
  if (command.includes("Weekday")) {
    command = command.replace("Weekday", "WeekDay")
  }


  switch (command) {
    default: return asUpperCamelCase(command)
  }
}

// Conversion of attribute names format from Unify to Matter for those not helped by functions.
function chipAttributeConversion(attributeName) {
  switch (attributeName) {
    case "MainsVoltageDwellTripPoint": return "MainsVoltageDwellTrip"
    case "BatteryAHrRating": return "BatteryAhrRating"
    case "Battery2AHrRating": return "Battery2AhrRating"
    case "Battery3AHrRating": return "Battery3AhrRating"
    case "RequirePINforRFOperation": return "RequirePINforRemoteOperation"
    case "RFOperationEventMask": return "RFIDOperationEventMask"
    case "RFProgrammingEventMask": return "RFIDProgrammingEventMask"
    case "WindowCoveringType": return "WindowCovering"
    case "ConfigOrStatus": return "ConfigStatus"
    case "ACCapacity": return "AcCapacity"
    case "ZoneID": return "ZoneId"
    case "ColorTemperatureMireds": return "ColorTemperature"
    case "IASCIEAddress": return "IasCieAddress"
    case "ManufacturerName": return "VendorName"
    case "ModelIdentifier": return "ProductName"
    case "HWVersion": return "HardwareVersion"
    case "ModelIdentifier": return "HardwareVersionString"
    case "ApplicationVersion": return "SoftwareVersion"
    case "SWBuildID": return "SoftwareVersionString"
    case "DateCode": return "ManufacturingDate"
    case "ProductURL": return "ProductURL"
    case "ProductLabel": return "ProductLabel"
    case "SerialNumber": return "SerialNumber"
    case "ApplicationVersion": return "UniqueID"
    default: return asUpperCamelCase(attributeName)
  }
}

function chipSupportedClusterWithCommands(cluster) {
  if (!chipSupportedCluster(cluster)) {
    return false
  }
  
  switch (cluster) {
    case "TemperatureMeasurement": return false
    case "OccupancySensing": return false
    case "FlowMeasurement": return false
    case "PressureMeasurement": return false
    case "TemperatureMeasurement": return false
    case "IlluminanceMeasurement": return false
    case "DehumidificationControl": return false
    case "ThermostatUserInterfaceConfiguration": return false
    case "FanControl": return false
    case "PumpConfigurationAndControl": return false
    case "PowerConfiguration": return false
    case "DeviceTemperatureConfiguration": return false
    case "Time": return false
    case "ShadeConfiguration": return false
    case "PowerConfiguration": return false
    case "Time": return false
    case "ShadeConfiguration": return false
    default: return true
  }
}

// Cluster conversions of name format from Unify to Matter
function chipClusterConversion(clusterName) {
  switch (clusterName) {
    case "Level": return "LevelControl"
    case "RelativityHumidity": return "RelativityHumidityMeasurement"
    case "IASWD": return "IasWd"
    case "IASZone": return "IasZone"
    case "Basic": return "BridgedDeviceBasic"
    //case "PowerConfiguration": return "PowerSourceConfiguration"
    default: return asUpperCamelCase(clusterName)
  }
}

function toLowerCase(string) {
  return string.toLowerCase()
}

exports.asUpperCamelCase = asUpperCamelCase
exports.chipClusterConversion = chipClusterConversion
exports.chipSupportedCluster = chipSupportedCluster
exports.toLowerCase = toLowerCase
exports.chipAttributeConversion = chipAttributeConversion
exports.chipCommandsConversion = chipCommandsConversion
exports.chipSupportedAttribute = chipSupportedAttribute
exports.chipSupportedCommands = chipSupportedCommands
exports.chipSupportedClusterWithCommands = chipSupportedClusterWithCommands
