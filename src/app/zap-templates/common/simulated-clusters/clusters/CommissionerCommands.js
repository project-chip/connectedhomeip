/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/*
 * This file declare test suites utilities methods for commissioner commands.
 *
 * Each method declared in this file needs to be implemented on a per-language
 * basis and permits to exposes methods to the test suites that are not part
 * of the regular cluster set of APIs.
 *
 */

const PairWithQRCode = {
  name : 'PairWithQRCode',
  arguments : [ { type : 'NODE_ID', name : 'nodeId' }, { type : 'CHAR_STRING', name : 'payload' } ],
  response : { arguments : [] }
};

const PairWithManualCode = {
  name : 'PairWithManualCode',
  arguments : [ { type : 'NODE_ID', name : 'nodeId' }, { type : 'CHAR_STRING', name : 'payload' } ],
  response : { arguments : [] }
};

const Unpair = {
  name : 'Unpair',
  arguments : [ { type : 'NODE_ID', name : 'nodeId' } ],
  response : { arguments : [] }
};

const name     = 'CommissionerCommands';
const commands = [ PairWithQRCode, PairWithManualCode, Unpair ];

const CommissionerCommands = {
  name,
  commands
};

//
// Module exports
//
exports.cluster = CommissionerCommands;
