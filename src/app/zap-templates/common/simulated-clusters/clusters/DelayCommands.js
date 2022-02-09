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

/*
 * This file declare test suites utilities methods for delayed commands.
 *
 * Each method declared in this file needs to be implemented on a per-language
 * basis and permits to exposes methods to the test suites that are not part
 * of the regular cluster set of APIs.
 *
 */

const WaitForMs = {
  name : 'WaitForMs',
  arguments : [ { type : 'INT32U', name : 'ms' } ],
  response : { arguments : [] }
};

const WaitForCommissioning = {
  name : 'WaitForCommissioning',
  arguments : [],
  response : { arguments : [] }
};

const WaitForCommissionee = {
  name : 'WaitForCommissionee',
  arguments : [],
  response : { arguments : [] }
};

const WaitForCommissionableAdvertisement = {
  name : 'WaitForCommissionableAdvertisement',
  arguments : [],
  response : { arguments : [] }
};

const WaitForOperationalAdvertisement = {
  name : 'WaitForOperationalAdvertisement',
  arguments : [],
  response : { arguments : [] }
};

const name = 'DelayCommands';
const commands =
    [ WaitForMs, WaitForCommissioning, WaitForCommissionee, WaitForCommissionableAdvertisement, WaitForOperationalAdvertisement ];

const DelayCommands = {
  name,
  commands
};

//
// Module exports
//
exports.cluster = DelayCommands;
