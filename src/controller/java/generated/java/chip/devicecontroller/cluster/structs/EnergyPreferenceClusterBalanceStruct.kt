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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class EnergyPreferenceClusterBalanceStruct (
    val step: UInt,
    val label: Optional<String>) {
  override fun toString(): String  = buildString {
    append("EnergyPreferenceClusterBalanceStruct {\n")
    append("\tstep : $step\n")
    append("\tlabel : $label\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_STEP), step)
      if (label.isPresent) {
      val optlabel = label.get()
      put(ContextSpecificTag(TAG_LABEL), optlabel)
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_STEP = 0
    private const val TAG_LABEL = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : EnergyPreferenceClusterBalanceStruct {
      tlvReader.enterStructure(tlvTag)
      val step = tlvReader.getUInt(ContextSpecificTag(TAG_STEP))
      val label = if (tlvReader.isNextTag(ContextSpecificTag(TAG_LABEL))) {
      Optional.of(tlvReader.getString(ContextSpecificTag(TAG_LABEL)))
    } else {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return EnergyPreferenceClusterBalanceStruct(step, label)
    }
  }
}
