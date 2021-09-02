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

const ListHelper   = require('../../common/ListHelper.js');
const StringHelper = require('../../common/StringHelper.js');
const StructHelper = require('../../common/StructHelper.js');

// Issue #8202
// The specification allow non-standard signed and unsigned integer with a width of 24, 40, 48 or 56, but those types does not have
// proper support yet into the codebase and the resulting generated code can not be built with them.
// Once they are supported, the following method could be removed.
const unsupportedTypes = [ 'INT24S', 'INT40S', 'INT48S', 'INT56S', 'INT24U', 'INT40U', 'INT48U', 'INT56U', 'EUI64' ];
function isUnsupportedType(type)
{
  return unsupportedTypes.includes(type.toUpperCase());
}

function canHaveSimpleAccessors(type)
{
  if (StringHelper.isString(type)) {
    return false;
  }

  if (ListHelper.isList(type)) {
    return false;
  }

  if (StructHelper.isStruct(type)) {
    return false;
  }

  if (isUnsupportedType(type)) {
    return false;
  }

  return true;
}

//
// Module exports
//
exports.canHaveSimpleAccessors = canHaveSimpleAccessors;
