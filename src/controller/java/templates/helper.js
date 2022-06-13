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

const ChipTypesHelper = require('../../../../src/app/zap-templates/common/ChipTypesHelper.js');
const StringHelper    = require('../../../../src/app/zap-templates/common/StringHelper.js');
const appHelper       = require('../../../../src/app/zap-templates/templates/app/helper.js');

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

function asJavaBoxedType(type, zclType)
{
  if (StringHelper.isOctetString(type)) {
    return 'byte[]';
  } else if (StringHelper.isCharString(type)) {
    return 'String';
  } else {
    try {
      return convertBasicCTypeToJavaBoxedType(ChipTypesHelper.asBasicType(zclType));
    } catch (error) {
      // Unknown type, default to Object.
      return "Object";
    }
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

function asJniSignatureBasic(type, useBoxedTypes)
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
  case 'double':
    return 'D';
  case 'float':
    return 'F';
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

async function asUnderlyingBasicType(type)
{
  const options = { 'hash' : {} };
  let zclType   = await zclHelper.asUnderlyingZclType.call(this, type, options);
  return ChipTypesHelper.asBasicType(zclType);
}

async function asJavaType(type, zclType, cluster, options)
{
  let pkgId = await templateUtil.ensureZclPackageId(this);
  if (zclType == null) {
    const options = { 'hash' : {} };
    zclType       = await zclHelper.asUnderlyingZclType.call(this, type, options);
  }
  let isStruct = await zclHelper.isStruct(this.global.db, type, pkgId).then(zclType => zclType != 'unknown');

  let classType = "";

  if (StringHelper.isOctetString(type)) {
    classType += 'byte[]';
  } else if (StringHelper.isCharString(type)) {
    classType += 'String';
  } else if (isStruct) {
    classType += `ChipStructs.${appHelper.asUpperCamelCase(cluster)}Cluster${appHelper.asUpperCamelCase(type)}`;
  } else {
    classType += asJavaBoxedType(type, zclType);
  }

  if (!options.hash.underlyingType) {
    if (!options.hash.forceNotList && (this.isArray || this.entryType)) {
      if (!options.hash.removeGenericType) {
        classType = 'ArrayList<' + classType + '>';
      } else {
        classType = 'ArrayList';
      }
    }

    if (this.isOptional) {
      if (!options.hash.removeGenericType) {
        classType = 'Optional<' + classType + '>';
      } else {
        classType = 'Optional';
      }
    }

    if (this.isNullable && options.hash.includeAnnotations) {
      classType = '@Nullable ' + classType;
    }
  }

  return classType;
}

async function asJniType(type, zclType, cluster, options)
{
  let types = await asJniHelper.call(this, type, zclType, cluster, options);
  return types["jniType"];
}

async function asJniSignature(type, zclType, cluster, useBoxedTypes, options)
{
  let types = await asJniHelper.call(this, type, zclType, cluster, options);
  return useBoxedTypes ? types["jniBoxedSignature"] : types["jniSignature"];
}

async function asJniClassName(type, zclType, cluster, options)
{
  let types = await asJniHelper.call(this, type, zclType, cluster, options);
  return types["jniClassName"];
}

async function asJniHelper(type, zclType, cluster, options)
{
  let pkgId = await templateUtil.ensureZclPackageId(this);
  if (zclType == null) {
    zclType = await zclHelper.asUnderlyingZclType.call(this, type, options);
  }
  let isStruct = await zclHelper.isStruct(this.global.db, type, pkgId).then(zclType => zclType != 'unknown');

  if (this.isOptional) {
    const signature = "Ljava/util/Optional;"
    return { jniType : "jobject", jniSignature : signature, jniBoxedSignature : signature };
  }

  if (this.isArray) {
    const signature = "Ljava/util/ArrayList;"
    return { jniType : "jobject", jniSignature : signature, jniBoxedSignature : signature };
  }

  if (StringHelper.isOctetString(type)) {
    const signature = "[B";
    return { jniType : "jbyteArray", jniSignature : signature, jniBoxedSignature : signature };
  }

  if (StringHelper.isCharString(type)) {
    const signature = "Ljava/lang/String;";
    return { jniType : "jstring", jniSignature : signature, jniBoxedSignature : signature };
  }

  if (isStruct) {
    const signature
        = `Lchip/devicecontroller/ChipStructs$${appHelper.asUpperCamelCase(cluster)}Cluster${appHelper.asUpperCamelCase(type)};`;
    return { jniType : "jobject", jniSignature : signature, jniBoxedSignature : signature };
  }

  let jniBoxedSignature;
  try {
    jniBoxedSignature = await asJniSignatureBasic.call(this, type, true);
  } catch (error) {
    jniBoxedSignature = "Ljava/lang/Object;";
  }
  let jniSignature;
  try {
    jniSignature = await asJniSignatureBasic.call(this, type, false);
  } catch (error) {
    jniSignature = "Ljava/lang/Object;";
  }
  // Example: Ljava/lang/Integer; -> java/lang/Integer, needed for JNI class lookup
  let jniClassName = jniBoxedSignature.substring(1, jniBoxedSignature.length - 1);
  return {
    jniType : asJniBasicType(type, true),
    jniSignature : jniSignature,
    jniBoxedSignature : jniBoxedSignature,
    jniClassName : jniClassName
  };
}

function incrementDepth(depth)
{
  return depth + 1;
}

//
// Module exports
//
exports.asUnderlyingBasicType                  = asUnderlyingBasicType;
exports.asJavaType                             = asJavaType;
exports.asJavaBoxedType                        = asJavaBoxedType;
exports.asJniType                              = asJniType;
exports.asJniSignature                         = asJniSignature;
exports.asJniClassName                         = asJniClassName;
exports.asJniBasicType                         = asJniBasicType;
exports.asJniSignatureBasic                    = asJniSignatureBasic;
exports.convertBasicCTypeToJniType             = convertBasicCTypeToJniType;
exports.convertCTypeToJniSignature             = convertCTypeToJniSignature;
exports.convertBasicCTypeToJavaBoxedType       = convertBasicCTypeToJavaBoxedType;
exports.convertAttributeCallbackTypeToJavaName = convertAttributeCallbackTypeToJavaName;
exports.incrementDepth                         = incrementDepth;
