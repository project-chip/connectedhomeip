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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AvAnalysisClusterAnalysisSessionEndEvent(
  val sessionID: UInt,
  val sourceNodeId: Optional<ULong>,
) {
  override fun toString(): String = buildString {
    append("AvAnalysisClusterAnalysisSessionEndEvent {\n")
    append("\tsessionID : $sessionID\n")
    append("\tsourceNodeId : $sourceNodeId\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SESSION_ID), sessionID)
      if (sourceNodeId.isPresent) {
        val optsourceNodeId = sourceNodeId.get()
        put(ContextSpecificTag(TAG_SOURCE_NODE_ID), optsourceNodeId)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_SESSION_ID = 0
    private const val TAG_SOURCE_NODE_ID = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): AvAnalysisClusterAnalysisSessionEndEvent {
      tlvReader.enterStructure(tlvTag)
      val sessionID = tlvReader.getUInt(ContextSpecificTag(TAG_SESSION_ID))
      val sourceNodeId =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SOURCE_NODE_ID))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_SOURCE_NODE_ID)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return AvAnalysisClusterAnalysisSessionEndEvent(sessionID, sourceNodeId)
    }
  }
}
