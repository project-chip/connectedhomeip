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

class ZoneManagementClusterZoneTriggerControlStruct (
    val initialDuration: UInt,
    val augmentationDuration: UInt,
    val maxDuration: ULong,
    val blindDuration: UInt,
    val sensitivity: Optional<UInt>) {
  override fun toString(): String  = buildString {
    append("ZoneManagementClusterZoneTriggerControlStruct {\n")
    append("\tinitialDuration : $initialDuration\n")
    append("\taugmentationDuration : $augmentationDuration\n")
    append("\tmaxDuration : $maxDuration\n")
    append("\tblindDuration : $blindDuration\n")
    append("\tsensitivity : $sensitivity\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_INITIAL_DURATION), initialDuration)
      put(ContextSpecificTag(TAG_AUGMENTATION_DURATION), augmentationDuration)
      put(ContextSpecificTag(TAG_MAX_DURATION), maxDuration)
      put(ContextSpecificTag(TAG_BLIND_DURATION), blindDuration)
      if (sensitivity.isPresent) {
      val optsensitivity = sensitivity.get()
      put(ContextSpecificTag(TAG_SENSITIVITY), optsensitivity)
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_INITIAL_DURATION = 0
    private const val TAG_AUGMENTATION_DURATION = 1
    private const val TAG_MAX_DURATION = 2
    private const val TAG_BLIND_DURATION = 3
    private const val TAG_SENSITIVITY = 4

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ZoneManagementClusterZoneTriggerControlStruct {
      tlvReader.enterStructure(tlvTag)
      val initialDuration = tlvReader.getUInt(ContextSpecificTag(TAG_INITIAL_DURATION))
      val augmentationDuration = tlvReader.getUInt(ContextSpecificTag(TAG_AUGMENTATION_DURATION))
      val maxDuration = tlvReader.getULong(ContextSpecificTag(TAG_MAX_DURATION))
      val blindDuration = tlvReader.getUInt(ContextSpecificTag(TAG_BLIND_DURATION))
      val sensitivity = if (tlvReader.isNextTag(ContextSpecificTag(TAG_SENSITIVITY))) {
      Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_SENSITIVITY)))
    } else {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return ZoneManagementClusterZoneTriggerControlStruct(initialDuration, augmentationDuration, maxDuration, blindDuration, sensitivity)
    }
  }
}
