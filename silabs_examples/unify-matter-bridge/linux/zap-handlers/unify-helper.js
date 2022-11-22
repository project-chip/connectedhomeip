
function asUpperCamelCaseUnify(label, options) {
  const preserveAcronyms = options && options.hash.preserveAcronyms;

  let tokens = label.replace(/[+()&]/g, '').split(/ |_|-|\//);

  let str = tokens
    .map((token) => {
      let isAcronym = (token == token.toUpperCase());
      if (!isAcronym) {
        let newToken = token[0].toUpperCase();
        if (token.length > 1) {
            newToken += token.substring(1);
        }
        return newToken;
      }

      if (preserveAcronyms) {
        return token;
      }

      // if preserveAcronyms is false, then anything beyond the first letter becomes lower-case.
      let newToken = token[0];
      if (token.length > 1) {
          newToken += token.substring(1).toLowerCase();
      }
      return newToken;
    })
    .join('');

  return str.replace(/[^A-Za-z0-9_]/g, '');
}

function supportedCluster(clusterName) {
  clusterName = asUpperCamelCaseUnify(clusterName)
  switch (clusterName) {
    case "NetworkCommissioning": return false
    case "Groups": return false   
    case "TestCluster": return false
    case "Descriptor": return false
    case "Binding": return false
    case "AccessControl": return false
    case "AdministratorCommissioning": return false
    case "BridgedDeviceBasic": return false
    case "Actions": return false
    case "EthernetNetworkDiagnostics": return false
    case "BallastConfiguration": return false
    case "DeviceTemperatureConfiguration": return false
    case "PumpConfigurationAndControl": return false
    case "FaultInjection": return false
    case "UnitTesting": return false
    default: return true
  }
}

function supportedStruct(structName) {
  switch (structName) {
    case "LabelStruct": return false
    default: return true
  }
}

function matterClusterConversion(clusterName) {
  switch (asUpperCamelCaseUnify(clusterName)) {
    case "LevelControl": return "Level"
    default: return asUpperCamelCaseUnify(clusterName)
  }
}

function overrideEnumType(typeName) {
  switch (typeName) {
    // Occupancy
    case "OccupancySensorType": return true
    // ColorControl
    case "ColorMode": return true
    case "EnhancedColorMode": return true
    default: return false
  }
}

function overrideBitmapType(typeName) {
  switch (typeName) {
    case "StartTime": return false
    default: return true
  }
}

/** Because the Matter and UCL "dotdot" xml differ in bitmap element names,
 *  this function maps Matter to UCL names  */
function matterToUclBitmapElem(name) {
  switch (name) {
    // Occupancy
    case "Occupied": return "SensedOccupancy"
    case "Pir": return "PIR"
    // ColorControl
    case "XYAttributesSupported": return "XYSupported"
    case "ColorTemperature": return "ColorTemperatureMireds"
    default: return name
  }
}


exports.matterClusterConversion = matterClusterConversion
exports.supportedCluster = supportedCluster
exports.asUpperCamelCaseUnify = asUpperCamelCaseUnify
exports.supportedStruct = supportedStruct
exports.overrideEnumType = overrideEnumType
exports.overrideBitmapType = overrideBitmapType
exports.matterToUclBitmapElem = matterToUclBitmapElem