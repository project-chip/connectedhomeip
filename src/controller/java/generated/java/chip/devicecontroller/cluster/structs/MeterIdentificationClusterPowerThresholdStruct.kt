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

class MeterIdentificationClusterPowerThresholdStruct (
    val powerThreshold: Optional<Long>,
    val apparentPowerThreshold: Optional<Long>,
    val powerThresholdSource: UInt?) {
  override fun toString(): String  = buildString {
    append("MeterIdentificationClusterPowerThresholdStruct {\n")
    append("\tpowerThreshold : $powerThreshold\n")
    append("\tapparentPowerThreshold : $apparentPowerThreshold\n")
    append("\tpowerThresholdSource : $powerThresholdSource\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (powerThreshold.isPresent) {
      val optpowerThreshold = powerThreshold.get()
      put(ContextSpecificTag(TAG_POWER_THRESHOLD), optpowerThreshold)
    }
      if (apparentPowerThreshold.isPresent) {
      val optapparentPowerThreshold = apparentPowerThreshold.get()
      put(ContextSpecificTag(TAG_APPARENT_POWER_THRESHOLD), optapparentPowerThreshold)
    }
      if (powerThresholdSource != null) {
      put(ContextSpecificTag(TAG_POWER_THRESHOLD_SOURCE), powerThresholdSource)
    } else {
      putNull(ContextSpecificTag(TAG_POWER_THRESHOLD_SOURCE))
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_POWER_THRESHOLD = 0
    private const val TAG_APPARENT_POWER_THRESHOLD = 1
    private const val TAG_POWER_THRESHOLD_SOURCE = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : MeterIdentificationClusterPowerThresholdStruct {
      tlvReader.enterStructure(tlvTag)
      val powerThreshold = if (tlvReader.isNextTag(ContextSpecificTag(TAG_POWER_THRESHOLD))) {
      Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_POWER_THRESHOLD)))
    } else {
      Optional.empty()
    }
      val apparentPowerThreshold = if (tlvReader.isNextTag(ContextSpecificTag(TAG_APPARENT_POWER_THRESHOLD))) {
      Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_APPARENT_POWER_THRESHOLD)))
    } else {
      Optional.empty()
    }
      val powerThresholdSource = if (!tlvReader.isNull()) {
      tlvReader.getUInt(ContextSpecificTag(TAG_POWER_THRESHOLD_SOURCE))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_POWER_THRESHOLD_SOURCE))
      null
    }
      
      tlvReader.exitContainer()

      return MeterIdentificationClusterPowerThresholdStruct(powerThreshold, apparentPowerThreshold, powerThresholdSource)
    }
  }
}
