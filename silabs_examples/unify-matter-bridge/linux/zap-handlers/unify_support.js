
var unify_matter_mapping = require('./unify_matter_mapping.js')
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

function unifySupportedClusterCommand(clusterID,commandID) {
  if(unify.model.hasOwnProperty(clusterID)) {
    return unify.model[clusterID].commands.hasOwnProperty(commandID);
  }
  return false;
}

function unifyClusterCommandName(clusterID,commandID) {
  if( unifySupportedClusterCommand(clusterID,commandID) ) {
    return unify.model[clusterID].commands[commandID].name
  } else {
    return "Command_"+commandID
  }
}

function unifyClusterCommandArgument(clusterID,commandID,index) {
  if( unifySupportedClusterCommand(clusterID,commandID) ) {
    return unify.model[clusterID].commands[commandID].arguments[index]
  } else {
    return "argument_"+commandID+"-" + index
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
  if(unify.model.enums.hasOwnProperty(name)) {
    return unify.model.enums[name][n.index]
  } else {
    return n.label + " FIXME"
  }
}

function unifyBitmapValName(n) {
  var name = unify_matter_mapping.unify_bitmap_name(n.parent.label);
  if(unify.model.bitmaps.hasOwnProperty(name)) {
    return unify.model.bitmaps[name][n.mask]
  } else {
    return n.label + " FIXME"
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

exports.unifySupportedCluster = unifySupportedCluster
exports.unifyClusterName = unifyClusterName
exports.unifySupportedClusterCommand = unifySupportedClusterCommand
exports.unifyClusterCommandName = unifyClusterCommandName
exports.unifySupportedClusterAttribute = unifySupportedClusterAttribute
exports.unifyClusterAttributeName = unifyClusterAttributeName
exports.unifyEnumValName = unifyEnumValName
exports.unifyBitmapValName = unifyBitmapValName
exports.unifyClusterCommandArgument = unifyClusterCommandArgument
exports.unifyTypeExists = unifyTypeExists