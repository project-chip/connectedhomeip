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
const zapPath      = '../../../../../third_party/zap/repo/src-electron/';
const templateUtil = require(zapPath + 'generator/template-util.js')
const zclHelper    = require(zapPath + 'generator/helper-zcl.js')

const ChipTypesHelper = require('../../../../../src/app/zap-templates/common/ChipTypesHelper.js');
const StringHelper    = require('../../../../../src/app/zap-templates/common/StringHelper.js');

// Ideally those clusters clusters endpoints should be retrieved from the
// descriptor cluster.
function asExpectedEndpointForCluster(clusterName)
{
  switch (clusterName) {
  case 'Basic':
  case 'Descriptor':
  case 'GeneralCommissioning':
  case 'GroupKeyManagement':
  case 'NetworkCommissioning':
  case 'OperationalCredentials':
  case 'TrustedRootCertificates':
    return 0;
  }

  return 1;
}

function asTestValue()
{
  if (StringHelper.isOctetString(this.type)) {
    return '[@"Test" dataUsingEncoding:NSUTF8StringEncoding]';
  } else if (StringHelper.isCharString(this.type)) {
    return '@"Test"';
  } else {
    return this.min || this.max || 0;
  }
}

//
// Module exports
//
exports.asExpectedEndpointForCluster = asExpectedEndpointForCluster;
exports.asTestValue                  = asTestValue;
