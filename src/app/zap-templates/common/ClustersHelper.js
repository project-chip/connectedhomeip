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
const zapPath           = '../../../../third_party/zap/repo/dist/src-electron/';
const queryConfig       = require(zapPath + 'db/query-config.js')
const queryCommand      = require(zapPath + 'db/query-command.js')
const queryEndpoint     = require(zapPath + 'db/query-endpoint.js')
const queryEndpointType = require(zapPath + 'db/query-endpoint-type.js')
const templateUtil      = require(zapPath + 'generator/template-util.js')
const zclHelper         = require(zapPath + 'generator/helper-zcl.js')
const zclQuery          = require(zapPath + 'db/query-zcl.js')

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
  return zclQuery.selectAllStructsWithItemCount(db, packageId)
      .then(structs => Promise.all(structs.map(struct => loadStructItems.call(this, struct, packageId))));
}

function loadClusters()
{
  const { db, sessionId } = this.global;
  return queryEndpointType.selectEndpointTypeIds(db, sessionId)
      .then(endpointTypes => queryEndpointType.selectAllClustersDetailsFromEndpointTypes(db, endpointTypes))
      .then(clusters => clusters.filter(cluster => cluster.enabled == 1));
}

function loadCommandResponse(command, packageId)
{
  const { db, sessionId } = this.global;
  return queryCommand.selectCommandById(db, command.id, packageId).then(commandDetails => {
    if (commandDetails.responseRef == null) {
      command.response = null;
      return command;
    }

    return queryCommand.selectCommandById(db, commandDetails.responseRef, packageId).then(response => {
      command.response = response;
      return command;
    });
  });
}

function loadCommandArguments(command, packageId)
{
  const { db, sessionId } = this.global;
  return queryCommand.selectCommandArgumentsByCommandId(db, command.id, packageId).then(commandArguments => {
    command.arguments = commandArguments;
    return command;
  });
}

function loadCommands(packageId)
{
  const { db, sessionId } = this.global;
  return queryEndpointType.selectEndpointTypeIds(db, sessionId)
      .then(endpointTypes => queryEndpointType.selectClustersAndEndpointDetailsFromEndpointTypes(db, endpointTypes))
      .then(endpointTypesAndClusters => queryCommand.selectCommandDetailsFromAllEndpointTypesAndClusters(
                db, endpointTypesAndClusters, true))
      .then(commands => Promise.all(commands.map(command => loadCommandResponse.call(this, command, packageId))))
      .then(commands => Promise.all(commands.map(command => loadCommandArguments.call(this, command, packageId))));
}

function loadAttributes(packageId)
{
  // The 'server' side is enforced here, because the list of attributes is used to generate client global
  // commands to retrieve server side attributes.
  const { db, sessionId } = this.global;
  return queryEndpointType.selectEndpointTypeIds(db, sessionId)
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

function loadGlobalAttributes(packageId)
{
  const { db, sessionId } = this.global;
  return zclQuery.selectAllAttributes(db, packageId)
      .then(attributes => attributes.filter(attribute => attribute.clusterRef == null))
      .then(attributes => attributes.map(attribute => attribute.code));
}

//
// Load step 2
//

function asChipCallback(item)
{
  if (StringHelper.isOctetString(item.type)) {
    return { name : 'OctetString', type : 'const chip::ByteSpan' };
  }

  if (StringHelper.isCharString(item.type)) {
    return { name : 'CharString', type : 'const chip::CharSpan' };
  }

  if (item.isList) {
    return { name : 'List', type : null };
  }

  if (item.isEnum) {
    // Unsupported or now, until we figure out what to do for callbacks for
    // strongly typed enums.
    return { name : 'Unsupported', type : null };
  }

  const basicType = ChipTypesHelper.asBasicType(item.chipType);
  switch (basicType) {
  case 'int8_t':
  case 'int16_t':
  case 'int32_t':
  case 'int64_t':
    return { name : 'Int' + basicType.replace(/[^0-9]/g, '') + 's', type : basicType };
  case 'uint8_t':
  case 'uint16_t':
  case 'uint32_t':
  case 'uint64_t':
    return { name : 'Int' + basicType.replace(/[^0-9]/g, '') + 'u', type : basicType };
  case 'bool':
    return { name : 'Boolean', type : 'bool' };
  case 'float':
    return { name : 'Float', type : 'float' };
  case 'double':
    return { name : 'Double', type : 'double' };
  default:
    return { name : 'Unsupported', type : null };
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
  if (StringHelper.isOctetString(item.type)) {
    item.chipType = 'chip::ByteSpan';
  } else {
    item.chipType = 'chip::CharSpan';
  }
  item.size = kLengthSizeInBytes + item.maxLength;
  item.name = item.name || item.label;
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

  item.isList  = true;
  item.isArray = true;
  item.type    = entryType;
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
  item.chipType = '_' + item.type;
  item.isStruct = true;

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
    item.isEnum = true;
    itemType    = enumItem.type;
  }

  const bitmap = getBitmap(bitmaps, itemType);
  if (bitmap) {
    item.isBitmap = true;
    itemType      = bitmap.type;
  }

  const atomic = getAtomic(atomics, itemType);
  if (atomic) {
    item.name         = item.name || item.label;
    item.isStruct     = false;
    item.atomicTypeId = atomic.atomicId;
    item.size         = atomic.size;
    item.chipType     = atomic.chipType;
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
    // Flag things ending in "Response" so we can filter out unused responses,
    // but don't stomp on a true isResponse value if it's set already because
    // some other command had this one as its response.
    command.isResponse                    = command.isResponse || command.name.includes('Response');
    command.isManufacturerSpecificCommand = !!this.mfgCode;

    command.hasSpecificResponse = !!command.response;
    if (command.response) {
      const responseName   = command.response.name;
      command.responseName = responseName;
      // The 'response' property contains the response returned by the `selectCommandById`
      // helper. But this one does not contains all the metadata informations added by
      // `enhancedItem`, so instead of using the one from ZAP, retrieve the enhanced version.
      command.response = commands.find(command => command.name == responseName);
      // We might have failed to find a response if our configuration is weird
      // in some way.
      if (command.response) {
        command.response.isResponse = true;
      }
    } else {
      command.responseName = 'DefaultSuccess';
      command.response     = { arguments : [] };
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

  // At this stage, 'command.arguments' may contains 'struct'. But some controllers does not know (yet) how
  // to handle them. So those needs to be inlined.
  commands.forEach(command => {
    if (command.isResponse) {
      return;
    }

    command.expandedArguments = inlineStructItems(command.arguments);
  });

  return commands;
}

function enhancedAttributes(attributes, globalAttributes, types)
{
  attributes.forEach(attribute => {
    enhancedItem(attribute, types);
    attribute.isGlobalAttribute     = globalAttributes.includes(attribute.code);
    attribute.isWritableAttribute   = attribute.isWritable === 1;
    attribute.isReportableAttribute = attribute.includedReportable === 1;
    attribute.chipCallback          = asChipCallback(attribute);
  });

  attributes.forEach(attribute => {
    const argument = {
      name : attribute.name,
      type : attribute.type,
      size : attribute.size,
      isList : attribute.isList,
      isArray : attribute.isList,
      isNullable : attribute.isNullable,
      chipType : attribute.chipType,
      chipCallback : attribute.chipCallback,
      label : attribute.name,
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
    loadGlobalAttributes.call(context, packageId),
  ];

  return Promise.all(promises).then(([types, clusters, commands, attributes, globalAttributes]) => {
    this._clusters = clusters;
    this._commands = enhancedCommands(commands, types);
    this._attributes = enhancedAttributes(attributes, globalAttributes, types);

    return this.ready.resolve();
  }, err => this.ready.reject(err));
}


//
// Helpers: All
//
function asBlocks(promise, options)
{
  const fn = pkgId => Clusters.init(this, pkgId).then(() => promise.then(data => templateUtil.collectBlocks(data, options, this)));
  return templateUtil.ensureZclPackageId(this).then(fn).catch(err => { console.log(err); throw err; });
}

function asPromise(promise)
{
  const fn = pkgId => Clusters.init(this, pkgId).then(() => promise);
  return templateUtil.ensureZclPackageId(this).then(fn).catch(err => { console.log(err); throw err; });
}

//
// Helpers: Get all clusters/commands/responses/attributes.
//
const kResponseFilter = (isResponse, item) => isResponse == item.isResponse;

Clusters.getClusters = function()
{
    return this.ready.then(() => this._clusters);
}

Clusters.getCommands = function()
{
    return this.ready.then(() => this._commands.filter(kResponseFilter.bind(null, false)));
}

Clusters.getResponses = function()
{
    return this.ready.then(() => this._commands.filter(kResponseFilter.bind(null, true)));
}

Clusters.getAttributes = function()
{
    return this.ready.then(() => this._attributes);
}

//
// Helpers: Get by Cluster Name
//
const kNameFilter = (name, item) => name.toLowerCase() == (item.clusterName || item.name).toLowerCase();

Clusters.getCommandsByClusterName = function(name)
{
    return this.getCommands().then(items => items.filter(kNameFilter.bind(null, name)));
}

Clusters.getResponsesByClusterName = function(name)
{
    return this.getResponses().then(items => items.filter(kNameFilter.bind(null, name)));
}

Clusters.getAttributesByClusterName = function(name)
{
    return this.ready.then(() => {
      const clusterId = this._clusters.find(kNameFilter.bind(null, name)).id;
      const filter = attribute => attribute.clusterId == clusterId;
      return this.getAttributes().then(items => items.filter(filter));
    });
}

//
// Helpers: Get by Cluster Side
//
const kSideFilter = (side, item) => item.source ? ((item.source == side && item.outgoing) || (item.source != side && item.incoming))
                                                : item.side == side;

Clusters.getCommandsByClusterSide = function(side)
{
    return this.getCommands().then(items => items.filter(kSideFilter.bind(null, side)));
}

Clusters.getResponsesByClusterSide = function(side)
{
    return this.getResponses().then(items => items.filter(kSideFilter.bind(null, side)));
}

Clusters.getAttributesByClusterSide = function(side)
{
    return this.getAttributes().then(items => items.filter(kSideFilter.bind(null, side)));
}

//
// Helpers: Client
//
const kClientSideFilter = kSideFilter.bind(null, 'client');

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
const kServerSideFilter = kSideFilter.bind(null, 'server');

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
