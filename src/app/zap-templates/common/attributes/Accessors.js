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

const zapPath      = '../../../../../third_party/zap/repo/dist/src-electron/';
const ListHelper   = require('../../common/ListHelper.js');
const StringHelper = require('../../common/StringHelper.js');
const appHelper    = require('../../templates/app/helper.js');
const cHelper      = require(zapPath + 'generator/helper-c.js')
const zclHelper    = require(zapPath + 'generator/helper-zcl.js')
const templateUtil = require(zapPath + 'generator/template-util.js')
const zclUtil      = require(zapPath + 'util/zcl-util.js')

// Not sure what to do with EUI64 yet.
const unsupportedTypes = [ 'EUI64' ];
function isUnsupportedType(type)
{
  return unsupportedTypes.includes(type.toUpperCase());
}

function canHaveSimpleAccessors(attr)
{
  if (attr.isArray) {
    return false;
  }

  if (ListHelper.isList(attr.type)) {
    return false;
  }

  // We can't check for being a struct synchronously, so that's handled manually
  // in the template.
  if (isUnsupportedType(attr.type)) {
    return false;
  }

  return true;
}

async function accessorGetterType(attr)
{
  let type;
  let mayNeedPointer = false;
  if (StringHelper.isCharString(attr.type)) {
    type = "chip::MutableCharSpan";
  } else if (StringHelper.isOctetString(attr.type)) {
    type = "chip::MutableByteSpan";
  } else {
    mayNeedPointer = true;
    const options  = { 'hash' : { forceNotNullable : true, forceNotOptional : true, ns : this.parent.name } };
    type           = await appHelper.zapTypeToEncodableClusterObjectType.call(this, attr.type, options);
  }

  if (attr.isNullable) {
    type = `DataModel::Nullable<${type}> &`;
  } else if (mayNeedPointer) {
    type = `${type} *`;
  }

  return type;
}

async function accessorTraitType(type)
{
  let temp    = type.toLowerCase();
  let matches = temp.match(/^int([0-9]+)(s?)/i);
  if (matches) {
    let signed = matches[2] != "";
    let size   = parseInt(matches[1]) / 8;

    if (size != 1 && size != 2 && size != 4 && size != 8) {
      return `OddSizedInteger<${size}, ${signed}>`;
    }
  }

  const options = { 'hash' : { forceNotNullable : true, forceNotOptional : true, ns : this.parent.name } };
  return appHelper.zapTypeToEncodableClusterObjectType.call(this, type, options);
}

async function typeAsDelimitedMacro(type)
{
  const { db }   = this.global;
  const pkgId    = await templateUtil.ensureZclPackageId(this);
  const typeInfo = await zclUtil.determineType(db, type, pkgId);
  return cHelper.asDelimitedMacro.call(this, typeInfo.atomicType);
}

//
// Module exports
//
exports.canHaveSimpleAccessors = canHaveSimpleAccessors;
exports.accessorGetterType     = accessorGetterType;
exports.accessorTraitType      = accessorTraitType;
exports.typeAsDelimitedMacro   = typeAsDelimitedMacro;
