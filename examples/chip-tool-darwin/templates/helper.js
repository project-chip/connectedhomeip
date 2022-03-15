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
const zapPath                       = '../../../third_party/zap/repo/dist/src-electron/';
const { asB_locks, ensureClusters } = require('../../../src/app/zap-templates/common/ClustersHelper.js');
const templateUtil                  = require(zapPath + 'generator/template-util.js');
const zclHelper                     = require(zapPath + 'generator/helper-zcl.js');
const zclQuery                      = require(zapPath + 'db/query-zcl.js');

const ChipTypesHelper = require('../../../src/app/zap-templates/common/ChipTypesHelper.js');

function asHyphenatedLower(name)
{
  name = name.replace(/\s+/g, '').replace(/\.?([A-Z])/g, function(x) {
    return '-' + x
  })
  return name.substring(1).toLowerCase();
}

function toLowerCase(name)
{
  return name.toLowerCase();
}

function getCommands()
{
  return ensureClusters(this).getClientCommands(this.name);
}

function hasCommands()
{
  return getCommands.call(this).then(commands => { return !!commands.length });
}

function hasArguments()
{
  return !!this.arguments.length
}

//
// Module exports
//
exports.asHyphenatedLower = asHyphenatedLower;
exports.hasCommands       = hasCommands;
exports.toLowerCase       = toLowerCase
exports.hasArguments      = hasArguments;
