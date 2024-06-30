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

class OvenCavityOperationalStateClusterOperationalStateStruct(
  val operationalStateID: UInt,
  val operationalStateLabel: Optional<String>
) {
  override fun toString(): String = buildString {
    append("OvenCavityOperationalStateClusterOperationalStateStruct {\n")
    append("\toperationalStateID : $operationalStateID\n")
    append("\toperationalStateLabel : $operationalStateLabel\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_OPERATIONAL_STATE_I_D), operationalStateID)
      if (operationalStateLabel.isPresent) {
        val optoperationalStateLabel = operationalStateLabel.get()
        put(ContextSpecificTag(TAG_OPERATIONAL_STATE_LABEL), optoperationalStateLabel)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_OPERATIONAL_STATE_I_D = 0
    private const val TAG_OPERATIONAL_STATE_LABEL = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): OvenCavityOperationalStateClusterOperationalStateStruct {
      tlvReader.enterStructure(tlvTag)
      val operationalStateID = tlvReader.getUInt(ContextSpecificTag(TAG_OPERATIONAL_STATE_I_D))
      val operationalStateLabel =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_OPERATIONAL_STATE_LABEL))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_OPERATIONAL_STATE_LABEL)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return OvenCavityOperationalStateClusterOperationalStateStruct(
        operationalStateID,
        operationalStateLabel
      )
    }
  }
}
