const deviceTypeConformance = require('./device-spec-conformance.js')
const matterHelper = require('../matter_support.js')

function listComma(obj) {
  if (obj.index == obj.count - 1) return ''
  return ','
}

function cleanDeviceTypeName(device_type) {
  return device_type.replace("MA-", '')
}

function supportedDeviceTypes(device_type) {
  switch (device_type) {
    case "MA-doorlock": return true
    case "MA-windowcovering": return true
    case "MA-thermostat": return true
    case "MA-temperaturesensor": return true
    case "MA-pressuresensor": return true
    case "MA-flowsensor": return true
    case "MA-onofflight": return true
    case "MA-dimmablelight": return true
    case "MA-lightsensor": return true
    case "MA-occupancysensor": return true
    case "MA-extendedcolorlight": return true
    case "MA-lightlevelsensor": return true
    case "MA-onofflightswitch": return true
    case "MA-onoffpluginunit": return true
    case "MA-dimmablepluginunit": return true
    default: return false
  }
}

function hardcodedClusterCodeFilter(clusterCode) {
  switch (clusterCode) {
    // Convert Unify basic to BridgeBasic
    case 0: return 57
    default: return clusterCode
  }
}

function nameNotNull(name) {
  return this.name != null
}

function hardcodedCommandCodeFilter(commandCode) {
  return commandCode
}

function hardcodedAttributeCodeFilter(attributeCode) {
  return attributeCode
}

function matterSupportedClusterFiltered(clusterID) {
  newClusterID = hardcodedClusterCodeFilter(clusterID)
  supported_cluster = matterHelper.matterSupportedCluster(newClusterID)
  return supported_cluster
}

function matterClusterNameFiltered(clusterID) {
  newClusterID = hardcodedClusterCodeFilter(clusterID)
  return matterHelper.matterClusterName(newClusterID)
}

function matterSupportedClusterCommandFiltered(clusterID,commandID) {
  newClusterID = hardcodedClusterCodeFilter(clusterID)
  newCommandID = hardcodedCommandCodeFilter(commandID)
  return matterHelper.matterSupportedClusterCommand(newClusterID,newCommandID)
}

function matterClusterCommandNameFiltered(clusterID,commandID) {
  newClusterID = hardcodedClusterCodeFilter(clusterID)
  newCommandID = hardcodedCommandCodeFilter(commandID)
  return matterHelper.matterClusterCommandName(newClusterID,newCommandID)
}

function matterSupportedClusterAttributeFiltered(clusterID,attributeID) {
  newClusterID = hardcodedClusterCodeFilter(clusterID)
  newAttributeID = hardcodedAttributeCodeFilter(attributeID)
  return matterHelper.matterSupportedClusterAttribute(newClusterID,newAttributeID)
}

function matterClusterAttributeNameFiltered(clusterID,attributesID) {
  newClusterID = hardcodedClusterCodeFilter(clusterID)
  newAttributesID = hardcodedAttributeCodeFilter(attributesID)
  return matterHelper.matterClusterAttributeName(newClusterID,newAttributesID)
}

function isMandatoryCluster() {
  if (this.includeClient || this.includeServer) {
    return "true"
  }
  return "false"
}

function deviceTypeClusterSpecConformance(deviceType) {
  let name = cleanDeviceTypeName(deviceType)
  return deviceTypeConformance.model[name]["clusterSpecConformance"] == true
}

function deviceTypeAttributeSpecConformance(deviceType) {
  let name = cleanDeviceTypeName(deviceType)
  return deviceTypeConformance.model[name]["attributeSpecConformance"] == true
}

function deviceTypeCommandSpecConformance(deviceType) {
  let name = cleanDeviceTypeName(deviceType)
  return deviceTypeConformance.model[name]["commandSpecConformance"] == true
}

function stupidLog(label) {
  console.log("STUPID LOG: " + label)
}
  
exports.stupidLog = stupidLog
exports.listComma = listComma
exports.nameNotNull = nameNotNull
exports.supportedDeviceTypes = supportedDeviceTypes
exports.cleanDeviceTypeName = cleanDeviceTypeName
exports.isMandatoryCluster = isMandatoryCluster
exports.deviceTypeClusterSpecConformance = deviceTypeClusterSpecConformance
exports.deviceTypeAttributeSpecConformance = deviceTypeAttributeSpecConformance
exports.deviceTypeCommandSpecConformance = deviceTypeCommandSpecConformance
exports.matterSupportedClusterFiltered = matterSupportedClusterFiltered
exports.matterClusterNameFiltered = matterClusterNameFiltered
exports.matterSupportedClusterCommandFiltered = matterSupportedClusterCommandFiltered
exports.matterClusterCommandNameFiltered = matterClusterCommandNameFiltered
exports.matterSupportedClusterAttributeFiltered = matterSupportedClusterAttributeFiltered
exports.matterClusterAttributeNameFiltered = matterClusterAttributeNameFiltered
