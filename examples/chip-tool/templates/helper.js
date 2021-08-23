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
const templateUtil = require(zapPath + 'generator/template-util.js')
const zclHelper    = require(zapPath + 'generator/helper-zcl.js')

const ChipTypesHelper = require('../../../src/app/zap-templates/common/ChipTypesHelper.js');

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
      case 'bool':
        return '0';
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
        error = 'asTypeMinValue: Unhandled underlying type ' + zclType + ' for original type ' + type;
        throw error;
      }
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => {
    console.log(err);
    throw err;
  });
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
      case 'bool':
        return '1';
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
        error = 'asTypeMaxValue: Unhandled underlying type ' + zclType + ' for original type ' + type;
        throw error;
      }
    })
  }

  const promise = templateUtil.ensureZclPackageId(this).then(fn.bind(this)).catch(err => {
    console.log(err);
    throw err;
  });
  return templateUtil.templatePromise(this.global, promise);
}

//
// Module exports
//
exports.asDelimitedCommand = asDelimitedCommand;
exports.asTypeMinValue     = asTypeMinValue;
exports.asTypeMaxValue     = asTypeMaxValue;
