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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class ElectricalGridConditionsClusterCurrentConditionsChangedEvent (
    val currentConditions: chip.devicecontroller.cluster.structs.ElectricalGridConditionsClusterElectricalGridConditionsStruct?) {
  override fun toString(): String  = buildString {
    append("ElectricalGridConditionsClusterCurrentConditionsChangedEvent {\n")
    append("\tcurrentConditions : $currentConditions\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (currentConditions != null) {
      currentConditions.toTlv(ContextSpecificTag(TAG_CURRENT_CONDITIONS), this)
    } else {
      putNull(ContextSpecificTag(TAG_CURRENT_CONDITIONS))
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CURRENT_CONDITIONS = 0

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ElectricalGridConditionsClusterCurrentConditionsChangedEvent {
      tlvReader.enterStructure(tlvTag)
      val currentConditions = if (!tlvReader.isNull()) {
      chip.devicecontroller.cluster.structs.ElectricalGridConditionsClusterElectricalGridConditionsStruct.fromTlv(ContextSpecificTag(TAG_CURRENT_CONDITIONS), tlvReader)
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_CURRENT_CONDITIONS))
      null
    }
      
      tlvReader.exitContainer()

      return ElectricalGridConditionsClusterCurrentConditionsChangedEvent(currentConditions)
    }
  }
}
