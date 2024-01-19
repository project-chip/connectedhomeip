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

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ValveConfigurationAndControlClusterValveStateChangedEvent(
  val valveState: UByte,
  val valveLevel: Optional<UByte>
) {
  override fun toString(): String = buildString {
    append("ValveConfigurationAndControlClusterValveStateChangedEvent {\n")
    append("\tvalveState : $valveState\n")
    append("\tvalveLevel : $valveLevel\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_VALVE_STATE), valveState)
      if (valveLevel.isPresent) {
        val optvalveLevel = valveLevel.get()
        put(ContextSpecificTag(TAG_VALVE_LEVEL), optvalveLevel)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_VALVE_STATE = 0
    private const val TAG_VALVE_LEVEL = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): ValveConfigurationAndControlClusterValveStateChangedEvent {
      tlvReader.enterStructure(tlvTag)
      val valveState = tlvReader.getUByte(ContextSpecificTag(TAG_VALVE_STATE))
      val valveLevel =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_VALVE_LEVEL))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_VALVE_LEVEL)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ValveConfigurationAndControlClusterValveStateChangedEvent(valveState, valveLevel)
    }
  }
}
