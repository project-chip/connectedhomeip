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
const zapPath      = '../../../../../third_party/zap/repo/dist/src-electron/';
const templateUtil = require(zapPath + 'generator/template-util.js')
const zclHelper    = require(zapPath + 'generator/helper-zcl.js')
const zclQuery     = require(zapPath + 'db/query-zcl.js')
const cHelper      = require(zapPath + 'generator/helper-c.js')
const string       = require(zapPath + 'util/string.js')

const StringHelper    = require('../../common/StringHelper.js');
const ChipTypesHelper = require('../../common/ChipTypesHelper.js');

// This list of attributes is taken from section '11.2. Global Attributes' of the
// Data Model specification.
const kGlobalAttributes = [
  0xfffc, // ClusterRevision
  0xfffd, // FeatureMap
];

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

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => {
    console.log(err);
    throw err;
  });
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
      const basicType = ChipTypesHelper.asBasicType(zclType);
      switch (basicType) {
      case 'bool':
        return 'Int8u';
      case 'int8_t':
        return 'Int8s';
      case 'uint8_t':
        return 'Int8u';
      case 'int16_t':
        return 'Int16s';
      case 'uint16_t':
        return 'Int16u';
      case 'int24_t':
        return 'Int24s';
      case 'uint24_t':
        return 'Int24u';
      case 'int32_t':
        return 'Int32s';
      case 'uint32_t':
        return 'Int32u';
      case 'int64_t':
        return 'Int64s';
      case 'uint64_t':
        return 'Int64u';
      default:
        error = 'Unhandled underlying type ' + zclType + ' for original type ' + type;
        throw error;
      }
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => {
    console.log(err);
    throw err;
  });
  return templateUtil.templatePromise(this.global, promise)
}

//  Endpoint-config specific helpers
// these helpers are a Hot fix for the "GENERATED_FUNCTIONS" problem
// They should be removed or replace once issue #4369 is resolved
// These helpers only works within the endpoint_config iterator

// List of all cluster with generated functions
const generatedFunctions = {
  Init : [
    'Basic',
    'Identify',
    'Groups',
    'Scenes',
    'OccupancySensing',
    'OnOff',
    'LevelControl',
    'ColorControl',
    'IasZone',
    'PumpConfigurationAndControl',
    'EthernetNetworkDiagnostics',
    'SoftwareDiagnostics',
    'ThreadNetworkDiagnostics',
    'GeneralDiagnostics',
    'WiFiNetworkDiagnostics',
  ],
  AttributeChanged : [ 'Identify', 'DoorLock', 'PumpConfigurationAndControl' ],
  PreAttributeChanged : [ 'IasZone' ],
  MessageSent : [ 'IasZone' ],
};

const generatedFunctionTypes = [ 'Init', 'AttributeChanged', 'MessageSent', 'PreAttributeChanged' ];

function getGeneratedFunctionTypes(context)
{
  if (context.clusterSide != 'server') {
    return [];
  }

  return generatedFunctionTypes.filter(type => generatedFunctions[type].includes(asUpperCamelCase(context.clusterName)));
}

const alreadyGeneratedFunctions = [];

function chip_cluster_functions()
{
  const clusterName = asUpperCamelCase(this.clusterName);
  // Only one array of Generated functions per cluster across all endpoints
  if (alreadyGeneratedFunctions.includes(clusterName)) {
    return null;
  }
  alreadyGeneratedFunctions.push(clusterName);

  const types = getGeneratedFunctionTypes(this);
  if (types.length == 0) {
    return null;
  }

  const functions = types.map(type => `(EmberAfGenericClusterFunction) emberAf${clusterName}ClusterServer${type}Callback,`);
  return `\\\nconst EmberAfGenericClusterFunction chipFuncArray${clusterName}Server[] = { \\\n${functions.join(' \\\n')}}; \\\n`;
}

function chip_cluster_functions_name(defaultValue)
{
  const types = getGeneratedFunctionTypes(this);
  return types.length == 0 ? defaultValue : `chipFuncArray${asUpperCamelCase(this.clusterName)}Server`;
}

function chip_cluster_functions_mask()
{
  const types = getGeneratedFunctionTypes(this);
  const masks = [ this.clusterSide ].concat(types.map(type => type + 'Function'));
  return masks.map(mask => `CLUSTER_MASK_${cHelper.asDelimitedMacro(mask)}`).join(' | ');
}

//  End of Endpoint-config specific helpers

function asPrintFormat(type)
{
  if (StringHelper.isString(type)) {
    return '%s';
  }

  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      const basicType = ChipTypesHelper.asBasicType(zclType);
      switch (basicType) {
      case 'bool':
        return '%d';
      case 'int8_t':
        return '%" PRId8 "';
      case 'uint8_t':
        return '%" PRIu8 "';
      case 'int16_t':
        return '%" PRId16 "';
      case 'uint16_t':
        return '%" PRIu16 "';
      case 'int24_t':
        return '%" PRId32 "';
      case 'uint24_t':
        return '%" PRIu32 "';
      case 'int32_t':
        return '%" PRId32 "';
      case 'uint32_t':
        return '%" PRIu32 "';
      case 'int64_t':
        return '%" PRId64 "';
      case 'uint64_t':
        return '%" PRIu64 "';
      default:
        return '%p';
      }
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => {
    console.log(err);
    throw err;
  });
  return templateUtil.templatePromise(this.global, promise)
}

function asTypeLiteralSuffix(type)
{
  switch (type) {
  case 'int32_t':
    return 'L';
  case 'int64_t':
    return 'LL';
  case 'uint16_t':
    return 'U';
  case 'uint32_t':
    return 'UL';
  case 'uint64_t':
    return 'ULL';
  default:
    return '';
  }
}

function hasSpecificAttributes(options)
{
  return this.count > kGlobalAttributes.length;
}

function asLowerCamelCase(label)
{
  let str = string.toCamelCase(label, true);
  return str.replace(/[\.:]/g, '');
}

function asUpperCamelCase(label)
{
  let str = string.toCamelCase(label, false);
  return str.replace(/[\.:]/g, '');
}

function asMEI(prefix, suffix)
{
  return cHelper.asHex((prefix << 16) + suffix, 8);
}

function asChipZapType(type)
{
  if (StringHelper.isOctetString(type)) {
    return 'chip::ByteSpan';
  }

  if (StringHelper.isCharString(type)) {
    return 'Span<const char>';
  }

  switch (type) {
  case 'BOOLEAN':
    return 'bool';
  case 'INT8S':
    return 'int8_t';
  case 'INT16S':
    return 'int16_t';
  case 'INT24S':
    return 'int24_t';
  case 'INT32S':
    return 'int32_t';
  case 'INT64S':
    return 'int64_t';
  case 'INT8U':
    return 'uint8_t';
  case 'INT16U':
    return 'uint16_t';
  case 'INT24U':
    return 'uint24_t';
  case 'INT32U':
    return 'uint32_t';
  case 'INT64U':
    return 'uint64_t';
  }

  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      const basicType = ChipTypesHelper.asBasicType(zclType);
      switch (basicType) {
      case 'bool':
      case 'int8_t':
      case 'uint8_t':
      case 'int16_t':
      case 'uint16_t':
      case 'int24_t':
      case 'uint24_t':
      case 'int32_t':
      case 'uint32_t':
      case 'int64_t':
      case 'uint64_t':
        return basicType;
      default:
        return type + '::Type'
      }
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => {
    console.log(err);
    throw err;
  });
  return templateUtil.templatePromise(this.global, promise)
}

//
// Module exports
//
exports.asPrintFormat               = asPrintFormat;
exports.asReadType                  = asReadType;
exports.asReadTypeLength            = asReadTypeLength;
exports.chip_cluster_functions      = chip_cluster_functions;
exports.chip_cluster_functions_name = chip_cluster_functions_name;
exports.chip_cluster_functions_mask = chip_cluster_functions_mask;
exports.asTypeLiteralSuffix         = asTypeLiteralSuffix;
exports.asLowerCamelCase            = asLowerCamelCase;
exports.asUpperCamelCase            = asUpperCamelCase;
exports.hasSpecificAttributes       = hasSpecificAttributes;
exports.asMEI                       = asMEI;
exports.asChipZapType               = asChipZapType;
