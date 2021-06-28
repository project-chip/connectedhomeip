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
  case 'GeneralDiagnostics':
  case 'SoftwareDiagnostics':
  case 'ThreadNetworkDiagnostics':
  case 'EthernetNetworkDiagnostics':
  case 'GroupKeyManagement':
  case 'NetworkCommissioning':
  case 'OperationalCredentials':
  case 'TrustedRootCertificates':
  case 'OtaSoftwareUpdateServer':
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
    return this.min || this.max || 0;
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

function asObjectiveCNumberType(label, type, asLowerCased)
{
  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options)
        .then(zclType => {
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
        .then(typeName => asLowerCased ? (typeName[0].toLowerCase() + typeName.substring(1)) : typeName);
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise)
}

function asTestIndex(index)
{
  return index.toString().padStart(6, 0);
}

//
// Module exports
//
exports.asObjectiveCBasicType        = asObjectiveCBasicType;
exports.asObjectiveCNumberType       = asObjectiveCNumberType;
exports.asExpectedEndpointForCluster = asExpectedEndpointForCluster;
exports.asTestIndex                  = asTestIndex;
exports.asTestValue                  = asTestValue;
