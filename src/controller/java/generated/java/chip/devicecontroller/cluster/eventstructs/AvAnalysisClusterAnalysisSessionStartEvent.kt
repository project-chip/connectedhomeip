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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AvAnalysisClusterAnalysisSessionStartEvent(
  val sessionID: UInt,
  val sourceNodeId: Optional<ULong>,
  val triggeredZones: List<UInt>?,
) {
  override fun toString(): String = buildString {
    append("AvAnalysisClusterAnalysisSessionStartEvent {\n")
    append("\tsessionID : $sessionID\n")
    append("\tsourceNodeId : $sourceNodeId\n")
    append("\ttriggeredZones : $triggeredZones\n")
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
      if (triggeredZones != null) {
        startArray(ContextSpecificTag(TAG_TRIGGERED_ZONES))
        for (item in triggeredZones.iterator()) {
          put(AnonymousTag, item)
        }
        endArray()
      } else {
        putNull(ContextSpecificTag(TAG_TRIGGERED_ZONES))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_SESSION_ID = 0
    private const val TAG_SOURCE_NODE_ID = 1
    private const val TAG_TRIGGERED_ZONES = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): AvAnalysisClusterAnalysisSessionStartEvent {
      tlvReader.enterStructure(tlvTag)
      val sessionID = tlvReader.getUInt(ContextSpecificTag(TAG_SESSION_ID))
      val sourceNodeId =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SOURCE_NODE_ID))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_SOURCE_NODE_ID)))
        } else {
          Optional.empty()
        }
      val triggeredZones =
        if (!tlvReader.isNull()) {
          buildList<UInt> {
            tlvReader.enterArray(ContextSpecificTag(TAG_TRIGGERED_ZONES))
            while (!tlvReader.isEndOfContainer()) {
              this.add(tlvReader.getUInt(AnonymousTag))
            }
            tlvReader.exitContainer()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TRIGGERED_ZONES))
          null
        }

      tlvReader.exitContainer()

      return AvAnalysisClusterAnalysisSessionStartEvent(sessionID, sourceNodeId, triggeredZones)
    }
  }
}
