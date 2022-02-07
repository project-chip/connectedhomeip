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

function pad(label, len, ch = ' ')
{
  return label.padEnd(len, ch);
}

function concat()
{
  let str = ''
  // Last argument is our hash argument.
  for (let arg = 0; arg < arguments.length - 1; ++arg)
  {
    if (typeof arguments[arg] != 'object' || arguments[arg] instanceof String) {
      str += arguments[arg];
    } else {
      throw new TypeError(
          `Unexpected object in concat: ${arg}:${Object.prototype.toString.call(arguments[arg])}, ${JSON.stringify(arguments)}`);
    }
  }
  return str
}

//
// Module exports
//
exports.pad    = pad;
exports.concat = concat;
