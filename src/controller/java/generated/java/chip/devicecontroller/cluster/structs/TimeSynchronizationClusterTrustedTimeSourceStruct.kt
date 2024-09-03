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

class TimeSynchronizationClusterTrustedTimeSourceStruct(
  val fabricIndex: UInt,
  val nodeID: ULong,
  val endpoint: UInt,
) {
  override fun toString(): String = buildString {
    append("TimeSynchronizationClusterTrustedTimeSourceStruct {\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("\tnodeID : $nodeID\n")
    append("\tendpoint : $endpoint\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      put(ContextSpecificTag(TAG_NODE_ID), nodeID)
      put(ContextSpecificTag(TAG_ENDPOINT), endpoint)
      endStructure()
    }
  }

  companion object {
    private const val TAG_FABRIC_INDEX = 0
    private const val TAG_NODE_ID = 1
    private const val TAG_ENDPOINT = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): TimeSynchronizationClusterTrustedTimeSourceStruct {
      tlvReader.enterStructure(tlvTag)
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))
      val nodeID = tlvReader.getULong(ContextSpecificTag(TAG_NODE_ID))
      val endpoint = tlvReader.getUInt(ContextSpecificTag(TAG_ENDPOINT))

      tlvReader.exitContainer()

      return TimeSynchronizationClusterTrustedTimeSourceStruct(fabricIndex, nodeID, endpoint)
    }
  }
}
