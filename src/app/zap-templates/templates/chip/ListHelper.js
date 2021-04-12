/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

const kListAttributeAtomicTypeId = 0x48;

/**
 * Returns the calculated length of the given attribute list
 *
 * This function is meant to be used inside a {{#chip_server_cluster_attributes}} block.
 * It will throws otherwise.
 *
 * @param {*} options
 */
function asListEntryLength(options)
{
  return this.size;
  if (this.atomicTypeId == undefined) {
    const error = 'asListEntryLength: missing atomic type.';
    console.log(error);
    throw error;
  }

  if (this.atomicTypeId != kListAttributeAtomicTypeId) {
    const error = 'asListEntryLength: Not a list.';
    console.log(error);
    throw error;
  }

  const entryType = this.entryType;
  return entryType.size || entryType.map(type => type.size).reduce((accumulator, currentValue) => accumulator + currentValue, 0);
}

//
// Module exports
//
exports.asListEntryLength = asListEntryLength;
