
var unify_matter_mapping = require('../../../unify-matter-common/zap-common/unify_matter_mapping.js')
var unify = require('../zap-generated/data-models/unify_support_model.js')


function unifySupportedCluster(clusterID) {
  return unify.model.hasOwnProperty(clusterID)
}

function unifyClusterName(clusterID) {
  if( unify.model.hasOwnProperty(clusterID) ) {
    return unify.model[clusterID].name
  } else {
    return "Cluster"+clusterID
  }
}

function unifySupportedClusterCommand(clusterID, commandID) {
  if(unify.model.hasOwnProperty(clusterID)) {
    return unify.model[clusterID].commands.hasOwnProperty(commandID);
  }
  return false;
}

function unifyClusterCommandName(clusterID, commandID) {
  if( unifySupportedClusterCommand(clusterID,commandID) ) {
    return unify.model[clusterID].commands[commandID].name
  } else {
    return "Command_"+commandID
  }
}

function unifyClusterCommandArgument(clusterID, commandID, index) {
  if( unifySupportedClusterCommand(clusterID,commandID) ) {
    return unify.model[clusterID].commands[commandID].arguments[index]
  } else {
    return "argument_"+commandID+"-" + index
  }
}

function unifySupportedClusterCommandResponse(clusterID, commandID) {
    if( unify.model.hasOwnProperty(clusterID) ) {
      return unify.model[clusterID].commandresponses.hasOwnProperty(commandID);
    }
    return false;
}

function unifyClusterCommandResponseName(clusterID,commandID) {
  if( unifySupportedClusterCommandResponse(clusterID,commandID) ) {
    return unify.model[clusterID].commandresponses[commandID].name;
  } else {
    return ""
  }
}

function unifySupportedClusterAttribute(clusterID,attributeID) {
  if(unify.model.hasOwnProperty(clusterID)) {
    return unify.model[clusterID].attributes.hasOwnProperty(attributeID);
  }
  return false;
}

function unifyClusterAttributeName(clusterID,attributesID) {
  if( unifySupportedClusterAttribute(clusterID,attributesID) ) {
    return unify.model[clusterID].attributes[attributesID]
  } else {
    return "Attribute_"+attributesID
  }
}

function unifyEnumValName(n) {
  var name = unify_matter_mapping.unify_enum_name(n.parent.label);
  if(!unify.model.enums.hasOwnProperty(name)) {
    // As few matter have same Enum types for multiple clutsers. Here, we differ them using Cluster name
    var enum_name = n.parent.parent.label+"::"+n.parent.label;
    name = unify_matter_mapping.unify_enum_name(enum_name);
    if(!unify.model.enums.hasOwnProperty(name)){
      console.log("--->  Enum " + name + " was not mapped, make sure that this is ok " );
      return n.label
    }
  }
  return unify.model.enums[name][n.index]
}

function unifyBitmapValName(n) {
  var name = unify_matter_mapping.unify_bitmap_name(n.parent.label);
  if(unify.model.bitmaps.hasOwnProperty(name)) {
    return unify.model.bitmaps[name][n.mask]
  } else {
    console.log("---> Bitmap " + name + " was not mapped, make sure that this is ok " );
    return n.label
  }
}

function unifyMatchStringValue(n, value) {
  if(n.label === value) {
    return true;
  } else {
    return false;
  }
}
  

function unifyTypeExists(matter_name) {
  var matter_type_that_dont_exits_in_unify = ["OnOffDelayedAllOffEffectVariant"];
  if( matter_type_that_dont_exits_in_unify.indexOf(matter_name) < 0) {
    return true;
  } else {
    return false;
  }
}

function unifyCanPassThroughEnumValue(cluster_name, attribute_id) {
  // List of Cluster (and array of attributes) whose values enum values
  // can be mapped directly to matter because they have an enum available.
  // These attributes need to have an enum defined under cluster-enums.h, e.g. here
  // ColorMode can be in the list because we have
  // 'enum class ColorMode : uint8_t' in cluster-enums.h
  const pass_through_enum_values = new Map([
    ["Color Control" , [8]], // ColorControl::ColorMode
  ]);

  if ((pass_through_enum_values.get(cluster_name) !== undefined) &&
       pass_through_enum_values.get(cluster_name).includes(attribute_id)) {
    return true;
  }
  return false;
}

function unifyCanUseChipBitMap(cluster_name, attribute_id) {
  // List of Cluster (and array of attributes) whose values bitmap values
  // can be mapped using the chip::BitMask type.
  const can_use_chip_bitmask = new Map([
    ["Color Control" , [0x400A]], // ColorControl::ColorCapabilities
  ]);

  if ((can_use_chip_bitmask.get(cluster_name) !== undefined) &&
  can_use_chip_bitmask.get(cluster_name).includes(attribute_id)) {
    return true;
  }
  return false;
}

// List of Cluster whose attributes/enum need cluster name append at start
const attribute_type_need_cluster_name_append = {
  "On/Off": {
      "EffectIdentifierEnum": true,
  },
};

function unifyAppendClusterToAttrType(cluster_name, attribute_type) {
  if (attribute_type_need_cluster_name_append.hasOwnProperty(cluster_name)) {
    const cluster = attribute_type_need_cluster_name_append[cluster_name];
    const attribute_type_name = String(attribute_type);

    if (cluster.hasOwnProperty(attribute_type_name)) {
      return true;
    }
  }
  return false;
}

const SpecialValues = {
  "Level Control": {
    StartUpCurrentLevel: {
      MinimumDeviceValuePermitted: 0,
      SetToPreviousValue: 0xff
    }
  }
};

function attributeHasSpecialValues(cluster, attribute) {
  return SpecialValues[cluster] && SpecialValues[cluster][attribute];
}

function getSpecialValues(cluster, attribute, modified_value) {
  if (cluster === "Level Control") {
    if (attribute === "StartUpCurrentLevel") {
      return `
        if (strcmp(${modified_value}.dump().c_str(), "\\"MinimumDeviceValuePermitted\\"") == 0) {
          ${modified_value} = 0;
        }
        else if (strcmp(${modified_value}.dump().c_str(), "\\"SetToPreviousValue\\"") == 0) {
          ${modified_value} = 0xFF;
        }
      `;
    }
  }
}

// List of Cluster whose attributes support events
const cluster_attributes_support_events = {
  "Door Lock": {
      "0": true,
      "3": true,
  },
};

function unifySupportedClusterEvents(cluster_name, attribute_id) {
  if (cluster_attributes_support_events.hasOwnProperty(cluster_name)) {
    const cluster = cluster_attributes_support_events[cluster_name];
    const attributeid = String(attribute_id);

    if (cluster.hasOwnProperty(attributeid)) {
      return true;
    }
  }
  return false;
}

function getMatterEventNameForAttribute(cluster_name, attribute_id) {
  return unify_matter_mapping.matter_event_name(cluster_name, attribute_id)
}

function getMatterEventEnum(clusterID, eventID, value) {
  return unify_matter_mapping.matter_event_enum(clusterID, eventID, value)
}

function unifySupportedStruct(label) {
  switch (label) {
    case 'CredentialStruct' : return true;
    default: return false
  }
  }

exports.getSpecialValues = getSpecialValues
exports.attributeHasSpecialValues = attributeHasSpecialValues
exports.unifySupportedCluster = unifySupportedCluster
exports.unifyClusterName = unifyClusterName
exports.unifySupportedClusterCommand = unifySupportedClusterCommand
exports.unifyClusterCommandName = unifyClusterCommandName
exports.unifySupportedClusterCommandResponse = unifySupportedClusterCommandResponse
exports.unifyClusterCommandResponseName = unifyClusterCommandResponseName
exports.unifySupportedClusterAttribute = unifySupportedClusterAttribute
exports.unifyClusterAttributeName = unifyClusterAttributeName
exports.unifyEnumValName = unifyEnumValName
exports.unifyBitmapValName = unifyBitmapValName
exports.unifyMatchStringValue = unifyMatchStringValue
exports.unifyClusterCommandArgument = unifyClusterCommandArgument
exports.unifyTypeExists = unifyTypeExists
exports.unifyCanPassThroughEnumValue = unifyCanPassThroughEnumValue
exports.unifyCanUseChipBitMap = unifyCanUseChipBitMap
exports.unifyAppendClusterToAttrType = unifyAppendClusterToAttrType
exports.unifySupportedClusterEvents = unifySupportedClusterEvents
exports.getMatterEventNameForAttribute = getMatterEventNameForAttribute
exports.getMatterEventEnum = getMatterEventEnum
exports.unifySupportedStruct = unifySupportedStruct
