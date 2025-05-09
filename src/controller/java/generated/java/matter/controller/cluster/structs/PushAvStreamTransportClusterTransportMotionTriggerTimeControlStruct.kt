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
package matter.controller.cluster.structs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct(
  val initialDuration: UShort,
  val augmentationDuration: UShort,
  val maxDuration: UInt,
  val blindDuration: UShort,
) {
  override fun toString(): String = buildString {
    append("PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct {\n")
    append("\tinitialDuration : $initialDuration\n")
    append("\taugmentationDuration : $augmentationDuration\n")
    append("\tmaxDuration : $maxDuration\n")
    append("\tblindDuration : $blindDuration\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_INITIAL_DURATION), initialDuration)
      put(ContextSpecificTag(TAG_AUGMENTATION_DURATION), augmentationDuration)
      put(ContextSpecificTag(TAG_MAX_DURATION), maxDuration)
      put(ContextSpecificTag(TAG_BLIND_DURATION), blindDuration)
      endStructure()
    }
  }

  companion object {
    private const val TAG_INITIAL_DURATION = 0
    private const val TAG_AUGMENTATION_DURATION = 1
    private const val TAG_MAX_DURATION = 2
    private const val TAG_BLIND_DURATION = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct {
      tlvReader.enterStructure(tlvTag)
      val initialDuration = tlvReader.getUShort(ContextSpecificTag(TAG_INITIAL_DURATION))
      val augmentationDuration = tlvReader.getUShort(ContextSpecificTag(TAG_AUGMENTATION_DURATION))
      val maxDuration = tlvReader.getUInt(ContextSpecificTag(TAG_MAX_DURATION))
      val blindDuration = tlvReader.getUShort(ContextSpecificTag(TAG_BLIND_DURATION))

      tlvReader.exitContainer()

      return PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct(
        initialDuration,
        augmentationDuration,
        maxDuration,
        blindDuration,
      )
    }
  }
}
