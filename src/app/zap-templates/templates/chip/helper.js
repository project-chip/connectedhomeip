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
 * BufBound.Put method.
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
      atts = atts.filter(att => att.clusterCode == clusterCode && att.side == clusterSide);
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

//
// Module exports
//
exports.chip_server_clusters                  = chip_server_clusters;
exports.chip_server_cluster_commands          = chip_server_cluster_commands;
exports.chip_server_cluster_command_arguments = chip_server_cluster_command_arguments
exports.asBasicType                           = ChipTypesHelper.asBasicType;
exports.isSignedType                          = isSignedType;
exports.isDiscreteType                        = isDiscreteType;
exports.chip_server_cluster_attributes        = chip_server_cluster_attributes;
exports.isWritableAttribute                   = isWritableAttribute;
exports.isReportableAttribute                 = isReportableAttribute;
exports.isManufacturerSpecificCommand         = isManufacturerSpecificCommand;
