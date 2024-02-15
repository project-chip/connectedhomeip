/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
package matter.controller.cluster.eventstructs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class BridgedDeviceBasicInformationClusterReachableChangedEvent(val reachableNewValue: Boolean) {
  override fun toString(): String = buildString {
    append("BridgedDeviceBasicInformationClusterReachableChangedEvent {\n")
    append("\treachableNewValue : $reachableNewValue\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_REACHABLE_NEW_VALUE), reachableNewValue)
      endStructure()
    }
  }

  companion object {
    private const val TAG_REACHABLE_NEW_VALUE = 0

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): BridgedDeviceBasicInformationClusterReachableChangedEvent {
      tlvReader.enterStructure(tlvTag)
      val reachableNewValue = tlvReader.getBoolean(ContextSpecificTag(TAG_REACHABLE_NEW_VALUE))

      tlvReader.exitContainer()

      return BridgedDeviceBasicInformationClusterReachableChangedEvent(reachableNewValue)
    }
  }
}
