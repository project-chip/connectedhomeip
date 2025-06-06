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

class ZoneManagementClusterZoneTriggerControlStruct(
  val zoneID: UInt,
  val initialDuration: ULong,
  val augmentationDuration: ULong,
  val maxDuration: ULong,
  val blindDuration: ULong,
  val sensitivity: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("ZoneManagementClusterZoneTriggerControlStruct {\n")
    append("\tzoneID : $zoneID\n")
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
      put(ContextSpecificTag(TAG_ZONE_ID), zoneID)
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
    private const val TAG_ZONE_ID = 0
    private const val TAG_INITIAL_DURATION = 1
    private const val TAG_AUGMENTATION_DURATION = 2
    private const val TAG_MAX_DURATION = 3
    private const val TAG_BLIND_DURATION = 4
    private const val TAG_SENSITIVITY = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ZoneManagementClusterZoneTriggerControlStruct {
      tlvReader.enterStructure(tlvTag)
      val zoneID = tlvReader.getUInt(ContextSpecificTag(TAG_ZONE_ID))
      val initialDuration = tlvReader.getULong(ContextSpecificTag(TAG_INITIAL_DURATION))
      val augmentationDuration = tlvReader.getULong(ContextSpecificTag(TAG_AUGMENTATION_DURATION))
      val maxDuration = tlvReader.getULong(ContextSpecificTag(TAG_MAX_DURATION))
      val blindDuration = tlvReader.getULong(ContextSpecificTag(TAG_BLIND_DURATION))
      val sensitivity =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SENSITIVITY))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_SENSITIVITY)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ZoneManagementClusterZoneTriggerControlStruct(
        zoneID,
        initialDuration,
        augmentationDuration,
        maxDuration,
        blindDuration,
        sensitivity,
      )
    }
  }
}
