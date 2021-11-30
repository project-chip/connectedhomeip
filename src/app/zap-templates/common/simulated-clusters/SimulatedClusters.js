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

const { Clusters }      = require('../ClustersHelper.js');
const { DelayCommands } = require('./TestDelayCommands.js');
const { LogCommands }   = require('./TestLogCommands.js');

const SimulatedClusters = [
  DelayCommands,
  LogCommands,
];

function getSimulatedCluster(clusterName)
{
  return SimulatedClusters.find(cluster => cluster.name == clusterName);
}

function getClusters()
{
  return Clusters.getClusters().then(clusters => clusters.concat(SimulatedClusters).flat(1));
}

function getCommands(clusterName)
{
  const cluster = getSimulatedCluster(clusterName);
  return cluster ? Promise.resolve(cluster.commands) : Clusters.getClientCommands(clusterName);
}

function getAttributes(clusterName)
{
  const cluster = getSimulatedCluster(clusterName);
  return cluster ? Promise.resolve(cluster.attributes) : Clusters.getServerAttributes(clusterName);
}

function isTestOnlyCluster(clusterName)
{
  return !!getSimulatedCluster(clusterName);
}

//
// Module exports
//
exports.getClusters       = getClusters;
exports.getCommands       = getCommands;
exports.getAttributes     = getAttributes;
exports.isTestOnlyCluster = isTestOnlyCluster;
