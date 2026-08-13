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

class ThermostatClusterRunningModeChangeEvent(
  val previousRunningMode: Optional<UInt>,
  val currentRunningMode: UInt,
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterRunningModeChangeEvent {\n")
    append("\tpreviousRunningMode : $previousRunningMode\n")
    append("\tcurrentRunningMode : $currentRunningMode\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (previousRunningMode.isPresent) {
        val optpreviousRunningMode = previousRunningMode.get()
        put(ContextSpecificTag(TAG_PREVIOUS_RUNNING_MODE), optpreviousRunningMode)
      }
      put(ContextSpecificTag(TAG_CURRENT_RUNNING_MODE), currentRunningMode)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PREVIOUS_RUNNING_MODE = 0
    private const val TAG_CURRENT_RUNNING_MODE = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterRunningModeChangeEvent {
      tlvReader.enterStructure(tlvTag)
      val previousRunningMode =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PREVIOUS_RUNNING_MODE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_PREVIOUS_RUNNING_MODE)))
        } else {
          Optional.empty()
        }
      val currentRunningMode = tlvReader.getUInt(ContextSpecificTag(TAG_CURRENT_RUNNING_MODE))

      tlvReader.exitContainer()

      return ThermostatClusterRunningModeChangeEvent(previousRunningMode, currentRunningMode)
    }
  }
}
