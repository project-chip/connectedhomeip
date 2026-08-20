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

class ProximityRangingClusterRangingResultEvent(
  val sessionID: UByte,
  val rangingResultData:
    matter.controller.cluster.structs.ProximityRangingClusterRangingMeasurementDataStruct,
) {
  override fun toString(): String = buildString {
    append("ProximityRangingClusterRangingResultEvent {\n")
    append("\tsessionID : $sessionID\n")
    append("\trangingResultData : $rangingResultData\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SESSION_ID), sessionID)
      rangingResultData.toTlv(ContextSpecificTag(TAG_RANGING_RESULT_DATA), this)
      endStructure()
    }
  }

  companion object {
    private const val TAG_SESSION_ID = 0
    private const val TAG_RANGING_RESULT_DATA = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ProximityRangingClusterRangingResultEvent {
      tlvReader.enterStructure(tlvTag)
      val sessionID = tlvReader.getUByte(ContextSpecificTag(TAG_SESSION_ID))
      val rangingResultData =
        matter.controller.cluster.structs.ProximityRangingClusterRangingMeasurementDataStruct
          .fromTlv(ContextSpecificTag(TAG_RANGING_RESULT_DATA), tlvReader)

      tlvReader.exitContainer()

      return ProximityRangingClusterRangingResultEvent(sessionID, rangingResultData)
    }
  }
}
