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
const templateUtil = require(zapPath + 'generator/template-util.js')
const zclHelper    = require(zapPath + 'generator/helper-zcl.js')

const { Clusters }    = require('../../common/ClustersHelper.js');
const StringHelper    = require('../../common/StringHelper.js');
const ChipTypesHelper = require('../../common/ChipTypesHelper.js');

function asBlocks(promise, options)
{
  function fn(pkgId)
  {
    return Clusters.init(this, pkgId).then(() => promise.then(data => templateUtil.collectBlocks(data, options, this)));
  }
  return templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
}

function asPromise(promise)
{
  return templateUtil.templatePromise(this.global, promise);
}

function throwErrorIfUndefined(item, errorMsg, conditions)
{
  conditions.forEach(condition => {
    if (condition == undefined) {
      console.log(item);
      console.log(errorMsg);
      throw error;
    }
  });
}

function checkIsInsideClusterBlock(context, name)
{
  const clusterName = context.name;
  const clusterSide = context.side;
  const errorMsg    = name + ': Not inside a ({#chip_server_clusters}} block.';

  throwErrorIfUndefined(context, errorMsg, [ clusterName, clusterSide ]);

  return { clusterName, clusterSide };
}

function checkIsInsideCommandBlock(context, name)
{
  const commandSource = context.commandSource;
  const commandId     = context.id;
  const errorMsg      = name + ': Not inside a ({#chip_cluster_commands}} block.';

  throwErrorIfUndefined(context, errorMsg, [ commandId, commandSource ]);

  return commandId;
}

function checkIsInsideAttributeBlock(context, name)
{
  const code     = context.code;
  const errorMsg = name + ': Not inside a ({#chip_server_attributes}} block.';

  throwErrorIfUndefined(context, errorMsg, [ code ]);
}

function checkIsChipType(context, name)
{
  const type     = context.chipType;
  const errorMsg = name + ': Could not find chipType';

  throwErrorIfUndefined(context, errorMsg, [ type ]);

  return type;
}

/**
 * Creates block iterator over the enabled server side clusters
 *
 * @param {*} options
 */
function chip_server_clusters(options)
{
  return asBlocks.call(this, Clusters.getServerClusters(), options);
}

/**
 * Check if there is any enabled server clusters
 *
 */
function chip_has_server_clusters(options)
{
  return asPromise.call(this, Clusters.getServerClusters().then(clusters => !!clusters.length));
}

/**
 * Creates block iterator over client side enabled clusters
 *
 * @param {*} options
 */
function chip_client_clusters(options)
{
  return asBlocks.call(this, Clusters.getClientClusters(), options);
}

/**
 * Check if there is any enabled client clusters
 *
 */
function chip_has_client_clusters(options)
{
  return asPromise.call(this, Clusters.getClientClusters().then(clusters => !!clusters.length));
}

/**
 * Creates block iterator over enabled clusters
 *
 * @param {*} options
 */
function chip_clusters(options)
{
  return asBlocks.call(this, Clusters.getClusters(), options);
}

/**
 * Check if there is any enabled clusters
 *
 */
function chip_has_clusters(options)
{
  return asPromise.call(this, Clusters.getClusters().then(clusters => !!clusters.length));
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
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this, 'chip_server_cluster_commands');
  return asBlocks.call(this, Clusters.getClientCommands(clusterName), options);
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
  const commandId                    = checkIsInsideCommandBlock(this, 'isManufacturerSpecificCommand');
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this.parent, 'isManufacturerSpecificCommand');

  const filter = command => command.id == commandId;
  const promise          = Clusters.getClientCommands(clusterName).then(commands => commands.find(filter).arguments);
  return asBlocks.call(this, promise, options);
}

/**
 * Returns if a given cluster has any attributes of type List[T]
 *
 * This function is meant to be used inside a {{#chip_server_clusters}}
 * block. It will throws otherwise.
 *
 * @param {*} options
 */
function chip_has_list_attributes(options)
{
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this, 'chip_has_list_attributes');

  const filter = attribute => attribute.isList;
  return asPromise.call(this, Clusters.getAttributes(clusterName, clusterSide).then(attributes => attributes.find(filter)));
}

/**
 * Returns if a given server cluster has any attributes of type List[T]
 *
 * This function is meant to be used inside a {{#chip_server_clusters}}
 * block. It will throws otherwise.
 *
 * @param {*} options
 */
function chip_server_has_list_attributes(options)
{
  const { clusterName } = checkIsInsideClusterBlock(this, 'chip_server_has_list_attributes');

  const filter = attribute => attribute.isList;
  return asPromise.call(this, Clusters.getServerAttributes(clusterName).then(attributes => attributes.find(filter)));
}

/**
 * Returns if a given client cluster has any attributes of type List[T]
 *
 * This function is meant to be used inside a {{#chip_client_clusters}}
 * block. It will throws otherwise.
 *
 * @param {*} options
 */
function chip_client_has_list_attributes(options)
{
  const { clusterName } = checkIsInsideClusterBlock(this, 'chip_client_has_list_attributes');

  const filter = attribute => attribute.isList;
  return asPromise.call(this, Clusters.getClientAttributes(clusterName).then(attributes => attributes.find(filter)));
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
  const type = checkIsChipType(this, 'isSignedType');
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
  checkIsChipType(this, 'isSignedType');
  return this.discrete;
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
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this, 'chip_server_cluster_attributes');
  return asBlocks.call(this, Clusters.getServerAttributes(clusterName), options);
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
  checkIsInsideAttributeBlock(this, 'isWritableAttribute');
  return this.isWritable == 1;
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
  checkIsInsideAttributeBlock(this, 'isReportableAttribute');
  return this.includedReportable == 1;
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
  checkIsInsideCommandBlock(this, 'isManufacturerSpecificCommand');
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
  case 'chip::ByteSpan':
    return 'bytes'
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
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this.parent, 'has_specific_response');
  const filter = response => response.name == (commandName + 'Response');
  return asPromise.call(this, Clusters.getServerResponses(clusterName).then(responses => responses.find(filter)));
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
  case 0x49: // struct / Structure
  case 0x50: // set / Set
  case 0x51: // bag / Bag
  case 0xE0: // ToD / Time of day
  case 0xEA: // bacOID / BACnet OID
  case 0xF1: // key128 / 128-bit security key
  case 0xFF: // unk / Unknown
    return 'Unsupported';
  case 0x41: // octstr / Octet string
  case 0x42: // string / Character string
  case 0x43: // octstr16 / Long octet string
  case 0x44: // string16 / Long character string
    return 'String';
  case 0x48: // array / Array
    return 'List';
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

function asObjectiveCBasicType(type)
{
  if (StringHelper.isOctetString(type)) {
    return 'NSData *';
  } else if (StringHelper.isCharString(type)) {
    return 'NSString *';
  } else {
    return ChipTypesHelper.asBasicType(this.chipType);
  }
}

function asObjectiveCNumberType(label, type)
{
  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      const basicType = ChipTypesHelper.asBasicType(zclType);
      switch (basicType) {
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

function chip_attribute_list_entryTypes(options)
{
  checkIsInsideAttributeBlock(this, 'chip_attribute_list_entry_types');
  return templateUtil.collectBlocks(this.items, options, this);
}

//
// Module exports
//
exports.chip_clusters                         = chip_clusters;
exports.chip_has_clusters                     = chip_has_clusters;
exports.chip_client_clusters                  = chip_client_clusters;
exports.chip_has_client_clusters              = chip_has_client_clusters;
exports.chip_server_clusters                  = chip_server_clusters;
exports.chip_has_server_clusters              = chip_has_server_clusters;
exports.chip_server_cluster_commands          = chip_server_cluster_commands;
exports.chip_server_cluster_command_arguments = chip_server_cluster_command_arguments
exports.chip_attribute_list_entryTypes        = chip_attribute_list_entryTypes;
exports.asBasicType                           = ChipTypesHelper.asBasicType;
exports.asObjectiveCBasicType                 = asObjectiveCBasicType;
exports.asObjectiveCNumberType                = asObjectiveCNumberType;
exports.isSignedType                          = isSignedType;
exports.isDiscreteType                        = isDiscreteType;
exports.chip_server_cluster_attributes        = chip_server_cluster_attributes;
exports.chip_has_list_attributes              = chip_has_list_attributes;
exports.chip_server_has_list_attributes       = chip_server_has_list_attributes;
exports.chip_client_has_list_attributes       = chip_client_has_list_attributes;
exports.isWritableAttribute                   = isWritableAttribute;
exports.isReportableAttribute                 = isReportableAttribute;
exports.isManufacturerSpecificCommand         = isManufacturerSpecificCommand;
exports.asPythonType                          = asPythonType;
exports.asPythonCType                         = asPythonCType;
exports.asCallbackAttributeType               = asCallbackAttributeType;
exports.hasSpecificResponse                   = hasSpecificResponse;
