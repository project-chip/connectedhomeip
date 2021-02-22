/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
const zapPath      = '../../../third_party/zap/repo/src-electron/';
const queryImpexp  = require(zapPath + 'db/query-impexp.js')
const templateUtil = require(zapPath + 'generator/template-util.js')
const zclHelper    = require(zapPath + 'generator/helper-zcl.js')
const zclQuery     = require(zapPath + 'db/query-zcl.js')
const cHelper      = require(zapPath + 'generator/helper-c.js')

const StringHelper    = require('../../../src/app/zap-templates/common/StringHelper.js');
const ChipTypesHelper = require('../../../src/app/zap-templates/common/ChipTypesHelper.js');

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

function asDelimitedCommand(name)
{
  return name.replace(/([a-z])([A-Z])/g, '$1-$2').toLowerCase();
}

function asTypeMinValue(type)
{
  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    this.isArray  = false;
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      const basicType = ChipTypesHelper.asBasicType(zclType);
      switch (basicType) {
      case 'int8_t':
      case 'int16_t':
      case 'int32_t':
      case 'int64_t':
        return 'INT' + parseInt(basicType.slice(3)) + '_MIN';
      case 'uint8_t':
      case 'uint16_t':
      case 'uint32_t':
      case 'uint64_t':
        return '0';
      default:
        error = 'Unhandled underlying type ' + zclType + ' for original type ' + type;
        throw error;
      }
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise);
}

function asTypeMaxValue(type)
{
  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      const basicType = ChipTypesHelper.asBasicType(zclType);
      switch (basicType) {
      case 'int8_t':
      case 'int16_t':
      case 'int32_t':
      case 'int64_t':
        return 'INT' + parseInt(basicType.slice(3)) + '_MAX';
      case 'uint8_t':
      case 'uint16_t':
      case 'uint32_t':
      case 'uint64_t':
        return 'UINT' + parseInt(basicType.slice(4)) + '_MAX';
      default:
        return 'err';
        error = 'Unhandled underlying type ' + zclType + ' for original type ' + type;
        throw error;
      }
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise);
}

function isStrEndsWith(str, substr)
{
  return str.endsWith(substr);
}

//
// Module exports
//
exports.hasSpecificResponse     = hasSpecificResponse;
exports.asCallbackAttributeType = asCallbackAttributeType;
exports.asDelimitedCommand      = asDelimitedCommand;
exports.asTypeMinValue          = asTypeMinValue;
exports.asTypeMaxValue          = asTypeMaxValue;
exports.isStrEndsWith           = isStrEndsWith;
