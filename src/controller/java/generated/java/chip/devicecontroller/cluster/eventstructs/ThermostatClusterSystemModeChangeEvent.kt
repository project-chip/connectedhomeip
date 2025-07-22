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

class ThermostatClusterSystemModeChangeEvent(
  val previousSystemMode: Optional<UInt>,
  val currentSystemMode: UInt,
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterSystemModeChangeEvent {\n")
    append("\tpreviousSystemMode : $previousSystemMode\n")
    append("\tcurrentSystemMode : $currentSystemMode\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (previousSystemMode.isPresent) {
        val optpreviousSystemMode = previousSystemMode.get()
        put(ContextSpecificTag(TAG_PREVIOUS_SYSTEM_MODE), optpreviousSystemMode)
      }
      put(ContextSpecificTag(TAG_CURRENT_SYSTEM_MODE), currentSystemMode)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PREVIOUS_SYSTEM_MODE = 0
    private const val TAG_CURRENT_SYSTEM_MODE = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterSystemModeChangeEvent {
      tlvReader.enterStructure(tlvTag)
      val previousSystemMode =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PREVIOUS_SYSTEM_MODE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_PREVIOUS_SYSTEM_MODE)))
        } else {
          Optional.empty()
        }
      val currentSystemMode = tlvReader.getUInt(ContextSpecificTag(TAG_CURRENT_SYSTEM_MODE))

      tlvReader.exitContainer()

      return ThermostatClusterSystemModeChangeEvent(previousSystemMode, currentSystemMode)
    }
  }
}
