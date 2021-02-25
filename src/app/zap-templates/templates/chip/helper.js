/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

// Import helpers from zap core
const zapPath      = '../../../../../third_party/zap/repo/src-electron/';
const queryConfig  = require(zapPath + 'db/query-config.js')
const queryImpexp  = require(zapPath + 'db/query-impexp.js')
const templateUtil = require(zapPath + 'generator/template-util.js')
const zclHelper    = require(zapPath + 'generator/helper-zcl.js')
const zclQuery     = require(zapPath + 'db/query-zcl.js')

const StringHelper    = require('../../common/StringHelper.js');
const ChipTypesHelper = require('../../common/ChipTypesHelper.js');

/**
 * This method converts a ZCL type to the length expected for the
 * BufferWriter.Put method.
 * TODO
 * Not all types are supported at the moment, so if there is any unsupported type
 * that we are trying to convert, it will throw an error.
 */
function asPutLength(zclType)
{
  const type = ChipTypesHelper.asBasicType(zclType);
  switch (type) {
  case 'int8_t':
  case 'int16_t':
  case 'int32_t':
  case 'int64_t':
  case 'uint8_t':
  case 'uint16_t':
  case 'uint32_t':
  case 'uint64_t':
    return type.replace(/[^0-9]/g, '');
  default:
    throw error = 'Unhandled type: ' + zclType;
  }
}

function asPutCastType(zclType)
{
  const type = ChipTypesHelper.asBasicType(zclType);
  switch (type) {
  case 'int8_t':
  case 'int16_t':
  case 'int32_t':
  case 'int64_t':
    return 'u' + type;
  case 'uint8_t':
  case 'uint16_t':
  case 'uint32_t':
  case 'uint64_t':
    return type;
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

function chip_clusters(options)
{
  const db = this.global.db;

  return queryImpexp.exportendPointTypeIds(db, this.global.sessionId)
      .then(endpointTypes => { return zclQuery.exportAllClustersDetailsFromEndpointTypes(db, endpointTypes) })
      .then(clusters => clusters.filter(cluster => cluster.enabled == 1))
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
        if (StringHelper.isString(item.type)) {
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
      atts = atts.filter(att => att.clusterCode == clusterCode && att.side == 'server');
      atts.forEach(att => {
        const sameAttributes = atts.filter(att2 => att.name == att2.name);
        let isWritable       = !!sameAttributes.find(att2 => att2.writable);
        let isReportable     = !!sameAttributes.find(att2 => att2.reportable.included);
        if (isWritable || isReportable) {
          att.chipTypePutLength   = asPutLength(att.chipType);
          att.chipTypePutCastType = asPutCastType(att.chipType);
          att.writable            = isWritable;
          att.reportable.included = isReportable;
        }
      })
      atts = atts.filter((att, index) => atts.findIndex(att2 => att.name == att2.name) == index);
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

function asPythonType(zclType)
{
  const type = ChipTypesHelper.asBasicType(zclType);
  switch (type) {
  case 'int8_t':
  case 'int16_t':
  case 'int32_t':
  case 'int64_t':
  case 'uint8_t':
  case 'uint16_t':
  case 'uint32_t':
  case 'uint64_t':
    return 'int';
  case 'char *':
    return 'str';
  case 'uint8_t *':
    return 'byte';
  }
}

function asPythonCType(zclType)
{
  const type = ChipTypesHelper.asBasicType(zclType);
  switch (type) {
  case 'int8_t':
  case 'int16_t':
  case 'int32_t':
  case 'int64_t':
  case 'uint8_t':
  case 'uint16_t':
  case 'uint32_t':
  case 'uint64_t':
    return 'c_' + type.replace('_t', '');
  case 'char *':
  case 'uint8_t *':
    return 'c_char_p';
  }
}

function hasSpecificResponse(commandName)
{
  // Retrieve the clusterName and the clusterSide. If any if not available, an error will be thrown.
  const clusterName = this.parent.name;
  const clusterSide = this.parent.side;
  if (clusterName == undefined || clusterSide == undefined) {
    const error = 'chip_server_cluster_commands: Could not find relevant parent cluster.';
    console.log(error);
    throw error;
  }

  function filterCommand(cmd)
  {
    return cmd.clusterName == clusterName && cmd.name == (commandName + "Response");
  }

  function fn(pkgId)
  {
    const db = this.global.db;
    return queryImpexp.exportendPointTypeIds(db, this.global.sessionId)
        .then(endpointTypes => zclQuery.exportClustersAndEndpointDetailsFromEndpointTypes(db, endpointTypes))
        .then(endpointsAndClusters => zclQuery.exportCommandDetailsFromAllEndpointTypesAndClusters(db, endpointsAndClusters))
        .then(endpointCommands => endpointCommands.filter(filterCommand).length)
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise);
}

function asCallbackAttributeType(attributeType)
{
  switch (parseInt(attributeType)) {
  case 0x00: // nodata / No data
  case 0x0A: // data24 / 24-bit data
  case 0x0C: // data40 / 40-bit data
  case 0x0D: // data48 / 48-bit data
  case 0x0E: // data56 / 56-bit data
  case 0x1A: // map24 / 24-bit bitmap
  case 0x1C: // map40 / 40-bit bitmap
  case 0x1D: // map48 / 48-bit bitmap
  case 0x1E: // map56 / 56-bit bitmap
  case 0x22: // uint24 / Unsigned 24-bit integer
  case 0x24: // uint40 / Unsigned 40-bit integer
  case 0x25: // uint48 / Unsigned 48-bit integer
  case 0x26: // uint56 / Unsigned 56-bit integer
  case 0x2A: // int24 / Signed 24-bit integer
  case 0x2C: // int40 / Signed 40-bit integer
  case 0x2D: // int48 / Signed 48-bit integer
  case 0x2E: // int56 / Signed 56-bit integer
  case 0x38: // semi / Semi-precision
  case 0x39: // single / Single precision
  case 0x3A: // double / Double precision
  case 0x41: // octstr / Octet string
  case 0x42: // string / Character string
  case 0x43: // octstr16 / Long octet string
  case 0x44: // string16 / Long character string
  case 0x48: // array / Array
  case 0x49: // struct / Structure
  case 0x50: // set / Set
  case 0x51: // bag / Bag
  case 0xE0: // ToD / Time of day
    return 'Unsupported';
  case 0x08: // data8 / 8-bit data
  case 0x18: // map8 / 8-bit bitmap
  case 0x20: // uint8 / Unsigned  8-bit integer
  case 0x30: // enum8 / 8-bit enumeration
    return 'Int8u';
  case 0x09: // data16 / 16-bit data
  case 0x19: // map16 / 16-bit bitmap
  case 0x21: // uint16 / Unsigned 16-bit integer
  case 0x31: // enum16 / 16-bit enumeration
  case 0xE8: // clusterId / Cluster ID
  case 0xE9: // attribId / Attribute ID
  case 0xEA: // bacOID / BACnet OID
  case 0xF1: // key128 / 128-bit security key
  case 0xFF: // unk / Unknown
    return 'Int16u';
  case 0x0B: // data32 / 32-bit data
  case 0x1B: // map32 / 32-bit bitmap
  case 0x23: // uint32 / Unsigned 32-bit integer
  case 0xE1: // date / Date
  case 0xE2: // UTC / UTCTime
    return 'Int32u';
  case 0x0F: // data64 / 64-bit data
  case 0x1F: // map64 / 64-bit bitmap
  case 0x27: // uint64 / Unsigned 64-bit integer
  case 0xF0: // EUI64 / IEEE address
    return 'Int64u';
  case 0x10: // bool / Boolean
    return 'Boolean';
  case 0x28: // int8 / Signed 8-bit integer
    return 'Int8s';
  case 0x29: // int16 / Signed 16-bit integer
    return 'Int16s';
  case 0x2B: // int32 / Signed 32-bit integer
    return 'Int32s';
  case 0x2F: // int64 / Signed 64-bit integer
    return 'Int64s';
  default:
    error = 'Unhandled attribute type ' + attributeType;
    throw error;
  }
}

function asObjectiveCNumberType(label, type)
{
  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      switch (zclType) {
      case 'uint8_t':
        return 'UnsignedChar';
      case 'uint16_t':
        return 'UnsignedShort';
      case 'uint32_t':
        return 'UnsignedLong';
      case 'uint64_t':
        return 'UnsignedLongLong';
      case 'int8_t':
        return 'Char';
      case 'int16_t':
        return 'Short';
      case 'int32_t':
        return 'Long';
      case 'int64_t':
        return 'LongLong';
      default:
        error = label + ': Unhandled underlying type ' + zclType + ' for original type ' + type;
        throw error;
      }
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise)
}

function isStrEndsWith(str, substr)
{
  return str.endsWith(substr);
}

//
// Module exports
//
exports.chip_clusters                         = chip_clusters;
exports.chip_server_clusters                  = chip_server_clusters;
exports.chip_server_cluster_commands          = chip_server_cluster_commands;
exports.chip_server_cluster_command_arguments = chip_server_cluster_command_arguments
exports.asBasicType                           = ChipTypesHelper.asBasicType;
exports.asObjectiveCNumberType                = asObjectiveCNumberType;
exports.isSignedType                          = isSignedType;
exports.isDiscreteType                        = isDiscreteType;
exports.chip_server_cluster_attributes        = chip_server_cluster_attributes;
exports.isWritableAttribute                   = isWritableAttribute;
exports.isReportableAttribute                 = isReportableAttribute;
exports.isManufacturerSpecificCommand         = isManufacturerSpecificCommand;
exports.asPythonType                          = asPythonType;
exports.asPythonCType                         = asPythonCType;
exports.asCallbackAttributeType               = asCallbackAttributeType;
exports.hasSpecificResponse                   = hasSpecificResponse;
exports.isStrEndsWith                         = isStrEndsWith;
