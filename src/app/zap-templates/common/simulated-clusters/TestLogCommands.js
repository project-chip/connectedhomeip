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
 * This file declares test suite utility methods for logging.
 *
 * Each method declared in this file needs to be implemented on a per-language
 * basis and allows exposing  methods to the test suites that are not part
 * of the regular cluster set of APIs.
 *
 */

const Log = {
  name : 'Log',
  arguments : [ { type : 'CHAR_STRING', name : 'message' } ],
  response : { arguments : [] }
};

const LogCommands = {
  name : 'LogCommands',
  commands : [ Log ],
};

//
// Module exports
//
exports.LogCommands = LogCommands;
