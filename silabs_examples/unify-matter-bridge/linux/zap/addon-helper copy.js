
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


let cluster_map = {
  "Identify" : {matter_name:"Identify" },
  "OnOff" : { matter_name:"OnOff"  },
  "Level" : {  
    matter_name: "LevelControl", 
    attribute_name_map: { },
    command_name_map: { }
  },
}

// These clusters are not supported directly translating from Unify -> Matter
function chipSupportedCluster(clusterName) {
  return clusterName in cluster_map
}

// These attributes can't be translated from Unify to Matter
function chipSupportedAttribute(cluster, attribute) {
  if(!chipSupportedCluster(cluster)) return false;
  
  if(attribute in cluster_map[cluster].attribute_name_map ) {
    return cluster_map[cluster].attribute_name_map[attribute] != null
  } else {
    return true
  }
}

// These commands can't be translated from Unify to Matter
function chipSupportedCommands(cluster, command) {
  if (command.includes("Response")) {
    return false
  }
}

function chipCommandsConversion(command) {
  return asUpperCamelCase(command)
}

// Conversion of attribute names format from Unify to Matter for those not helped by functions.
function chipAttributeConversion(cluster,attributeName) {
  if( attributeName in cluster_map[cluster].attribute_name_map) {
    return cluster_map[cluster].attribute_name_map[attributeName]
  } else {
    return asUpperCamelCase(attributeName)
  }
}

function chipSupportedClusterWithCommands(cluster) {
  if (!chipSupportedCluster(cluster)) {
    return false
  } else {
    return true
  }
}

// Cluster conversions of name format from Unify to Matter
function chipClusterConversion(clusterName) {
  return asUpperCamelCase (cluster_map[clusterName].matter_name)
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
