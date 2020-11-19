/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 * This module contains the API for templating. For more detailed instructions, read {@tutorial template-tutorial}
 *
 * @module Templating API: toplevel utility helpers
 */

// Import helpers from zap core
const zapPath      = '../../../third_party/zap/repo/src-electron/';
const cHelper      = require(zapPath + 'generator/helper-c.js')
const zclHelper    = require(zapPath + 'generator/helper-zcl.js')
const zclQuery     = require(zapPath + 'db/query-zcl.js')
const templateUtil = require(zapPath + 'generator/template-util.js')

/**
 * Produces the top-of-the-file header for a C file.
 *
 * @returns The header content
 */
function chip_header()
{
  return `
  /*
  *
  *    Copyright (c) 2020 Project CHIP Authors
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
  */`;
}

const stringShortTypes = [ 'CHAR_STRING', 'OCTET_STRING' ];
const stringLongTypes  = [ 'LONG_CHAR_STRING', 'LONG_OCTET_STRING' ];

function isShortString(type)
{
  return stringShortTypes.includes(type);
}

function isLongString(type)
{
  return stringLongTypes.includes(type);
}

function isString(type)
{
  return isShortString(type) || isLongString(type);
}

function asValueIfNotPresent(type, isArray)
{
  if (isString(type) || isArray) {
    return 'NULL';
  }

  function resolve(packageId)
  {
    const options = { 'hash' : {} };
    return cHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      switch (zclType) {
      case 'uint8_t':
        return 'UINT8_MAX';
      case 'uint16_t':
        return 'UINT16_MAX';
      case 'uint32_t':
        return 'UINT32_MAX';
      default:
        error = 'Unhandled underlying type ' + zclType + ' for original type ' + type;
        throw error;
      }
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(resolve.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise)
}

function asReadTypeLength(type)
{
  if (isShortString(type)) {
    return '1u';
  }

  if (isLongString(type)) {
    return '2u';
  }

  function resolve(packageId)
  {
    const db = this.global.db;

    const defaultResolver = zclQuery.selectAtomicType(db, packageId, type);

    const enumResolver = zclHelper.isEnum(db, type, packageId).then(result => {
      return result == 'unknown' ? null : zclQuery.selectEnumByName(db, type, packageId).then(rec => {
        return zclQuery.selectAtomicType(db, packageId, rec.type);
      });
    });

    const bitmapResolver = zclHelper.isBitmap(db, type, packageId).then(result => {
      return result == 'unknown' ? null : zclQuery.selectBitmapByName(db, packageId, type).then(rec => {
        return zclQuery.selectAtomicType(db, packageId, rec.type);
      });
    });

    const typeResolver = Promise.all([ defaultResolver, enumResolver, bitmapResolver ]);
    return typeResolver.then(types => (types.find(type => type)).size);
  }

  const promise = templateUtil.ensureZclPackageId(this).then(resolve.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise)
}

function asReadType(type)
{
  if (isShortString(type)) {
    return 'String';
  }

  if (isLongString(type)) {
    return 'LongString';
  }

  function resolve(packageId)
  {
    const options = { 'hash' : {} };
    return zclHelper.asUnderlyingZclType.call(this, type, options).then(zclType => {
      switch (zclType) {
      case 'int8_t':
      case 'uint8_t':
        return 'Int8u';
      case 'int16_t':
      case 'uint16_t':
        return 'Int16u';
      case 'int24_t':
      case 'uint24_t':
        return 'Int24u';
      case 'int32_t':
      case 'uint32_t':
        return 'Int32u';
      default:
        error = 'Unhandled underlying type ' + zclType + ' for original type ' + type;
        throw error;
      }
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(resolve.bind(this)).catch(err => console.log(err));
  return templateUtil.templatePromise(this.global, promise)
}

// WARNING! WARNING! WARNING! WARNING! WARNING! WARNING!
//
// Note: these exports are public API. Templates that might have been created in the past and are
// available in the wild might depend on these names.
// If you rename the functions, you need to still maintain old exports list.
exports.chip_header         = chip_header;
exports.isString            = isString;
exports.asReadType          = asReadType;
exports.asReadTypeLength    = asReadTypeLength;
exports.asValueIfNotPresent = asValueIfNotPresent;
