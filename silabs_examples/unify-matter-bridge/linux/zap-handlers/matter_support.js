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


// List of Cluster whose attributes/enum need cluster name append at start
const attribute_type_need_cluster_name_append = {
  "On/Off": {
    "Feature": true,
    "EffectIdentifierEnum": true,
  },
  "*": {
    "Feature": true,
    "NameSupportBitmap": true,
  },
};

function matterAppendClusterToAttrType(clusterName, attributeType) {
  const attributeTypeName = String(attributeType);

  if (!attribute_type_need_cluster_name_append.hasOwnProperty(clusterName)) {
    clusterName = '*';
  }

  if (attribute_type_need_cluster_name_append[clusterName].hasOwnProperty(attributeTypeName)) {
    return true;
  }

  return false;
}

const bitmap_need_cluster_name_append = {
  "Feature": {
    "Lighting": true,
    "AdvancedSeek": true,
    "TextTracks": true,
    "AudioTracks": true,
  },
};

function matterAppendClusterToBitmap(bitmapName, bitmapItem) {
  const bitmapItemName = String(bitmapItem);
  
  if (!bitmap_need_cluster_name_append.hasOwnProperty(bitmapName)) {
    return false;
  }

  if (bitmap_need_cluster_name_append[bitmapName].hasOwnProperty(bitmapItemName)) {
    return true;
  }

  return false;
}

exports.matterSupportedCluster = matterSupportedCluster
exports.matterClusterName = matterClusterName
exports.matterSupportedClusterCommand = matterSupportedClusterCommand
exports.matterClusterCommandName = matterClusterCommandName
exports.matterSupportedClusterAttribute = matterSupportedClusterAttribute
exports.matterClusterAttributeName = matterClusterAttributeName
exports.matterAppendClusterToAttrType = matterAppendClusterToAttrType
exports.matterAppendClusterToBitmap = matterAppendClusterToBitmap