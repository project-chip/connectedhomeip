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

class ValveConfigurationAndControlClusterValveFaultEvent(val valveFault: UShort) {
  override fun toString(): String = buildString {
    append("ValveConfigurationAndControlClusterValveFaultEvent {\n")
    append("\tvalveFault : $valveFault\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_VALVE_FAULT), valveFault)
      endStructure()
    }
  }

  companion object {
    private const val TAG_VALVE_FAULT = 0

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): ValveConfigurationAndControlClusterValveFaultEvent {
      tlvReader.enterStructure(tlvTag)
      val valveFault = tlvReader.getUShort(ContextSpecificTag(TAG_VALVE_FAULT))

      tlvReader.exitContainer()

      return ValveConfigurationAndControlClusterValveFaultEvent(valveFault)
    }
  }
}
