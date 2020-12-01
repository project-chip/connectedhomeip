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

// Import Zcl helper from zap core
const helperZcl = require('../../../third_party/zap/repo/src-electron/generator/helper-zcl.js')

/**
 * Dummy helper that add a string to the templates showing
 * if the strings matches. Use to demonstrate the use
 * of ZAP helper within the chip-helper environment
 *
 * @param {*} str1 : First string to compare
 * @param {*} str2 : Second string to comapre
 */
function example_helper(str1, str2) {
  if (helperZcl.isStrEqual(str1, str2)) {
    return 'The two strings are identical'
  } else {
    return 'The two strings are different'
  }
}

/**
 * Produces the top-of-the-file header for a C file.
 *
 * @returns The header content
 */
function chip_header() {
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

// WARNING! WARNING! WARNING! WARNING! WARNING! WARNING!
//
// Note: these exports are public API. Templates that might have been created in the past and are
// available in the wild might depend on these names.
// If you rename the functions, you need to still maintain old exports list.
exports.chip_header = chip_header;
exports.example_helper = example_helper;
