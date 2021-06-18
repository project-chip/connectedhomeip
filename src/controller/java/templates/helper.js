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
const zapPath      = '../../../../third_party/zap/repo/src-electron/';
const templateUtil = require(zapPath + 'generator/template-util.js')
const zclHelper    = require(zapPath + 'generator/helper-zcl.js')
const queryZcl     = require(zapPath + 'db/query-zcl.js')

const ChipTypesHelper = require('../../../../src/app/zap-templates/common/ChipTypesHelper.js');
const StringHelper    = require('../../../../src/app/zap-templates/common/StringHelper.js');

function convertBasicCTypeToJavaType(cType)
{
  switch (cType) {
  case 'uint8_t':
  case 'int8_t':
  case 'uint16_t':
  case 'int16_t':
    return 'int';
  case 'uint32_t':
  case 'int32_t':
  case 'uint64_t':
  case 'int64_t':
    return 'long';
  default:
    error = ': Unhandled type ' + cType;
    throw error;
  }
}

function convertBasicCTypeToJniType(cType)
{
  switch (convertBasicCTypeToJavaType(cType)) {
  case 'int':
    return 'jint';
  case 'long':
    return 'jlong';
  default:
    error = ': Unhandled type ' + cType;
    throw error;
  }
}

function asJavaBasicType(type)
{
  if (StringHelper.isOctetString(type)) {
    return 'byte[]';
  } else if (StringHelper.isCharString(type)) {
    return 'String';
  } else {
    return convertBasicCTypeToJavaType(ChipTypesHelper.asBasicType(this.chipType));
  }
}

function asJniBasicType(type)
{
  if (StringHelper.isOctetString(type)) {
    return 'jbyteArray';
  } else if (StringHelper.isCharString(type)) {
    return 'jstring';
  } else {
    return convertBasicCTypeToJniType(ChipTypesHelper.asBasicType(this.chipType));
  }
}

function asJavaBasicTypeForZclType(type)
{
  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      return convertBasicCTypeToJavaType(ChipTypesHelper.asBasicType(zclType));
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise)
}

function asJniBasicTypeForZclType(type)
{
  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      return convertBasicCTypeToJniType(ChipTypesHelper.asBasicType(zclType));
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise)
}

function asJniSignature(label, type)
{
  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      switch (convertBasicCTypeToJavaType(ChipTypesHelper.asBasicType(zclType))) {
      case 'int':
        return 'I';
      case 'long':
        return 'J';
      default:
        error = label + ': Unhandled underlying type ' + zclType + ' for original type ' + type;
        throw error;
      }
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise)
}

function omitCommaForFirstNonStatusCommand(id, index)
{
  let promise = templateUtil.ensureZclPackageId(this)
                    .then((pkgId) => { return queryZcl.selectCommandArgumentsByCommandId(this.global.db, id, pkgId) })
                    .catch(err => console.log(err))
                    .then((result) => {
                      // Currently, we omit array types, so don't count it as a valid non-status command.
                      let firstNonStatusCommandIndex = result.findIndex((command) => command.label != "status" && !command.isArray);
                      if (firstNonStatusCommandIndex == -1 || firstNonStatusCommandIndex != index) {
                        return ", ";
                      }
                      return "";
                    })
                    .catch(err => console.log(err));

  return templateUtil.templatePromise(this.global, promise);
}

//
// Module exports
//
exports.asJavaBasicType                   = asJavaBasicType;
exports.asJniBasicType                    = asJniBasicType;
exports.asJniBasicTypeForZclType          = asJniBasicTypeForZclType;
exports.asJniSignature                    = asJniSignature;
exports.asJavaBasicTypeForZclType         = asJavaBasicTypeForZclType;
exports.omitCommaForFirstNonStatusCommand = omitCommaForFirstNonStatusCommand;
