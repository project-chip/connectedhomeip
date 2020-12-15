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

// Import helpers from zap core
const zapPath      = '../../../third_party/zap/repo/src-electron/';
const cHelper      = require(zapPath + 'generator/helper-c.js')
const zclHelper    = require(zapPath + 'generator/helper-zcl.js')
const zclQuery     = require(zapPath + 'db/query-zcl.js')
const queryConfig  = require(zapPath + 'db/query-config.js')
const queryImpexp  = require(zapPath + 'db/query-impexp.js')
const templateUtil = require(zapPath + 'generator/template-util.js')

/**
 * Produces the top-of-the-file header for a C file.
 *
 * @returns The header content
 */
function chip_header()
{
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

const stringShortTypes = [ 'CHAR_STRING', 'OCTET_STRING' ];
const stringLongTypes  = [ 'LONG_CHAR_STRING', 'LONG_OCTET_STRING' ];

function isShortString(type)
{
  return stringShortTypes.includes(type);
}

function isLongString(type)
{
  return stringLongTypes.includes(type);
}

function isString(type)
{
  return isShortString(type) || isLongString(type);
}

function asValueIfNotPresent(type, isArray)
{
  if (isString(type) || isArray) {
    return 'NULL';
  }

  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      switch (zclType) {
      case 'uint8_t':
        return 'UINT8_MAX';
      case 'uint16_t':
        return 'UINT16_MAX';
      case 'uint32_t':
        return 'UINT32_MAX';
      default:
        error = 'Unhandled underlying type ' + zclType + ' for original type ' + type;
        throw error;
      }
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise)
}

// TODO Expose the readTypeLength as an additional member field of {{asUnderlyingZclType}} instead
//      of having to call this method separately.
function asReadTypeLength(type)
{
  const db = this.global.db;

  if (isShortString(type)) {
    return '1u';
  }

  if (isLongString(type)) {
    return '2u';
  }

  function fn(pkgId)
  {
    const defaultResolver = zclQuery.selectAtomicType(db, pkgId, type);

    const enumResolver = zclHelper.isEnum(db, type, pkgId).then(result => {
      return result == 'unknown' ? null : zclQuery.selectEnumByName(db, type, pkgId).then(rec => {
        return zclQuery.selectAtomicType(db, pkgId, rec.type);
      });
    });

    const bitmapResolver = zclHelper.isBitmap(db, type, pkgId).then(result => {
      return result == 'unknown' ? null : zclQuery.selectBitmapByName(db, pkgId, type).then(rec => {
        return zclQuery.selectAtomicType(db, pkgId, rec.type);
      });
    });

    const typeResolver = Promise.all([ defaultResolver, enumResolver, bitmapResolver ]);
    return typeResolver.then(types => (types.find(type => type)).size);
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise)
}

// TODO Expose the readType as an additional member field of {{asUnderlyingZclType}} instead
//      of having to call this method separately.
function asReadType(type)
{
  if (isShortString(type)) {
    return 'String';
  }

  if (isLongString(type)) {
    return 'LongString';
  }

  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      switch (zclType) {
      case 'int8_t':
      case 'uint8_t':
        return 'Int8u';
      case 'int16_t':
      case 'uint16_t':
        return 'Int16u';
      case 'int24_t':
      case 'uint24_t':
        return 'Int24u';
      case 'int32_t':
      case 'uint32_t':
        return 'Int32u';
      case 'int64_t':
      case 'uint64_t':
        return 'Int64u';
      default:
        error = 'Unhandled underlying type ' + zclType + ' for original type ' + type;
        throw error;
      }
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise)
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
function asChipUnderlyingType(label, type)
{

  if (zclHelper.isStrEqual(label, "endpoint")) {
    return 'chip::EndpointId'
  } else if (zclHelper.isStrEqual(label, "endpointId")) {
    return 'chip::EndpointId'
  } else if (zclHelper.isStrEqual(type, "CLUSTER_ID")) {
    return 'chip::ClusterId'
  } else if (zclHelper.isStrEqual(type, "ATTRIBUTE_ID")) {
    return 'chip::AttributeId'
  } else if (zclHelper.isStrEqual(label, "groupId")) {
    return 'chip::GroupId'
  } else if (zclHelper.isStrEqual(label, "commandId")) {
    return 'chip::CommandId'
  } else {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options)
  }
}

function pad(label, len, ch = ' ')
{
  return label.padEnd(len, ch)
}

function concat()
{
  let str = ''
  for (let arg in arguments)
  {
    if (typeof arguments[arg] != 'object') {
      str += arguments[arg]
    }
  }
  return str
}

/**
 * This method converts a ZCL type to the length expected for the
 * BufBound.Put method.
 * TODO
 * Not all types are supported at the moment, so if there is any unssupported type
 * that we are trying to convert, it will throw an error.
 */
function asPutLength(zclType)
{
  switch (zclType) {
  case 'int8_t':
  case 'uint8_t':
  case 'int16_t':
  case 'uint16_t':
  case 'int32_t':
  case 'uint32_t':
  case 'int64_t':
  case 'uint64_t':
    return zclType.replace(/[^0-9]/g, '');
  case 'chip::ClusterId':
    return '16';
  default:
    throw error = 'Unhandled type: ' + zclType;
  }
}

function asPutCastType(zclType)
{
  switch (zclType) {
  case 'int8_t':
  case 'int16_t':
  case 'int32_t':
  case 'int64_t':
    return 'u' + zclType;
  case 'uint8_t':
  case 'uint16_t':
  case 'uint32_t':
  case 'uint64_t':
    return zclType;
  case 'chip::ClusterId':
    return 'uin16_t';
  default:
    throw error = 'Unhandled type: ' + zclType;
  }
}

/**
 * Creates block iterator over the enabled server side clusters
 *
 * @param {*} options
 */
function chip_server_clusters(options)
{
  const db = this.global.db;

  return queryImpexp.exportendPointTypeIds(db, this.global.sessionId)
      .then(endpointTypes => { return zclQuery.exportAllClustersDetailsFromEndpointTypes(db, endpointTypes) })
      .then(clusters => clusters.filter(cluster => cluster.enabled == 1 && cluster.side == 'server'))
      .then(clusters => templateUtil.collectBlocks(clusters, options, this))
}

/**
 * Creates block iterator over the server side cluster command
 * for a given cluster.
 *
 * This function is meant to be used inside a {{#chip_server_clusters}}
 * block. It will throws otherwise.
 *
 * @param {*} options
 */
function chip_server_cluster_commands(options)
{
  // Retrieve the clusterName and the clusterSide. If any if not available, an error will be thrown.
  const clusterName = this.name;
  const clusterSide = this.side;
  if (clusterName == undefined || clusterSide == undefined) {
    const error = 'chip_server_cluster_commands: Could not find relevant parent cluster.';
    console.log(error);
    throw error;
  }

  function filterCommand(cmd)
  {
    return cmd.clusterName == clusterName && cmd.clusterSide == 'client' && cmd.name.includes('Response') == false;
  }

  const db = this.global.db;
  return queryImpexp.exportendPointTypeIds(db, this.global.sessionId)
      .then(endpointTypes => zclQuery.exportClustersAndEndpointDetailsFromEndpointTypes(db, endpointTypes))
      .then(endpointsAndClusters => zclQuery.exportCommandDetailsFromAllEndpointTypesAndClusters(db, endpointsAndClusters))
      .then(endpointCommands => endpointCommands.filter(filterCommand))
      .then(endpointCommands => templateUtil.collectBlocks(endpointCommands, options, this))
}

/**
 * Creates block iterator over the server side cluster command arguments
 * for a given command.
 *
 * This function is meant to be used inside a {{#chip_server_cluster_commands}}
 * block. It will throws otherwise.
 *
 * @param {*} options
 */
function chip_server_cluster_command_arguments(options)
{
  const db = this.global.db;

  function collectItem(arg, pkgId)
  {
    return zclHelper.isStruct(db, arg.type, pkgId).then(result => {
      if (result == 'unknown') {
        return arg;
      }

      return zclQuery.selectStructByName(db, arg.type, pkgId).then(rec => {
        return zclQuery.selectAllStructItemsById(db, rec.id).then(items => items.map(item => {
          item.name = item.label;
          return item;
        }));
      })
    })
  }

  function collectItems(args, pkgId)
  {
    return Promise.all(args.map(arg => collectItem.call(this, arg, pkgId))).then(items => items.flat()).then(items => {
      return Promise.all(items.map(item => {
        if (isString(item.type)) {
          // Enhanced the command argument with 'chipType' for conveniences.
          item.chipType = 'char *';
          return item;
        }

        return zclHelper.asUnderlyingZclType.call(this, item.type, options).then(zclType => {
          // Enhanced the command argument with 'chipType', 'chipTypePutLength', 'chipTypePutCastType' for conveniences.
          item.chipType            = zclType;
          item.chipTypePutLength   = asPutLength(zclType);
          item.chipTypePutCastType = asPutCastType(zclType);
          return item;
        })
      }));
    });
  }

  function fn(pkgId)
  {
    return zclQuery.selectCommandArgumentsByCommandId(db, this.id, pkgId)
        .then(args => collectItems.call(this, args, pkgId))
        .then(items => templateUtil.collectBlocks(items, options, this));
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise)
}

/**
 * Returns if a given command argument chip type is signed.
 *
 * This function is meant to be used inside a {{#chip_*}} block.
 * It will throws otherwise.
 *
 * @param {*} options
 */
function isSignedType()
{
  const type = this.chipType;
  if (!type) {
    const error = 'isSignedType: Could not find chipType';
    console.log(error);
    throw error;
  }

  switch (type) {
  case 'int8_t':
  case 'int16_t':
  case 'int32_t':
  case 'int64_t':
    return true;
  default:
    return false;
  }
}

/**
 * Returns if a given command argument chip type is discrete.
 *
 * This function is meant to be used inside a {{#chip_*}} block.
 * It will throws otherwise.
 *
 * @param {*} options
 */
function isDiscreteType()
{
  const type = this.chipType;
  if (!type) {
    const error = 'isDiscreteType: Could not find chipType';
    console.log(error);
    throw error;
  }

  return this.discrete;
}

function getAttributes(pkgId, options)
{
  const db = this.global.db;
  return queryConfig.getAllSessionAttributes(db, this.global.sessionId).then(atts => {
    return Promise.all(atts.map(att => zclQuery.selectAtomicByName(db, att.type, pkgId).then(atomic => {
      // Enhanced the attribute with 'atomidId', 'discrete', chipType properties for convenience.
      att.atomicTypeId = atomic.atomicId;
      att.discrete     = atomic.discrete;
      return zclHelper.asUnderlyingZclType.call(this, att.type, options).then(zclType => {
        att.chipType = zclType;
        return att;
      });
    })));
  })
}

/**
 * Creates block iterator over the server side cluster attributes
 * for a given cluster.
 *
 * This function is meant to be used inside a {{#chip_server_clusters}}
 * block. It will throws otherwise.
 *
 * @param {*} options
 */
function chip_server_cluster_attributes(options)
{
  // Retrieve the clusterCode and the clusterSide. If any if not available, an error will be thrown.
  const clusterCode = this.code;
  const clusterSide = this.side;
  if (clusterCode == undefined || clusterSide == undefined) {
    const error = 'chip_server_cluster_attributes: Could not find relevant parent cluster.';
    console.log(error);
    throw error;
  }

  function fn(pkgId)
  {
    return getAttributes.call(this, pkgId, options).then(atts => {
      atts = atts.filter(att => att.clusterCode == clusterCode && att.side == clusterSide);
      atts.forEach(att => {
        if (att.writable || att.reportable.included) {
          att.chipTypePutLength   = asPutLength(att.chipType);
          att.chipTypePutCastType = asPutCastType(att.chipType);
        }
      })
      return templateUtil.collectBlocks(atts, options, this);
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise);
}

/**
 * Returns if a given attribute is writable.
 *
 * This function is meant to be used inside a {{#chip_server_cluster_attributes}} block.
 * It will throws otherwise.
 *
 * @param {*} options
 */
function isWritableAttribute(options)
{
  if (this.attributeCode == undefined) {
    const error = 'isWritableAttribute: missing attribute code.';
    console.log(error);
    throw error;
  }

  return this.writable == 1;
}

/**
 * Returns if a given attribute is reportable.
 *
 * This function is meant to be used inside a {{#chip_server_cluster_attributes}} block.
 * It will throws otherwise.
 *
 * @param {*} options
 */
function isReportableAttribute(options)
{
  if (this.attributeCode == undefined) {
    const error = 'isReportableAttribute: missing attribute code.';
    console.log(error);
    throw error;
  }

  return this.reportable.included == 1;
}

/**
 * Check if the cluster (name) has any enabled manufacturer commands. This works only inside
 * cluster block helpers.
 *
 * @param {*} name : Cluster name
 * @param {*} side : Cluster side
 * @param {*} options
 * @returns True if cluster has enabled commands otherwise false
 */
function user_cluster_has_enabled_manufacturer_command(name, side, options)
{
  return queryImpexp.exportendPointTypeIds(this.global.db, this.global.sessionId)
      .then((endpointTypes) => zclQuery.exportClustersAndEndpointDetailsFromEndpointTypes(this.global.db, endpointTypes))
      .then((endpointsAndClusters) => zclQuery.exportCommandDetailsFromAllEndpointTypesAndClusters(
                this.global.db, endpointsAndClusters))
      .then((endpointCommands) => {
        return !!endpointCommands.find(cmd => cmd.mfgCode && zclHelper.isStrEqual(name, cmd.clusterName)
                && zclHelper.isCommandAvailable(side, cmd.incoming, cmd.outgoing, cmd.commandSource, cmd.name));
      })
}

/**
 * Returns if a given command is manufacturer specific
 *
 * This function is meant to be used inside a {{#chip_server_cluster_commands}} block.
 * It will throws otherwise.
 *
 * @param {*} options
 */
function isManufacturerSpecificCommand()
{
  if (this.commandSource == undefined) {
    const error = 'isManufacturerSpecificCommand: Not inside a ({#chip_server_cluster_commands}} block.';
    console.log(error);
    throw error;
  }

  return !!this.mfgCode;
}

// WARNING! WARNING! WARNING! WARNING! WARNING! WARNING!
//
// Note: these exports are public API. Templates that might have been created in the past and are
// available in the wild might depend on these names.
// If you rename the functions, you need to still maintain old exports list.
exports.chip_header          = chip_header;
exports.isString             = isString;
exports.pad                  = pad
exports.concat               = concat
exports.asReadType           = asReadType;
exports.asReadTypeLength     = asReadTypeLength;
exports.asValueIfNotPresent  = asValueIfNotPresent;
exports.asChipUnderlyingType = asChipUnderlyingType;

exports.chip_server_clusters                          = chip_server_clusters;
exports.chip_server_cluster_commands                  = chip_server_cluster_commands;
exports.chip_server_cluster_command_arguments         = chip_server_cluster_command_arguments
exports.isSignedType                                  = isSignedType;
exports.isDiscreteType                                = isDiscreteType;
exports.chip_server_cluster_attributes                = chip_server_cluster_attributes;
exports.isWritableAttribute                           = isWritableAttribute;
exports.isReportableAttribute                         = isReportableAttribute;
exports.user_cluster_has_enabled_manufacturer_command = user_cluster_has_enabled_manufacturer_command;
exports.isManufacturerSpecificCommand                 = isManufacturerSpecificCommand;
