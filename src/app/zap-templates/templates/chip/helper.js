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
const zapPath      = '../../../../../third_party/zap/repo/dist/src-electron/';
const templateUtil = require(zapPath + 'generator/template-util.js');
const zclHelper    = require(zapPath + 'generator/helper-zcl.js');
const iteratorUtil = require(zapPath + 'util/iterator-util.js');
const queryAccess  = require(zapPath + 'db/query-access')
const queryZcl     = require(zapPath + 'db/query-zcl');

const { asBlocks, ensureClusters } = require('../../common/ClustersHelper.js');
const StringHelper                 = require('../../common/StringHelper.js');
const ChipTypesHelper              = require('../../common/ChipTypesHelper.js');

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
  const clusterName = context.name ? context.name : context.clusterName;
  const clusterSide = context.side ? context.side : context.clusterSide;
  const errorMsg    = name + ': Not inside a ({#chip_server_clusters}} block.';
  throwErrorIfUndefined(context, errorMsg, [ clusterName, clusterSide ]);

  return { clusterName, clusterSide };
}

function checkIsInsideCommandBlock(context, name)
{
  const clusterName = context.clusterName;
  const clusterSide = context.clusterSide;
  const commandId   = context.id;
  const errorMsg    = name + ': Not inside a ({#chip_cluster_commands}} block.';

  throwErrorIfUndefined(context, errorMsg, [ commandId, clusterName, clusterSide ]);

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

function getCommands(methodName)
{
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this, methodName);
  return clusterSide == 'client' ? ensureClusters(this).getClientCommands(clusterName)
                                 : ensureClusters(this).getServerCommands(clusterName);
}

function getAttributes(methodName)
{
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this, methodName);
  return ensureClusters(this).getAttributesByClusterName(clusterName);
}

function getResponses(methodName)
{
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this, methodName);
  return clusterSide == 'client' ? ensureClusters(this).getClientResponses(clusterName)
                                 : ensureClusters(this).getServerResponses(clusterName);
}

/**
 * Creates block iterator over the enabled server side clusters
 *
 * @param {*} options
 */
function chip_server_clusters(options)
{
  return asBlocks.call(this, ensureClusters(this, options.hash.includeAll).getServerClusters(), options);
}

/**
 * Check if there is any enabled server clusters
 *
 */
function chip_has_server_clusters(options)
{
  return ensureClusters(this).getServerClusters().then(clusters => !!clusters.length);
}

/**
 * Creates block iterator over client side enabled clusters
 *
 * @param {*} options
 */
function chip_client_clusters(options)
{
  return asBlocks.call(this, ensureClusters(this, options.hash.includeAll).getClientClusters(), options);
}

/**
 * Check if there is any enabled client clusters
 *
 */
function chip_has_client_clusters(options)
{
  return ensureClusters(this).getClientClusters().then(clusters => !!clusters.length);
}

/**
 * Creates block iterator over enabled clusters
 *
 * @param {*} options
 */
function chip_clusters(options)
{
  return asBlocks.call(this, ensureClusters(this, options.hash.includeAll).getClusters(), options);
}

/**
 * Check if there is any enabled clusters
 *
 */
function chip_has_clusters(options)
{
  return ensureClusters(this).getClusters().then(clusters => !!clusters.length);
}

/**
 * Creates block iterator over the server global responses
 *
 * @param {*} options
 */
function chip_server_global_responses(options)
{
  return asBlocks.call(this, getServerGlobalAttributeResponses(this), options);
}

async function if_basic_global_response(options)
{
  const attribute          = this.response.arguments[0];
  const globalResponses    = await getServerGlobalAttributeResponses(this);
  const complexType        = attribute.isNullable || attribute.isOptional || attribute.isStruct || attribute.isArray;
  const responseTypeExists = globalResponses.find(item => item.chipType == attribute.chipType);

  if (!complexType && responseTypeExists) {
    return options.fn(this);
  } else {
    return options.inverse(this);
  }
}

function getServerGlobalAttributeResponses(context)
{
  const sorter = (a, b) => a.chipCallback.name.localeCompare(b.chipCallback.name, 'en', { numeric : true });

  const reducer = (unique, item) => {
    const { type, size, isArray, isOptional, isNullable, chipCallback, chipType } = item.response.arguments[0];

    // List-typed elements have a dedicated callback
    if (isArray) {
      return unique;
    }

    if (unique.find(item => item.chipCallback.name == chipCallback.name)) {
      return unique;
    }

    return [...unique, { chipCallback, chipType, size, isOptional, isNullable } ];
  };

  const filter = attributes => attributes.reduce(reducer, []).sort(sorter);
  return ensureClusters(context).getAttributesByClusterSide('server').then(filter);
}

/**
 * Creates block iterator over the cluster commands for a given cluster/side.
 *
 * This function is meant to be used inside a {{#chip_*_clusters}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
function chip_cluster_commands(options)
{
  const commands = getCommands.call(this, 'chip_cluster_commands');

  return asBlocks.call(this, commands, options);
}

/**
 * Creates block iterator over the cluster responses for a given cluster/side.
 *
 * This function is meant to be used inside a {{#chip_*_clusters}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
function chip_cluster_responses(options)
{
  const responses = getResponses.call(this, 'chip_cluster_responses');

  return asBlocks.call(this, responses, options);
}

/**
 * Creates block iterator over the current command arguments for a given cluster/side.
 *
 * This function is meant to be used inside a {{#chip_cluster_commands}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
function chip_cluster_command_arguments(options)
{
  const commandId = checkIsInsideCommandBlock(this, 'chip_cluster_command_arguments');
  const commands  = getCommands.call(this.parent, 'chip_cluster_commands_argments');

  const filter = command => command.id == commandId;
  return asBlocks.call(this, commands.then(items => items.find(filter).arguments), options);
}

/**
 * Creates block iterator over the current command arguments for a given cluster/side.
 *
 * This function is meant to be used inside a {{#chip_cluster_commands}}
 * block. It will throw otherwise.
 *
 * The arguments list built by this function differs from {{chip_cluster_command_arguments}}.
 * For example, if a command contains a single struct argument "SomeStruct", with the following type:
 *
 * struct SomeStruct {
 *   uint8_t a;
 *   uint16_t b;
 *   uint32_t c;
 * }
 *
 * then that argument will be expanded into 3 arguments (uint8_t a, uint16_t b, uint32_t c).
 *
 * @param {*} options
 */
function chip_cluster_command_arguments_with_structs_expanded(options)
{
  const commandId = checkIsInsideCommandBlock(this, 'chip_cluster_command_arguments');
  const commands  = getCommands.call(this.parent, 'chip_cluster_command_arguments_with_structs_expanded');

  const filter = command => command.id == commandId;
  return asBlocks.call(this, commands.then(items => {
    const item = items.find(filter);
    if (item === undefined) {
      return [];
    }
    return item.expandedArguments || item.arguments;
  }),
      options);
}

/**
 * Creates block iterator over the current response arguments for a given cluster/side.
 *
 * This function is meant to be used inside a {{#chip_cluster_responses}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
function chip_cluster_response_arguments(options)
{
  const commandId = checkIsInsideCommandBlock(this, 'chip_cluster_response_arguments');
  const responses = getResponses.call(this.parent, 'chip_cluster_responses_argments');

  const filter = command => command.id == commandId;
  return asBlocks.call(this, responses.then(items => items.find(filter).arguments), options);
}

/**
 * Returns if a given server cluster has any attributes of type List[T]
 *
 * This function is meant to be used inside a {{#chip_server_clusters}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
function chip_server_has_list_attributes(options)
{
  const { clusterName } = checkIsInsideClusterBlock(this, 'chip_server_has_list_attributes');
  const attributes      = ensureClusters(this).getServerAttributes(clusterName);

  const filter = attribute => attribute.isArray;
  return attributes.then(items => items.find(filter));
}

/**
 * Returns if a given client cluster has any attributes of type List[T]
 *
 * This function is meant to be used inside a {{#chip_client_clusters}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
function chip_client_has_list_attributes(options)
{
  const { clusterName } = checkIsInsideClusterBlock(this, 'chip_client_has_list_attributes');
  const attributes      = ensureClusters(this).getClientAttributes(clusterName);

  const filter = attribute => attribute.isArray;
  return attributes.then(items => items.find(filter));
}

/**
 * Returns if a given server cluster has any reportable attribute
 *
 * This function is meant to be used inside a {{#chip_server_clusters}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
function chip_server_has_reportable_attributes(options)
{
  const { clusterName } = checkIsInsideClusterBlock(this, 'chip_server_has_reportable_attributes');
  const attributes      = ensureClusters(this).getServerAttributes(clusterName);

  const filter = attribute => attribute.isReportableAttribute;
  return attributes.then(items => items.find(filter));
}

/**
 * Creates block iterator over the server side cluster attributes
 * for a given cluster.
 *
 * This function is meant to be used inside a {{#chip_server_clusters}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
function chip_server_cluster_attributes(options)
{
  const { clusterName } = checkIsInsideClusterBlock(this, 'chip_server_cluster_attributes');
  const attributes      = ensureClusters(this).getServerAttributes(clusterName);

  return asBlocks.call(this, attributes, options);
}

/**
 * Creates block iterator over the server side cluster attributes
 * for a given cluster.
 *
 * This function is meant to be used inside a {{#chip_server_clusters}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
function chip_server_cluster_events(options)
{
  const { clusterName } = checkIsInsideClusterBlock(this, 'chip_server_cluster_events');
  const events          = ensureClusters(this).getServerEvents(clusterName);

  return asBlocks.call(this, events, options);
}

function chip_attribute_list_entryTypes(options)
{
  checkIsInsideAttributeBlock(this, 'chip_attribute_list_entry_types');
  return templateUtil.collectBlocks(this.items, options, this);
}

/**
 * Creates block iterator over commands for a given cluster that have the
 * following properties:
 *
 * 1) Are not manufacturer-specific (to exclude MfgSpecificPing)
 * 2) Are available in the isCommandAvailable sense.
 */
function chip_available_cluster_commands(options)
{
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this, 'chip_available_cluster_commands');
  let promise                        = iteratorUtil.all_user_cluster_commands_helper.call(this, options)
                    .then(endpointCommands => endpointCommands.filter(command => {
                      return command.clusterName == clusterName
                          && zclHelper.isCommandAvailable(
                              clusterSide, command.incoming, command.outgoing, command.commandSource, command.name)
                          && /* exclude MfgSpecificPing */ !command.mfgCode;
                    }))
                    .then(filteredCommands => templateUtil.collectBlocks(filteredCommands, options, this));
  return promise;
}

/**
 * Creates block iterator over structures belonging to the current cluster
 */
async function chip_cluster_specific_structs(options)
{
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this, 'chip_cluster_specific_structs');

  const structs = await ensureClusters(this).getStructuresByClusterName(clusterName);

  return templateUtil.collectBlocks(structs, options, this);
}

/**
 * Creates block iterator over structures that are shared between clusters
 */
async function chip_shared_structs(options)
{
  const structs = await ensureClusters(this).getSharedStructs();
  return templateUtil.collectBlocks(structs, options, this);
}

async function chip_endpoints(options)
{
  const endpoints = await ensureClusters(this).getEndPoints();
  return templateUtil.collectBlocks(endpoints, options, this);
}

async function chip_endpoint_clusters(options)
{
  const clusters = this.clusters;
  return templateUtil.collectBlocks(clusters, options, this);
}

/**
 * Helper checks if the type for the bitmap is BitFlags. This generally includes
 * all bitmaps apart from
 * bitmap8/16/32 (generally defined in types.xml)
 * example:
 * {{#if_is_strongly_typed_bitmap type}}
 * strongly typed bitmap
 * {{else}}
 * not a strongly typed bitmap
 * {{/if_is_strongly_typed_bitmap}}
 *
 * @param {*} type
 * @returns Promise of content.
 */
async function if_is_strongly_typed_bitmap(type, options)
{
  let packageId = await templateUtil.ensureZclPackageId(this);
  let bitmap;
  if (type && typeof type === 'string') {
    bitmap = await queryZcl.selectBitmapByName(this.global.db, packageId, type);
  } else {
    bitmap = await queryZcl.selectBitmapById(this.global.db, type);
  }

  if (bitmap) {
    let a = await queryZcl.selectAtomicType(this.global.db, packageId, bitmap.name);
    if (a) {
      // If this is an atomic type, it's a generic, weakly typed, bitmap.
      return options.inverse(this);
    } else {
      return options.fn(this);
    }
  }
  return options.inverse(this);
}

/**
 * Handlebar helper function which checks if an enum is a strongly typed enum or
 * not. This generally includes all enums apart from
 * enum8/16/32 (generally defined in types.xml)
 * example for if_is_strongly_typed_chip_enum:
 * {{#if_is_strongly_typed_chip_enum type}}
 * strongly typed enum
 * {{else}}
 * not a strongly typed enum
 * {{/if_is_strongly_typed_chip_enum}}
 *
 * @param {*} type
 * @param {*} options
 * @returns Promise of content.
 */
async function if_is_strongly_typed_chip_enum(type, options)
{
  // There are certain exceptions.
  if (type.toLowerCase() == 'vendor_id') {
    return options.fn(this);
  } else {
    let packageId = await templateUtil.ensureZclPackageId(this);
    let enumRes;
    // Retrieving the enum from the enum table
    if (type && typeof type === 'string') {
      enumRes = await queryZcl.selectEnumByName(this.global.db, type, packageId);
    } else {
      enumRes = await queryZcl.selectEnumById(this.global.db, type);
    }

    // Checking if an enum is atomic. If an enum is not atomic then the enum
    // is a strongly typed enum
    if (enumRes) {
      let a = await queryZcl.selectAtomicType(this.global.db, packageId, enumRes.name);
      if (a) {
        // if an enum has an atomic type that means it's a weakly-typed enum.
        return options.inverse(this);
      } else {
        return options.fn(this);
      }
    }
    return options.inverse(this);
  }
}

/**
 * Checks whether a type is an enum for purposes of its chipType.  That includes
 * both spec-defined enum types and types that we map to enum types in our code.
 */
async function if_chip_enum(type, options)
{
  if (type.toLowerCase() == 'vendor_id') {
    return options.fn(this);
  }

  let pkgId       = await templateUtil.ensureZclPackageId(this);
  let checkResult = await zclHelper.isEnum(this.global.db, type, pkgId);
  let result;
  if (checkResult != 'unknown') {
    result = options.fn(this);
  } else {
    result = options.inverse(this);
  }
  return templateUtil.templatePromise(this.global, result);
}

async function if_chip_complex(options)
{
  // `zcl_command_arguments` has an `isArray` property and `type`
  // contains the array element type.
  if (this.isArray) {
    return options.fn(this);
  }

  // zcl_attributes iterators does not expose an `isArray` property
  // and `entryType` contains the array element type, while `type`
  // contains the atomic type, which is array in this case.
  // https://github.com/project-chip/zap/issues/412
  if (this.type == 'array') {
    return options.fn(this);
  }

  let pkgId       = await templateUtil.ensureZclPackageId(this);
  let checkResult = await zclHelper.isStruct(this.global.db, this.type, pkgId);
  let result;
  if (checkResult != 'unknown') {
    result = options.fn(this);
  } else {
    result = options.inverse(this);
  }
  return templateUtil.templatePromise(this.global, result);
}

async function chip_access_elements(options)
{

  // console.log(options);
  let entityType = options.hash.entity

  if (entityType == null)
  {
    throw new Error('Access helper requires entityType, either from context, or from the entity="<entityType>" option.')
  }

  let accessList = null

  // Exaples of operations:
  //   { operation: null, role: null, accessModifier: 'fabric-scoped' },
  //   { operation: 'read', role: 'administer', accessModifier: null },
  //   { operation: 'write', role: 'administer', accessModifier: null }
  //
  // Note the existence of a null operation with a modifier of fabric-scoped

  // accessDefaults contains acceptable operations
  // together with their default value
  let accessDefaults = new Map();

  switch (entityType) {
  case 'attribute':
    accessList = await queryAccess.selectAttributeAccess(this.global.db, this.id);
    accessDefaults.set('read', 'view');
    accessDefaults.set('write', 'operate');
    break;
  case 'command':
    accessList = await queryAccess.selectCommandAccess(this.global.db, this.id);
    accessDefaults.set('invoke', 'operate');
    break;
  case 'event':
    accessList = await queryAccess.selectEventAccess(this.global.db, this.id);
    accessDefaults.set('read', 'view');
    break;
  default:
    throw new Error(`Entity type ${entityType} not supported. Requires: attribute/command/event.`)
  }

  let accessEntries = [];

  for (element of accessList) {
    if (!element.operation) {
      continue; // not a valid operation (likely null)
    }

    const operation = element.operation.toLowerCase();
    if (!accessDefaults.has(operation)) {
      continue; // not a valid operation (may be a bug or non-matter operation)
    }

    const role = element.role.toLowerCase();

    if (role === accessDefaults.get(operation)) {
      continue; // already set as a default
    }

    accessEntries.push({ operation, role })
  }

  let p = templateUtil.collectBlocks(accessEntries, options, this)
  return templateUtil.templatePromise(this.global, p)
}

//
// Module exports
//
exports.chip_clusters                                        = chip_clusters;
exports.chip_has_clusters                                    = chip_has_clusters;
exports.chip_client_clusters                                 = chip_client_clusters;
exports.chip_has_client_clusters                             = chip_has_client_clusters;
exports.chip_server_clusters                                 = chip_server_clusters;
exports.chip_has_server_clusters                             = chip_has_server_clusters;
exports.chip_cluster_commands                                = chip_cluster_commands;
exports.chip_cluster_command_arguments                       = chip_cluster_command_arguments;
exports.chip_cluster_command_arguments_with_structs_expanded = chip_cluster_command_arguments_with_structs_expanded;
exports.chip_server_global_responses                         = chip_server_global_responses;
exports.chip_cluster_responses                               = chip_cluster_responses;
exports.chip_cluster_response_arguments                      = chip_cluster_response_arguments
exports.chip_attribute_list_entryTypes                       = chip_attribute_list_entryTypes;
exports.chip_server_cluster_attributes                       = chip_server_cluster_attributes;
exports.chip_server_cluster_events                           = chip_server_cluster_events;
exports.chip_server_has_list_attributes                      = chip_server_has_list_attributes;
exports.chip_server_has_reportable_attributes                = chip_server_has_reportable_attributes;
exports.chip_available_cluster_commands                      = chip_available_cluster_commands;
exports.chip_endpoints                                       = chip_endpoints;
exports.chip_endpoint_clusters                               = chip_endpoint_clusters;
exports.if_chip_enum                                         = if_chip_enum;
exports.if_chip_complex                                      = if_chip_complex;
exports.if_basic_global_response                             = if_basic_global_response;
exports.chip_cluster_specific_structs                        = chip_cluster_specific_structs;
exports.chip_shared_structs                                  = chip_shared_structs;
exports.chip_access_elements                                 = chip_access_elements
exports.if_is_strongly_typed_chip_enum                       = if_is_strongly_typed_chip_enum
exports.if_is_strongly_typed_bitmap                          = if_is_strongly_typed_bitmap
