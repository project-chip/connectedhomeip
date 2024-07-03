/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

package matter.controller

object MatterICDClientImpl {
  fun isPeerICDClient(fabricIndex: Int, deviceId: Long): Boolean {
    val clientInfo = getICDClientInfo(fabricIndex) ?: return false
    return clientInfo.firstOrNull { it.peerNodeId == deviceId } != null
  }

  external fun storeICDEntryWithKey(fabricIndex: Int, icdClientInfo: ICDClientInfo, key: ByteArray)

  external fun removeICDEntryWithKey(fabricIndex: Int, icdClientInfo: ICDClientInfo)

  external fun clearICDClientInfo(fabricIndex: Int, deviceId: Long)

  external fun getICDClientInfo(fabricIndex: Int): List<ICDClientInfo>?
}
