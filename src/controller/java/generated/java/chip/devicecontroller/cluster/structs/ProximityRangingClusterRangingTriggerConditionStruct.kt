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

class ProximityRangingClusterRangingTriggerConditionStruct(
  val startTime: ULong,
  val endTime: ULong,
  val rangingInstanceInterval: Optional<ULong>,
) {
  override fun toString(): String = buildString {
    append("ProximityRangingClusterRangingTriggerConditionStruct {\n")
    append("\tstartTime : $startTime\n")
    append("\tendTime : $endTime\n")
    append("\trangingInstanceInterval : $rangingInstanceInterval\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_START_TIME), startTime)
      put(ContextSpecificTag(TAG_END_TIME), endTime)
      if (rangingInstanceInterval.isPresent) {
        val optrangingInstanceInterval = rangingInstanceInterval.get()
        put(ContextSpecificTag(TAG_RANGING_INSTANCE_INTERVAL), optrangingInstanceInterval)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_START_TIME = 0
    private const val TAG_END_TIME = 1
    private const val TAG_RANGING_INSTANCE_INTERVAL = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ProximityRangingClusterRangingTriggerConditionStruct {
      tlvReader.enterStructure(tlvTag)
      val startTime = tlvReader.getULong(ContextSpecificTag(TAG_START_TIME))
      val endTime = tlvReader.getULong(ContextSpecificTag(TAG_END_TIME))
      val rangingInstanceInterval =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_RANGING_INSTANCE_INTERVAL))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_RANGING_INSTANCE_INTERVAL)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ProximityRangingClusterRangingTriggerConditionStruct(
        startTime,
        endTime,
        rangingInstanceInterval,
      )
    }
  }
}
