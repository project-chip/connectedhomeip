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
const zapPath      = '../../../../../third_party/zap/repo/dist/src-electron/';
const string       = require(zapPath + 'util/string.js')
const templateUtil = require(zapPath + 'generator/template-util.js')
const zclHelper    = require(zapPath + 'generator/helper-zcl.js')

const ChipTypesHelper = require('../../../../../src/app/zap-templates/common/ChipTypesHelper.js');
const StringHelper    = require('../../../../../src/app/zap-templates/common/StringHelper.js');
const appHelper       = require('../../../../../src/app/zap-templates/templates/app/helper.js');

// Ideally those clusters clusters endpoints should be retrieved from the
// descriptor cluster.
function asExpectedEndpointForCluster(clusterName)
{
  switch (clusterName) {
  case 'AdministratorCommissioning':
  case 'Basic':
  case 'Descriptor':
  case 'GeneralCommissioning':
  case 'GeneralDiagnostics':
  case 'SoftwareDiagnostics':
  case 'ThreadNetworkDiagnostics':
  case 'EthernetNetworkDiagnostics':
  case 'WiFiNetworkDiagnostics':
  case 'GroupKeyManagement':
  case 'NetworkCommissioning':
  case 'OperationalCredentials':
  case 'TrustedRootCertificates':
  case 'OtaSoftwareUpdateProvider':
  case 'OtaSoftwareUpdateRequestor':
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
    return `@(${this.min || this.max || 0})`;
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
          case 'bool':
            return 'Bool';
          case 'uint8_t':
            return 'UnsignedChar';
          case 'uint16_t':
            return 'UnsignedShort';
          case 'uint32_t':
            return 'UnsignedInt';
          case 'uint64_t':
            return 'UnsignedLongLong';
          case 'int8_t':
            return 'Char';
          case 'int16_t':
            return 'Short';
          case 'int32_t':
            return 'Int';
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

async function asObjectiveCClass(type, cluster, options)
{
  let pkgId    = await templateUtil.ensureZclPackageId(this);
  let isStruct = await zclHelper.isStruct(this.global.db, type, pkgId).then(zclType => zclType != 'unknown');

  if ((this.isList || this.isArray || this.entryType) && !options.hash.forceNotList) {
    return 'NSArray';
  }

  if (StringHelper.isOctetString(type)) {
    return 'NSData';
  }

  if (StringHelper.isCharString(type)) {
    return 'NSString';
  }

  if (isStruct) {
    return `CHIP${appHelper.asUpperCamelCase(cluster)}Cluster${appHelper.asUpperCamelCase(type)}`;
  }

  return 'NSNumber';
}

async function asObjectiveCType(type, cluster, options)
{
  let typeStr = await asObjectiveCClass.call(this, type, cluster, options);
  if (this.isNullable || this.isOptional) {
    typeStr = `${typeStr} * _Nullable`;
  } else {
    typeStr = `${typeStr} * _Nonnull`;
  }

  return typeStr;
}

async function arrayElementObjectiveCClass(type, cluster, options)
{
  options.hash.forceNotList = true;
  return asObjectiveCClass.call(this, type, cluster, options);
}

function incrementDepth(depth)
{
  return depth + 1;
}

function asStructPropertyName(prop)
{
  prop = appHelper.asLowerCamelCase(prop);

  // If prop is now "description", we need to rename it, because that's
  // reserved.
  if (prop == "description") {
    return "descriptionString";
  }

  // If prop starts with a sequence of capital letters (which can happen for
  // output of asLowerCamelCase if the original string started that way,
  // lowercase all but the last one.
  return prop.replace(/^([A-Z]+)([A-Z])/, (match, p1, p2) => { return p1.toLowerCase() + p2 });
}

function asGetterName(prop)
{
  let propName = asStructPropertyName(prop);
  if (propName.match(/^new[A-Z]/) || propName == "count") {
    return "get" + appHelper.asUpperCamelCase(prop);
  }
  return propName;
}

function commandHasRequiredField(command)
{
  return command.arguments.some(arg => !arg.isOptional);
}

//
// Module exports
//
exports.asObjectiveCBasicType        = asObjectiveCBasicType;
exports.asObjectiveCNumberType       = asObjectiveCNumberType;
exports.asExpectedEndpointForCluster = asExpectedEndpointForCluster;
exports.asTestIndex                  = asTestIndex;
exports.asTestValue                  = asTestValue;
exports.asObjectiveCClass            = asObjectiveCClass;
exports.asObjectiveCType             = asObjectiveCType;
exports.arrayElementObjectiveCClass  = arrayElementObjectiveCClass;
exports.incrementDepth               = incrementDepth;
exports.asStructPropertyName         = asStructPropertyName;
exports.asGetterName                 = asGetterName;
exports.commandHasRequiredField      = commandHasRequiredField;
