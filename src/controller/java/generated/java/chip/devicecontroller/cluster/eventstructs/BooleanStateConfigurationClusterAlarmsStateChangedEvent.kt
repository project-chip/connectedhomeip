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

class BooleanStateConfigurationClusterAlarmsStateChangedEvent(
  val alarmsActive: UInt,
  val alarmsSuppressed: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("BooleanStateConfigurationClusterAlarmsStateChangedEvent {\n")
    append("\talarmsActive : $alarmsActive\n")
    append("\talarmsSuppressed : $alarmsSuppressed\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ALARMS_ACTIVE), alarmsActive)
      if (alarmsSuppressed.isPresent) {
        val optalarmsSuppressed = alarmsSuppressed.get()
        put(ContextSpecificTag(TAG_ALARMS_SUPPRESSED), optalarmsSuppressed)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ALARMS_ACTIVE = 0
    private const val TAG_ALARMS_SUPPRESSED = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): BooleanStateConfigurationClusterAlarmsStateChangedEvent {
      tlvReader.enterStructure(tlvTag)
      val alarmsActive = tlvReader.getUInt(ContextSpecificTag(TAG_ALARMS_ACTIVE))
      val alarmsSuppressed =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ALARMS_SUPPRESSED))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_ALARMS_SUPPRESSED)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return BooleanStateConfigurationClusterAlarmsStateChangedEvent(alarmsActive, alarmsSuppressed)
    }
  }
}
