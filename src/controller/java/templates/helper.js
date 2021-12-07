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
const zapPath      = '../../../../third_party/zap/repo/dist/src-electron/';
const templateUtil = require(zapPath + 'generator/template-util.js')
const zclHelper    = require(zapPath + 'generator/helper-zcl.js')
const queryCommand = require(zapPath + 'db/query-command.js')

const ChipTypesHelper = require('../../../../src/app/zap-templates/common/ChipTypesHelper.js');
const StringHelper    = require('../../../../src/app/zap-templates/common/StringHelper.js');
const ChipHelper      = require('../../../../src/app/zap-templates/templates/chip/helper.js');

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
  // TODO(#8074): replace with BigInteger, or mark as unsigned.
  case 'uint64_t':
  case 'int64_t':
    return 'long';
  case 'bool':
    return 'boolean';
  case 'float':
    return 'float';
  case 'double':
    return 'double';
  default:
    error = 'Unhandled type ' + cType;
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
  case 'boolean':
    return 'jboolean';
  case 'float':
    return 'jfloat';
  case 'double':
    return 'jdouble';
  default:
    error = 'Unhandled type ' + cType;
    throw error;
  }
}

function convertBasicCTypeToJavaBoxedType(cType)
{
  switch (convertBasicCTypeToJavaType(cType)) {
  case 'int':
    return 'Integer';
  case 'long':
    return 'Long';
  case 'boolean':
    return 'Boolean';
  case 'float':
    return 'Float';
  case 'double':
    return 'Double';
  default:
    error = 'Unhandled type ' + cType;
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

function asJavaBoxedType(type)
{
  if (StringHelper.isOctetString(type)) {
    return 'byte[]';
  } else if (StringHelper.isCharString(type)) {
    return 'String';
  } else {
    return convertBasicCTypeToJavaBoxedType(ChipTypesHelper.asBasicType(this.chipType));
  }
}

function asJniBasicType(type, useBoxedTypes)
{
  if (this.isOptional) {
    return 'jobject';
  } else if (StringHelper.isOctetString(type)) {
    return 'jbyteArray';
  } else if (StringHelper.isCharString(type)) {
    return 'jstring';
  } else {
    if (useBoxedTypes) {
      return 'jobject';
    }
    return convertBasicCTypeToJniType(ChipTypesHelper.asBasicType(this.chipType));
  }
}

function asJavaBasicTypeForZclType(type, useBoxedTypes)
{
  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      if (useBoxedTypes) {
        return convertBasicCTypeToJavaBoxedType(ChipTypesHelper.asBasicType(zclType));
      } else {
        return convertBasicCTypeToJavaType(ChipTypesHelper.asBasicType(zclType));
      }
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

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => {
    console.log(err);
    throw err;
  });
  return templateUtil.templatePromise(this.global, promise)
}

function asJniSignature(type, useBoxedTypes)
{
  function fn(pkgId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      return convertCTypeToJniSignature(ChipTypesHelper.asBasicType(zclType), useBoxedTypes);
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => {
    console.log(err);
    throw err;
  });
  return templateUtil.templatePromise(this.global, promise)
}

function convertCTypeToJniSignature(cType, useBoxedTypes)
{
  let javaType;
  if (useBoxedTypes) {
    javaType = convertBasicCTypeToJavaBoxedType(cType);
  } else {
    javaType = convertBasicCTypeToJavaType(cType);
  }

  switch (javaType) {
  case 'int':
    return 'I';
  case 'long':
    return 'J';
  case 'boolean':
    return 'Z';
  case 'Boolean':
    return 'Ljava/lang/Boolean;';
  case 'Integer':
    return 'Ljava/lang/Integer;';
  case 'Long':
    return 'Ljava/lang/Long;';
  case 'double':
    return 'D';
  case 'Double':
    return 'Ljava/lang/Double;';
  case 'float':
    return 'F';
  case 'Float':
    return 'Ljava/lang/Float;';
  default:
    error = 'Unhandled Java type ' + javaType + ' for C type ' + cType;
    throw error;
  }
}

function convertAttributeCallbackTypeToJavaName(cType)
{
  // These correspond to OctetStringAttributeCallback and CharStringAttributeCallback in ChipClusters-java.zapt.
  if (StringHelper.isOctetString(this.type)) {
    return 'OctetString';
  } else if (StringHelper.isCharString(this.type)) {
    return 'CharString';
  } else {
    return convertBasicCTypeToJavaBoxedType(cType);
  }
}

function notLastSupportedEntryTypes(context, options)
{
  if (context.items.length == 0) {
    return
  }

  let lastIndex = context.items.length - 1;
  while (context.items[lastIndex].isStruct || context.items[lastIndex].isArray) {
    lastIndex--;
  }

  if (this.index != lastIndex) {
    return options.fn(this);
  }
}

function notLastSupportedCommandResponseType(items, options)
{
  if (items.length == 0) {
    return
  }

  let lastIndex = items.length - 1;
  while (items[lastIndex].isArray) {
    lastIndex--;
  }

  if (this.index != lastIndex) {
    return options.fn(this);
  }
}

//
// Module exports
//
exports.asJavaBasicType                        = asJavaBasicType;
exports.asJavaBoxedType                        = asJavaBoxedType;
exports.asJniBasicType                         = asJniBasicType;
exports.asJniBasicTypeForZclType               = asJniBasicTypeForZclType;
exports.asJniSignature                         = asJniSignature;
exports.asJavaBasicTypeForZclType              = asJavaBasicTypeForZclType;
exports.convertBasicCTypeToJniType             = convertBasicCTypeToJniType;
exports.convertCTypeToJniSignature             = convertCTypeToJniSignature;
exports.convertBasicCTypeToJavaBoxedType       = convertBasicCTypeToJavaBoxedType;
exports.convertAttributeCallbackTypeToJavaName = convertAttributeCallbackTypeToJavaName;
exports.notLastSupportedEntryTypes             = notLastSupportedEntryTypes;
exports.notLastSupportedCommandResponseType    = notLastSupportedCommandResponseType;
