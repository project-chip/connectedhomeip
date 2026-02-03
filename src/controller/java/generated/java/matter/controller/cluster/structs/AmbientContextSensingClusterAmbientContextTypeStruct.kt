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

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AmbientContextSensingClusterAmbientContextTypeStruct(
  val ambientContextSensed: List<AmbientContextSensingClusterSemanticTagStruct>,
  val detectionStartTime: Optional<UInt>,
  val objectCountThreshold: Optional<UShort>,
  val objectCount: Optional<UShort>,
) {
  override fun toString(): String = buildString {
    append("AmbientContextSensingClusterAmbientContextTypeStruct {\n")
    append("\tambientContextSensed : $ambientContextSensed\n")
    append("\tdetectionStartTime : $detectionStartTime\n")
    append("\tobjectCountThreshold : $objectCountThreshold\n")
    append("\tobjectCount : $objectCount\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      startArray(ContextSpecificTag(TAG_AMBIENT_CONTEXT_SENSED))
      for (item in ambientContextSensed.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      if (detectionStartTime.isPresent) {
        val optdetectionStartTime = detectionStartTime.get()
        put(ContextSpecificTag(TAG_DETECTION_START_TIME), optdetectionStartTime)
      }
      if (objectCountThreshold.isPresent) {
        val optobjectCountThreshold = objectCountThreshold.get()
        put(ContextSpecificTag(TAG_OBJECT_COUNT_THRESHOLD), optobjectCountThreshold)
      }
      if (objectCount.isPresent) {
        val optobjectCount = objectCount.get()
        put(ContextSpecificTag(TAG_OBJECT_COUNT), optobjectCount)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_AMBIENT_CONTEXT_SENSED = 0
    private const val TAG_DETECTION_START_TIME = 1
    private const val TAG_OBJECT_COUNT_THRESHOLD = 2
    private const val TAG_OBJECT_COUNT = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AmbientContextSensingClusterAmbientContextTypeStruct {
      tlvReader.enterStructure(tlvTag)
      val ambientContextSensed =
        buildList<AmbientContextSensingClusterSemanticTagStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_AMBIENT_CONTEXT_SENSED))
          while (!tlvReader.isEndOfContainer()) {
            add(AmbientContextSensingClusterSemanticTagStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      val detectionStartTime =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DETECTION_START_TIME))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_DETECTION_START_TIME)))
        } else {
          Optional.empty()
        }
      val objectCountThreshold =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_OBJECT_COUNT_THRESHOLD))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_OBJECT_COUNT_THRESHOLD)))
        } else {
          Optional.empty()
        }
      val objectCount =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_OBJECT_COUNT))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_OBJECT_COUNT)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return AmbientContextSensingClusterAmbientContextTypeStruct(
        ambientContextSensed,
        detectionStartTime,
        objectCountThreshold,
        objectCount,
      )
    }
  }
}
