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
const templateUtil = require(zapPath + 'generator/template-util.js');
const zclHelper    = require(zapPath + 'generator/helper-zcl.js');
const iteratorUtil = require(zapPath + 'util/iterator-util.js');

const { Clusters, asBlocks, asPromise } = require('../../common/ClustersHelper.js');
const StringHelper                      = require('../../common/StringHelper.js');
const ChipTypesHelper                   = require('../../common/ChipTypesHelper.js');

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
  return clusterSide == 'client' ? Clusters.getClientCommands(clusterName) : Clusters.getServerCommands(clusterName);
}

function getResponses(methodName)
{
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this, methodName);
  return clusterSide == 'client' ? Clusters.getClientResponses(clusterName) : Clusters.getServerResponses(clusterName);
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
 * Creates block iterator over the server global responses
 *
 * @param {*} options
 */
function chip_server_global_responses(options)
{
  const sorter = (a, b) => a.chipCallback.name.localeCompare(b.chipCallback.name, 'en', { numeric : true });

  const reducer = (unique, item) => {
    const { type, size, isList, chipCallback, chipType } = item.response.arguments[0];

    // List-typed elements have a dedicated callback
    if (isList) {
      return unique;
    }

    if (unique.find(item => item.chipCallback.name == chipCallback.name)) {
      return unique;
    }

    return [...unique, { chipCallback, chipType, size } ];
  };

  const filter = attributes => attributes.reduce(reducer, []).sort(sorter);
  return asBlocks.call(this, Clusters.getAttributesByClusterSide('server').then(filter), options);
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
  const attributes      = Clusters.getServerAttributes(clusterName);

  const filter = attribute => attribute.isList;
  return asPromise.call(this, attributes.then(items => items.find(filter)));
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
  const attributes      = Clusters.getClientAttributes(clusterName);

  const filter = attribute => attribute.isList;
  return asPromise.call(this, attributes.then(items => items.find(filter)));
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
  const attributes      = Clusters.getServerAttributes(clusterName);

  return asBlocks.call(this, attributes, options);
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

//
// Module exports
//
exports.chip_clusters                   = chip_clusters;
exports.chip_has_clusters               = chip_has_clusters;
exports.chip_client_clusters            = chip_client_clusters;
exports.chip_has_client_clusters        = chip_has_client_clusters;
exports.chip_server_clusters            = chip_server_clusters;
exports.chip_has_server_clusters        = chip_has_server_clusters;
exports.chip_cluster_commands           = chip_cluster_commands;
exports.chip_cluster_command_arguments  = chip_cluster_command_arguments;
exports.chip_server_global_responses    = chip_server_global_responses;
exports.chip_cluster_responses          = chip_cluster_responses;
exports.chip_cluster_response_arguments = chip_cluster_response_arguments
exports.chip_attribute_list_entryTypes  = chip_attribute_list_entryTypes;
exports.chip_server_cluster_attributes  = chip_server_cluster_attributes;
exports.chip_server_has_list_attributes = chip_server_has_list_attributes;
exports.chip_available_cluster_commands = chip_available_cluster_commands;
