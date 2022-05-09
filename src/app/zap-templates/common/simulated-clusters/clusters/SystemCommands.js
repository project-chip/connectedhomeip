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
 * This file declares test suite utility methods for system commands.
 *
 * Each method declared in this file needs to be implemented on a per-language
 * basis and allows exposing  methods to the test suites that are not part
 * of the regular cluster set of APIs.
 *
 */

const Start = {
  name : 'Start',
  arguments : [
    { 'name' : 'discriminator', type : 'INT16U', isOptional : true }, { 'name' : 'port', type : 'INT16U', isOptional : true },
    { 'name' : 'kvs', type : 'CHAR_STRING', isOptional : true },
    { 'name' : 'minCommissioningTimeout', type : 'INT16U', isOptional : true },
    { 'name' : 'registerKey', type : 'CHAR_STRING', isOptional : true }
  ],
};

const Stop = {
  name : 'Stop',
  arguments : [ { 'name' : 'registerKey', type : 'CHAR_STRING', isOptional : true } ],
};

const Reboot = {
  name : 'Reboot',
  arguments : [ { 'name' : 'registerKey', type : 'CHAR_STRING', isOptional : true } ],
};

const FactoryReset = {
  name : 'FactoryReset',
  arguments : [ { 'name' : 'registerKey', type : 'CHAR_STRING', isOptional : true } ],
};

const SystemCommands = {
  name : 'SystemCommands',
  commands : [ Start, Stop, Reboot, FactoryReset ],
};

//
// Module exports
//
exports.cluster = SystemCommands;
