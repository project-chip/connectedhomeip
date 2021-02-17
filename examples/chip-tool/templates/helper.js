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
exports.asDelimitedCommand      = asDelimitedCommand;
exports.asTypeMinValue          = asTypeMinValue;
exports.asTypeMaxValue          = asTypeMaxValue;
exports.isStrEndsWith           = isStrEndsWith;
