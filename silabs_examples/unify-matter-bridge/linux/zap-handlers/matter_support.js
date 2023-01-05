var matter = require('../zap-generated/data-models/matter_support_model.js')


function matterSupportedCluster(clusterID) {
  return matter.model.hasOwnProperty(clusterID)
}

function matterClusterName(clusterID) {
  if( matter.model.hasOwnProperty(clusterID) ) {
    return matter.model[clusterID].name
  } else {
    return "Cluster"+clusterID
  }
}

function matterSupportedClusterCommand(clusterID,commandID) {
  if(matter.model.hasOwnProperty(clusterID)) {
    return matter.model[clusterID].commands.hasOwnProperty(commandID);
  }
  return false;
}

function matterClusterCommandName(clusterID,commandID) {
  if( matterSupportedClusterCommand(clusterID,commandID) ) {
    return matter.model[clusterID].commands[commandID].name
  } else {
    return "Command_"+commandID
  }
}

function matterSupportedClusterAttribute(clusterID,attributeID) {
  if(matter.model.hasOwnProperty(clusterID)) {
    return matter.model[clusterID].attributes.hasOwnProperty(attributeID);
  }
  return false;
}

function matterClusterAttributeName(clusterID,attributesID) {
  if( matterSupportedClusterAttribute(clusterID,attributesID) ) {
    return matter.model[clusterID].attributes[attributesID]
  } else {
    return "Attribute_"+attributesID
  }
}

exports.matterSupportedCluster = matterSupportedCluster
exports.matterClusterName = matterClusterName
exports.matterSupportedClusterCommand = matterSupportedClusterCommand
exports.matterClusterCommandName = matterClusterCommandName
exports.matterSupportedClusterAttribute = matterSupportedClusterAttribute
exports.matterClusterAttributeName = matterClusterAttributeName