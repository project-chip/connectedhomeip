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
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct(
  val nodeID: ULong,
  val endpoint: UInt,
) {
  override fun toString(): String = buildString {
    append("TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct {\n")
    append("\tnodeID : $nodeID\n")
    append("\tendpoint : $endpoint\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NODE_ID), nodeID)
      put(ContextSpecificTag(TAG_ENDPOINT), endpoint)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NODE_ID = 0
    private const val TAG_ENDPOINT = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct {
      tlvReader.enterStructure(tlvTag)
      val nodeID = tlvReader.getULong(ContextSpecificTag(TAG_NODE_ID))
      val endpoint = tlvReader.getUInt(ContextSpecificTag(TAG_ENDPOINT))

      tlvReader.exitContainer()

      return TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct(nodeID, endpoint)
    }
  }
}
