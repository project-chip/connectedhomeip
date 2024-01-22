/*******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

var unify_matter_mapping = require('../../../unify-matter-common/zap-common/unify_matter_mapping.js')
var unify = require('../zap-generated/data-models/unify_support_model.js')
var matter = require('../zap-generated/data-models/matter_support_model.js')

function matterSupportedCluster(clusterID) {
  return matter.model.hasOwnProperty(clusterID)
}

function unifyEnumValName(n, cluster_name) {
    var name = unify_matter_mapping.unify_enum_name(n.parent.label);
    if(!unify.model.enums.hasOwnProperty(name)) {
      // As few matter have same Enum types for multiple clutsers. Here, we differ them using Cluster name
      var enum_name = cluster_name+"::"+n.parent.label;
      name = unify_matter_mapping.unify_enum_name(enum_name);
      if(!unify.model.enums.hasOwnProperty(name)){
        console.log("--->  Enum " + name + " was not mapped, make sure that this is ok " );
        return n.label
      }
    }
    return unify.model.enums[name][n.index]
}

function unifyEnumType(n,cluster_name){
  var name = unify_matter_mapping.unify_enum_name(n);
  if(!unify.model.enums.hasOwnProperty(name)) {
    // As few matter have same Enum types for multiple clutsers. Here, we differ them using Cluster name
    var enum_name = cluster_name+"::"+n;
    name = unify_matter_mapping.unify_enum_name(enum_name);
    if(!unify.model.enums.hasOwnProperty(name)){
      return n
    }
  }
  return name
}

function matterUnifyEnumMapped(n, cluster_name){
  var name = unify_matter_mapping.unify_enum_name(n);
  if(!unify.model.enums.hasOwnProperty(name)) {
    // As few matter have same Enum types for multiple clutsers. Here, we differ them using Cluster name
    var enum_name = cluster_name+"::"+n;
    name = unify_matter_mapping.unify_enum_name(enum_name);
    if(!unify.model.enums.hasOwnProperty(name)){
      return false
    }
  }
  return true
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

function unifyTypeExists(matter_name) {
    var matter_type_that_dont_exits_in_unify = ["DelayedAllOffEffectVariantEnum", "DyingLightEffectVariantEnum"];
    if( matter_type_that_dont_exits_in_unify.indexOf(matter_name) < 0) {
      return true;
    } else {
      return false;
    }
}

function unknownEnumValue(label, num) {
  var name = unify_matter_mapping.unify_enum_name(label);
    if(unify.model.enums.hasOwnProperty(name)) {
      return unify.model.enums[name][num+1]
    }
}

function checkUnknownEnumValue(label, num) {
  if (unknownEnumValue(label, num)){
    return true
  } return false
}

exports.matterSupportedCluster = matterSupportedCluster
exports.unifyEnumValName = unifyEnumValName
exports.unifyBitmapValName = unifyBitmapValName
exports.unifyTypeExists = unifyTypeExists
exports.unifyEnumType = unifyEnumType
exports.unknownEnumValue = unknownEnumValue
exports.checkUnknownEnumValue = checkUnknownEnumValue
exports.matterUnifyEnumMapped = matterUnifyEnumMapped