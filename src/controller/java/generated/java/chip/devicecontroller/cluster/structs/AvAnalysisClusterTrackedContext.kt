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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AvAnalysisClusterTrackedContext(
  val identifiedContextID: UInt,
  val identifiedContext: AvAnalysisClusterSemanticTagStruct,
  val previousZone: Optional<UInt>?,
  val currentZone: Optional<UInt>?,
  val startTime: ULong,
  val endTime: ULong?,
) {
  override fun toString(): String = buildString {
    append("AvAnalysisClusterTrackedContext {\n")
    append("\tidentifiedContextID : $identifiedContextID\n")
    append("\tidentifiedContext : $identifiedContext\n")
    append("\tpreviousZone : $previousZone\n")
    append("\tcurrentZone : $currentZone\n")
    append("\tstartTime : $startTime\n")
    append("\tendTime : $endTime\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_IDENTIFIED_CONTEXT_ID), identifiedContextID)
      identifiedContext.toTlv(ContextSpecificTag(TAG_IDENTIFIED_CONTEXT), this)
      if (previousZone != null) {
        if (previousZone.isPresent) {
          val optpreviousZone = previousZone.get()
          put(ContextSpecificTag(TAG_PREVIOUS_ZONE), optpreviousZone)
        }
      } else {
        putNull(ContextSpecificTag(TAG_PREVIOUS_ZONE))
      }
      if (currentZone != null) {
        if (currentZone.isPresent) {
          val optcurrentZone = currentZone.get()
          put(ContextSpecificTag(TAG_CURRENT_ZONE), optcurrentZone)
        }
      } else {
        putNull(ContextSpecificTag(TAG_CURRENT_ZONE))
      }
      put(ContextSpecificTag(TAG_START_TIME), startTime)
      if (endTime != null) {
        put(ContextSpecificTag(TAG_END_TIME), endTime)
      } else {
        putNull(ContextSpecificTag(TAG_END_TIME))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_IDENTIFIED_CONTEXT_ID = 0
    private const val TAG_IDENTIFIED_CONTEXT = 1
    private const val TAG_PREVIOUS_ZONE = 2
    private const val TAG_CURRENT_ZONE = 3
    private const val TAG_START_TIME = 4
    private const val TAG_END_TIME = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): AvAnalysisClusterTrackedContext {
      tlvReader.enterStructure(tlvTag)
      val identifiedContextID = tlvReader.getUInt(ContextSpecificTag(TAG_IDENTIFIED_CONTEXT_ID))
      val identifiedContext =
        AvAnalysisClusterSemanticTagStruct.fromTlv(
          ContextSpecificTag(TAG_IDENTIFIED_CONTEXT),
          tlvReader,
        )
      val previousZone =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_PREVIOUS_ZONE))) {
            Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_PREVIOUS_ZONE)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_PREVIOUS_ZONE))
          null
        }
      val currentZone =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_CURRENT_ZONE))) {
            Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_CURRENT_ZONE)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CURRENT_ZONE))
          null
        }
      val startTime = tlvReader.getULong(ContextSpecificTag(TAG_START_TIME))
      val endTime =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_END_TIME))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_END_TIME))
          null
        }

      tlvReader.exitContainer()

      return AvAnalysisClusterTrackedContext(
        identifiedContextID,
        identifiedContext,
        previousZone,
        currentZone,
        startTime,
        endTime,
      )
    }
  }
}
