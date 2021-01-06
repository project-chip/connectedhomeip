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
const queryImpexp  = require(zapPath + 'db/query-impexp.js')
const templateUtil = require(zapPath + 'generator/template-util.js')
const zclHelper    = require(zapPath + 'generator/helper-zcl.js')
const zclQuery     = require(zapPath + 'db/query-zcl.js')

const StringHelper = require('../../common/StringHelper.js');

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

function asValueIfNotPresent(type, isArray)
{
  if (StringHelper.isString(type) || isArray) {
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

  if (StringHelper.isShortString(type)) {
    return '1u';
  }

  if (StringHelper.isLongString(type)) {
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
  if (StringHelper.isShortString(type)) {
    return 'String';
  }

  if (StringHelper.isLongString(type)) {
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
    return 'chip::EndpointId';
  } else if (zclHelper.isStrEqual(label, "endpointId")) {
    return 'chip::EndpointId';
  } else if (zclHelper.isStrEqual(type, "CLUSTER_ID")) {
    return 'chip::ClusterId';
  } else if (zclHelper.isStrEqual(type, "ATTRIBUTE_ID")) {
    return 'chip::AttributeId';
  } else if (zclHelper.isStrEqual(label, "groupId")) {
    return 'chip::GroupId';
  } else if (zclHelper.isStrEqual(label, "commandId")) {
    return 'chip::CommandId';
  } else {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options);
  }
}

//
// Module exports
//
exports.asReadType                                    = asReadType;
exports.asReadTypeLength                              = asReadTypeLength;
exports.asValueIfNotPresent                           = asValueIfNotPresent;
exports.asChipUnderlyingType                          = asChipUnderlyingType;
exports.user_cluster_has_enabled_manufacturer_command = user_cluster_has_enabled_manufacturer_command;
