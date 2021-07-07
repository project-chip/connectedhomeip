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
const zapPath       = '../../../../third_party/zap/repo/src-electron/';
const queryConfig   = require(zapPath + 'db/query-config.js')
const queryEndpoint = require(zapPath + 'db/query-endpoint.js')
const templateUtil  = require(zapPath + 'generator/template-util.js')
const zclHelper     = require(zapPath + 'generator/helper-zcl.js')
const zclQuery      = require(zapPath + 'db/query-zcl.js')

const { Deferred }    = require('./Deferred.js');
const ListHelper      = require('./ListHelper.js');
const StringHelper    = require('./StringHelper.js');
const ChipTypesHelper = require('./ChipTypesHelper.js');

//
// Load Step 1
//
function loadAtomics(packageId)
{
  const { db, sessionId } = this.global;
  const options           = { 'hash' : {} };

  const resolveZclTypes = atomics => Promise.all(atomics.map(atomic => {
    return zclHelper.asUnderlyingZclType.call(this, atomic.name, options).then(zclType => {
      atomic.chipType = zclType;
      return atomic;
    });
  }));

  return zclQuery.selectAllAtomics(db, packageId).then(resolveZclTypes);
}

function loadBitmaps(packageId)
{
  const { db, sessionId } = this.global;
  return zclQuery.selectAllBitmaps(db, packageId);
}

function loadEnums(packageId)
{
  const { db, sessionId } = this.global;
  return zclQuery.selectAllEnums(db, packageId);
}

function loadStructItems(struct, packageId)
{
  const { db, sessionId } = this.global;
  return zclQuery.selectAllStructItemsById(db, struct.id).then(structItems => {
    struct.items = structItems;
    return struct;
  });
}

function loadStructs(packageId)
{
  const { db, sessionId } = this.global;
  return zclQuery.selectAllStructs(db, packageId)
      .then(structs => Promise.all(structs.map(struct => loadStructItems.call(this, struct, packageId))));
}

function loadClusters()
{
  const { db, sessionId } = this.global;
  return queryEndpoint.selectEndPointTypeIds(db, sessionId)
      .then(endpointTypes => zclQuery.selectAllClustersDetailsFromEndpointTypes(db, endpointTypes))
      .then(clusters => clusters.filter(cluster => cluster.enabled == 1));
}

function loadCommandArguments(command, packageId)
{
  const { db, sessionId } = this.global;
  return zclQuery.selectCommandArgumentsByCommandId(db, command.id, packageId).then(commandArguments => {
    command.arguments = commandArguments;
    return command;
  });
}

function loadCommands(packageId)
{
  const { db, sessionId } = this.global;
  return queryEndpoint.selectEndPointTypeIds(db, sessionId)
      .then(endpointTypes => zclQuery.exportClustersAndEndpointDetailsFromEndpointTypes(db, endpointTypes))
      .then(endpointTypesAndClusters => zclQuery.exportCommandDetailsFromAllEndpointTypesAndClusters(db, endpointTypesAndClusters))
      .then(commands => Promise.all(commands.map(command => loadCommandArguments.call(this, command, packageId))));
}

function loadAttributes(packageId)
{
  // The 'server' side is enforced here, because the list of attributes is used to generate client global
  // commands to retrieve server side attributes.
  const { db, sessionId } = this.global;
  return queryEndpoint.selectEndPointTypeIds(db, sessionId)
      .then(endpointTypes => Promise.all(
                endpointTypes.map(({ endpointTypeId }) => queryEndpoint.selectEndpointClusters(db, endpointTypeId))))
      .then(clusters => clusters.flat())
      .then(clusters => Promise.all(
                clusters.map(({ clusterId, side, endpointTypeId }) => queryEndpoint.selectEndpointClusterAttributes(
                                 db, clusterId, 'server', endpointTypeId))))
      .then(attributes => attributes.flat())
      .then(attributes => attributes.filter(attribute => attribute.isIncluded))
      .then(attributes => attributes.sort((a, b) => a.code - b.code));
  //.then(attributes => Promise.all(attributes.map(attribute => types.typeSizeAttribute(db, packageId, attribute))
}

//
// Load step 2
//

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
    throw error = 'asPutLength: Unhandled type: ' + zclType;
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
    throw error = 'asPutCastType: Unhandled type: ' + zclType;
  }
}

function asChipCallback(atomic)
{
  switch (parseInt(atomic.atomicId)) {
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
    return { name : 'Unsupported', type : null };
  case 0x41: // octstr / Octet string
  case 0x42: // string / Character string
  case 0x43: // octstr16 / Long octet string
  case 0x44: // string16 / Long character string
    return { name : 'String', type : 'const chip::ByteSpan' };
  case 0x48: // array / Array
    return { name : 'List', type : null };
  case 0x08: // data8 / 8-bit data
  case 0x18: // map8 / 8-bit bitmap
  case 0x20: // uint8 / Unsigned  8-bit integer
  case 0x30: // enum8 / 8-bit enumeration
    return { name : 'Int8u', type : ChipTypesHelper.asBasicType(atomic.type) };
  case 0x09: // data16 / 16-bit data
  case 0x19: // map16 / 16-bit bitmap
  case 0x21: // uint16 / Unsigned 16-bit integer
  case 0x31: // enum16 / 16-bit enumeration
  case 0xE8: // clusterId / Cluster ID
  case 0xE9: // attribId / Attribute ID
  case 0xF8: // endpoint_no / Endpoint Number
    return { name : 'Int16u', type : ChipTypesHelper.asBasicType(atomic.type) };
  case 0x0B: // data32 / 32-bit data
  case 0x1B: // map32 / 32-bit bitmap
  case 0x23: // uint32 / Unsigned 32-bit integer
  case 0xE1: // date / Date
  case 0xE2: // UTC / UTCTime
    return { name : 'Int32u', type : ChipTypesHelper.asBasicType(atomic.type) };
  case 0x0F: // data64 / 64-bit data
  case 0x1F: // map64 / 64-bit bitmap
  case 0x27: // uint64 / Unsigned 64-bit integer
  case 0xF0: // EUI64 / IEEE address
    return { name : 'Int64u', type : ChipTypesHelper.asBasicType(atomic.type) };
  case 0x10: // bool / Boolean
    return { name : 'Boolean', type : 'bool' };
  case 0x28: // int8 / Signed 8-bit integer
    return { name : 'Int8s', type : ChipTypesHelper.asBasicType(atomic.type) };
  case 0x29: // int16 / Signed 16-bit integer
    return { name : 'Int16s', type : ChipTypesHelper.asBasicType(atomic.type) };
  case 0x2B: // int32 / Signed 32-bit integer
    return { name : 'Int32s', type : ChipTypesHelper.asBasicType(atomic.type) };
  case 0x2F: // int64 / Signed 64-bit integer
    return { name : 'Int64s', type : ChipTypesHelper.asBasicType(atomic.type) };
  default:
    error = 'asCallbackAttributeType: Unhandled attribute type ' + attributeType;
    throw error;
  }
}

function getAtomic(atomics, type)
{
  return atomics.find(atomic => atomic.name == type.toLowerCase());
}

function getBitmap(bitmaps, type)
{
  return bitmaps.find(bitmap => bitmap.label == type);
}

function getEnum(enums, type)
{
  return enums.find(enumItem => enumItem.label == type);
}

function getStruct(structs, type)
{
  return structs.find(struct => struct.label == type);
}

function handleString(item, [ atomics, enums, bitmaps, structs ])
{
  if (!StringHelper.isString(item.type)) {
    return false;
  }

  const atomic = getAtomic(atomics, item.type);
  if (!atomic) {
    return false;
  }

  const kLengthSizeInBytes = 2;

  item.atomicTypeId = atomic.atomicId;
  item.chipType     = 'chip::ByteSpan';
  item.size         = kLengthSizeInBytes + item.maxLength;
  item.name         = item.name || item.label;
  return true;
}

function handleList(item, [ atomics, enums, bitmaps, structs ])
{
  if (!ListHelper.isList(item.type)) {
    return false;
  }

  const entryType = item.entryType;
  if (!entryType) {
    console.log(item);
    throw new Error(item.label, 'List[T] is missing type "T" information');
  }

  item.isList = true;
  item.type   = entryType;
  enhancedItem(item, [ atomics, enums, bitmaps, structs ]);

  return true;
}

function handleStruct(item, [ atomics, enums, bitmaps, structs ])
{
  const struct = getStruct(structs, item.type);
  if (!struct) {
    return false;
  }

  // Add a leading `_` before the name of struct to match what is done in the af-structs.zapt template.
  // For instance structs are declared as "typedef struct _{{asType label}}".
  item.chipType     = '_' + item.type;
  item.isStruct     = true;
  item.atomicTypeId = 0x49;

  struct.items.map(structItem => enhancedItem(structItem, [ atomics, enums, bitmaps, structs ]));
  item.items = struct.items;
  item.size  = struct.items.map(type => type.size).reduce((accumulator, currentValue) => accumulator + currentValue, 0);
  return true;
}

function handleBasic(item, [ atomics, enums, bitmaps, structs ])
{
  let itemType = item.type;

  const enumItem = getEnum(enums, itemType);
  if (enumItem) {
    itemType = enumItem.type;
  }

  const bitmap = getBitmap(bitmaps, itemType);
  if (bitmap) {
    itemType = bitmap.type;
  }

  const atomic = getAtomic(atomics, itemType);
  if (atomic) {
    item.name                = item.name || item.label;
    item.isStruct            = false;
    item.atomicTypeId        = atomic.atomicId;
    item.isAnalog            = atomic.isDiscrete == false && atomic.isString == false;
    item.size                = atomic.size;
    item.chipType            = atomic.chipType;
    item.chipTypePutLength   = asPutLength(atomic.chipType);
    item.chipTypePutCastType = asPutCastType(atomic.chipType);
    return true;
  }

  return false;
}

function enhancedItem(item, types)
{
  if (handleString(item, types)) {
    return;
  }

  if (handleList(item, types)) {
    return;
  }

  if (handleStruct(item, types)) {
    return;
  }

  if (handleBasic(item, types)) {
    return;
  }

  console.log(item);
  throw new Error(item.type + ' not found.');
}

function inlineStructItems(args)
{
  const arguments = [];
  args.forEach(argument => {
    if (!argument.isStruct) {
      arguments.push(argument);
      return;
    }

    argument.items.forEach(item => {
      arguments.push(item);
    });
  });

  return arguments;
}

function enhancedCommands(commands, types)
{
  commands.forEach(command => {
    command.arguments.forEach(argument => {
      enhancedItem(argument, types);
    });
  });

  commands.forEach(command => {
    command.isResponse                    = command.name.includes('Response');
    command.isManufacturerSpecificCommand = !!this.mfgCode;
  });

  commands.forEach(command => {
    // This filter uses the assumption that a response to a command has a well defined name, such as
    // (response name) == (command name + 'Response') or s/Request/Response. This is very often the case,
    // but this is not always true since some clusters use the same response to answer different commands, such as the
    // operational cluster.
    const automaticFilter = response => {
      if (!response.isResponse) {
        return false;
      }

      if (response.clusterName != command.clusterName) {
        return false;
      }

      if (response.name == command.name) {
        return false;
      }

      return (response.name == (command.name + 'Response')) || (response.name == (command.name.replace('Request', 'Response')));
    };

    const manualFilter = response => {
      switch (command.name) {
      case 'AddOpCert':
      case 'UpdateOpCert':
      case 'UpdateFabricLabel':
      case 'RemoveFabric':
        return response.name == 'OpCertResponse';
      default:
        return false;
      }
    };
    const filter = response => automaticFilter(response) || manualFilter(response);

    const response = commands.find(filter);
    if (response) {
      command.hasSpecificResponse = true;
      command.responseName        = response.name;
      command.response            = response;
    } else {
      command.hasSpecificResponse = false;
      command.responseName        = 'DefaultSuccess';
      command.response            = { arguments : [] };
    }
  });

  // Filter unused responses
  commands = commands.filter(command => {
    if (!command.isResponse) {
      return true;
    }

    const responseName = command.name;
    return commands.find(command => command.responseName == responseName);
  });

  // At this stage, 'command.arguments' may contains 'struct'. But controllers does not know (yet) how
  // to handle them. So those needs to be inlined.
  commands.forEach(command => {
    if (command.isResponse) {
      return;
    }

    command.arguments = inlineStructItems(command.arguments);
  });
  return commands;
}

function enhancedAttributes(attributes, types)
{
  const kGlobalAttributes = [ 0xfffc, 0xfffd ];

  attributes.forEach(attribute => {
    enhancedItem(attribute, types);
    attribute.isGlobalAttribute     = kGlobalAttributes.includes(attribute.code);
    attribute.isWritableAttribute   = attribute.isWritable == 1;
    attribute.isReportableAttribute = attribute.includedReportable == 1;
    attribute.chipCallback          = asChipCallback({ atomicId : attribute.atomicTypeId, type : attribute.chipType });
  });

  attributes.forEach(attribute => {
    const argument = {
      name : attribute.name,
      type : attribute.type,
      size : attribute.size,
      isList : attribute.isList,
      chipType : attribute.chipType,
      chipCallback : attribute.chipCallback
    };
    attribute.arguments = [ argument ];
    attribute.response  = { arguments : [ argument ] };
  });

  // At this stage, the 'attributes' array contains all attributes enabled for all endpoints. It means
  // that a lot of attributes are duplicated if a cluster is enabled on multiple endpoints but that's
  // not what the templates expect. So let's deduplicate them.
  const compare = (a, b) => (a.name == b.name && a.clusterId == b.clusterId && a.side == b.side);
  return attributes.filter((att, index) => attributes.findIndex(att2 => compare(att, att2)) == index);
}

const Clusters = {
  ready : new Deferred()
};

Clusters.init = function(context, packageId) {
  if (this.ready.running)
  {
    return this.ready;
  }
  this.ready.running = true;

  const loadTypes = [
    loadAtomics.call(context, packageId),
    loadEnums.call(context, packageId),
    loadBitmaps.call(context, packageId),
    loadStructs.call(context, packageId),
  ];

  const promises = [
    Promise.all(loadTypes),
    loadClusters.call(context),
    loadCommands.call(context, packageId),
    loadAttributes.call(context, packageId),
  ];

  return Promise.all(promises).then(([types, clusters, commands, attributes]) => {
    this._clusters = clusters;
    this._commands = enhancedCommands(commands, types);
    this._attributes = enhancedAttributes(attributes, types);

    return this.ready.resolve();
  });
}


//
// Helpers: All
//
function asBlocks(promise, options)
{
  const fn = pkgId => Clusters.init(this, pkgId).then(() => promise.then(data => templateUtil.collectBlocks(data, options, this)));
  return templateUtil.ensureZclPackageId(this).then(fn).catch(err => console.log(err));
}

function asPromise(promise)
{
  const fn = pkgId => Clusters.init(this, pkgId).then(() => promise);
  return templateUtil.ensureZclPackageId(this).then(fn).catch(err => console.log(err));
}

//
// Helpers: Get all
//
Clusters.getClusters = function()
{
    return this.ready.then(() => this._clusters);
}

Clusters.getCommands = function()
{
    const filter = command => command.isResponse == false;
    return this.ready.then(() => this._commands.filter(filter));
}

Clusters.getResponses = function()
{
    const filter = command => command.isResponse == true;
    return this.ready.then(() => this._commands.filter(filter));
}

Clusters.getAttributes = function()
{
    return this.ready.then(() => this._attributes);
}

//
// Helpers: Get by Cluster Name
//
Clusters.getCommandsByClusterName = function(name)
{
    const filter = command => command.clusterName.toLowerCase() == name.toLowerCase();
    return this.getCommands().then(items => items.filter(filter));
}

Clusters.getResponsesByClusterName = function(name)
{
    const filter = command => command.clusterName.toLowerCase() == name.toLowerCase();
    return this.getResponses().then(items => items.filter(filter));
}

Clusters.getAttributesByClusterName = function(name)
{
    return this.ready.then(() => {
      const code = this._clusters.find(cluster => cluster.name.toLowerCase() == name.toLowerCase()).id;
      const filter = attribute => attribute.clusterId == code;
      return this.getAttributes().then(items => items.filter(filter));
    });
}

//
// Helpers: Get by Cluster Side
//
Clusters.getCommandsByClusterSide = function(side)
{
    const filter = item => item.clusterSide == side;
    return this.getCommands().then(items => items.filter(filter));
}

Clusters.getResponsesByClusterSide = function(side)
{
    const filter = item => item.clusterSide == side;
    return this.getResponses().then(items => items.filter(filter));
}

Clusters.getAttributesByClusterSide = function(side)
{
    const filter = item => item.side == side;
    return this.getAttributes().then(items => items.filter(filter));
}

//
// Helpers: Client
//
const kClientSideFilter = item => (item.clusterSide == 'client') || (item.side == 'client');

Clusters.getClientClusters = function()
{
    return this.getClusters().then(items => items.filter(kClientSideFilter));
}

Clusters.getClientCommands = function(name)
{
    return this.getCommandsByClusterName(name).then(items => items.filter(kClientSideFilter));
}

Clusters.getClientResponses = function(name)
{
    return this.getResponsesByClusterName(name).then(items => items.filter(kClientSideFilter));
}

Clusters.getClientAttributes = function(name)
{
    return this.getAttributesByClusterName(name).then(items => items.filter(kClientSideFilter));
}

//
// Helpers: Server
//
const kServerSideFilter = item => (item.clusterSide == 'server') || (item.side == 'server');

Clusters.getServerClusters = function()
{
    return this.getClusters().then(items => items.filter(kServerSideFilter));
}

Clusters.getServerCommands = function(name)
{
    return this.getCommandsByClusterName(name).then(items => items.filter(kServerSideFilter));
}

Clusters.getServerResponses = function(name)
{
    return this.getResponsesByClusterName(name).then(items => items.filter(kServerSideFilter));
}

Clusters.getServerAttributes = function(name)
{
    return this.getAttributesByClusterName(name).then(items => items.filter(kServerSideFilter));
}

//
// Module exports
//
exports.Clusters  = Clusters;
exports.asBlocks  = asBlocks;
exports.asPromise = asPromise;
