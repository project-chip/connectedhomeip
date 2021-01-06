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

//
// Module exports
//
exports.isString      = isString;
exports.isShortString = isShortString;
exports.isLongString  = isLongString;
