/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * This module contains the API for templating. For more detailed instructions, read {@tutorial template-tutorial}
 *
 * @module Templating API: toplevel utility helpers
 */

const cHelper = require('../../../third_party/zap/repo/src-electron/generator/helper-c.js')
const zclHelper = require('../../../third_party/zap/repo/src-electron/generator/helper-zcl.js')

/**
 * Produces the top-of-the-file header for a C file.
 *
 * @returns The header content
 */
function chip_header() {
  return `
  /*
  *
  *    Copyright (c) 2020 Project CHIP Authors
  *
  *    Licensed under the Apache License, Version 2.0 (the "License");
  *    you may not use this file except in compliance with the License.
  *    You may obtain a copy of the License at
  *
  *        http://www.apache.org/licenses/LICENSE-2.0
  *
  *    Unless required by applicable law or agreed to in writing, software
  *    distributed under the License is distributed on an "AS IS" BASIS,
  *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  *    See the License for the specific language governing permissions and
  *    limitations under the License.
  */`;
}

function isClient(side) { return 0 == side.localeCompare("client"); }

function isServer(side) { return 0 == side.localeCompare("server"); }

function isStrEqual(str1, str2) { return 0 == str1.localeCompare(str2); }

function isLastElement(index, count) { return index == count - 1; }

function isEnabled(enable) { return 1 == enable; }

function isCommandAvailable(clusterSide, incoming, outgoing, source, name) {
  if (0 == clusterSide.localeCompare(source)) {
    return false;
  }

  if (isClient(clusterSide) && outgoing) {
    return true;
  } else if (isServer(clusterSide) && incoming) {
    return true;
  }
  return false;
}

/**
 * Returns CHIP specific type for ZCL framework
 * This function is flawed since it relies on the
 * type label for CHIP type conversion. CHIP specific XML should have the
 * correct type directly embedded inside.
 *
 * @param {*} label : The xml label of the type.
 * @param {*} type : The xml type to be converted
 */
function asChipUnderlyingType(label, type) {

  if (zclHelper.isStrEqual(label, "endpoint")) {
    return 'chip::EndpointId'
  } else if (zclHelper.isStrEqual(label, "endpointId")) {
    return 'chip::EndpointId'
  } else if (zclHelper.isStrEqual(type, "CLUSTER_ID")) {
    return 'chip::ClusterId'
  } else if (zclHelper.isStrEqual(type, "ATTRIBUTE_ID")){
    return 'chip::AttributeId'
  } else if (zclHelper.isStrEqual(label, "groupId")) {
    return 'chip::GroupId'
  } else if (zclHelper.isStrEqual(label, "commandId")) {
    return 'chip::CommandId'
  } else {
    // Might want to use asUnderlyingZclType instead. TBD
    return cHelper.asUnderlyingType.call(this, type)
  }
}


// WARNING! WARNING! WARNING! WARNING! WARNING! WARNING!
//
// Note: these exports are public API. Templates that might have been created in the past and are
// available in the wild might depend on these names.
// If you rename the functions, you need to still maintain old exports list.
exports.chip_header = chip_header;
exports.isClient = isClient;
exports.isServer = isServer;
exports.isStrEqual = isStrEqual;
exports.isLastElement = isLastElement;
exports.isEnabled = isEnabled;
exports.isCommandAvailable = isCommandAvailable;
exports.asChipUnderlyingType = asChipUnderlyingType;