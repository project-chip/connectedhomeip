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
 * Creates block iterator over the server side cluster command
 * for a given cluster.
 *
 * This function is meant to be used inside a {{#chip_server_clusters}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
function chip_server_cluster_commands(options)
{
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this, 'chip_server_cluster_commands');
  return asBlocks.call(this, Clusters.getClientCommands(clusterName), options);
}

/**
 * Creates block iterator over the server side cluster responses
 * for a given cluster.
 *
 * This function is meant to be used inside a {{#chip_server_clusters}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
function chip_server_cluster_responses(options)
{
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this, 'chip_server_cluster_response');
  return asBlocks.call(this, Clusters.getServerResponses(clusterName), options);
}

/**
 * Creates block iterator over the server side cluster command arguments
 * for a given command.
 *
 * This function is meant to be used inside a {{#chip_server_cluster_commands}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
function chip_server_cluster_command_arguments(options)
{
  const commandId                    = checkIsInsideCommandBlock(this, 'chip_server_cluster_command_arguments');
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this.parent, 'chip_server_cluster_command_arguments');

  const filter = command => command.id == commandId;
  const promise          = Clusters.getClientCommands(clusterName).then(commands => commands.find(filter).arguments);
  return asBlocks.call(this, promise, options);
}

/**
 * Creates block iterator over the server side cluster response arguments
 * for a given command.
 *
 * This function is meant to be used inside a {{#chip_server_cluster_responses}}
 * block. It will throw otherwise.
 *
 * @param {*} options
 */
function chip_server_cluster_response_arguments(options)
{
  const commandId                    = checkIsInsideCommandBlock(this, 'chip_server_cluster_response_arguments');
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this.parent, 'chip_server_cluster_response_arguments');

  const filter = command => command.id == commandId;
  const promise          = Clusters.getServerResponses(clusterName).then(commands => commands.find(filter).arguments);
  return asBlocks.call(this, promise, options);
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

  const filter = attribute => attribute.isList;
  return asPromise.call(this, Clusters.getServerAttributes(clusterName).then(attributes => attributes.find(filter)));
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

  const filter = attribute => attribute.isList;
  return asPromise.call(this, Clusters.getClientAttributes(clusterName).then(attributes => attributes.find(filter)));
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
  const { clusterName, clusterSide } = checkIsInsideClusterBlock(this, 'chip_server_cluster_attributes');
  return asBlocks.call(this, Clusters.getServerAttributes(clusterName), options);
}

function chip_attribute_list_entryTypes(options)
{
  checkIsInsideAttributeBlock(this, 'chip_attribute_list_entry_types');
  return templateUtil.collectBlocks(this.items, options, this);
}

//
// Module exports
//
exports.chip_clusters                          = chip_clusters;
exports.chip_has_clusters                      = chip_has_clusters;
exports.chip_client_clusters                   = chip_client_clusters;
exports.chip_has_client_clusters               = chip_has_client_clusters;
exports.chip_server_clusters                   = chip_server_clusters;
exports.chip_has_server_clusters               = chip_has_server_clusters;
exports.chip_server_cluster_commands           = chip_server_cluster_commands;
exports.chip_server_cluster_command_arguments  = chip_server_cluster_command_arguments
exports.chip_server_global_responses           = chip_server_global_responses;
exports.chip_server_cluster_responses          = chip_server_cluster_responses;
exports.chip_server_cluster_response_arguments = chip_server_cluster_response_arguments
exports.chip_attribute_list_entryTypes         = chip_attribute_list_entryTypes;
exports.chip_server_cluster_attributes         = chip_server_cluster_attributes;
exports.chip_server_has_list_attributes        = chip_server_has_list_attributes;
