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

function isClient(side) {
    return 0 == side.localeCompare("client");
}

function isServer(side) {
    return 0 == side.localeCompare("server");
}

function isStrEqual(str1, str2) {
    return 0 == str1.localeCompare(str2);
}

function isLastElement(index, count) {
    return index == count - 1;
}

function isEnabled(enable) {
    return 1 == enable;
}

function isCommandAvailable(clusterSide, incoming, outgoing, source, name) {
    if (0 == clusterSide.localeCompare(source)) {
        return false;
    }

    if (isClient(clusterSide) && outgoing) {
        return true;
    } else if (isServer(clusterSide) && incoming) {
        return true;
    }
    return false;
}

// WARNING! WARNING! WARNING! WARNING! WARNING! WARNING!
//
// Note: these exports are public API. Templates that might have been created in the past and are
// available in the wild might depend on these names.
// If you rename the functions, you need to still maintain old exports list.
exports.chip_header = chip_header;
exports.isClient = isClient;
exports.isServer = isServer;
exports.isStrEqual = isStrEqual;
exports.isLastElement = isLastElement;
exports.isEnabled = isEnabled;
exports.isCommandAvailable = isCommandAvailable;
