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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AmbientContextSensingClusterAmbientContextDetectStartedEvent(
  val ambientContextDetected:
    Optional<
      chip.devicecontroller.cluster.structs.AmbientContextSensingClusterAmbientContextTypeStruct
    >,
  val objectCountReached: Optional<Boolean>,
  val objectCount: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("AmbientContextSensingClusterAmbientContextDetectStartedEvent {\n")
    append("\tambientContextDetected : $ambientContextDetected\n")
    append("\tobjectCountReached : $objectCountReached\n")
    append("\tobjectCount : $objectCount\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (ambientContextDetected.isPresent) {
        val optambientContextDetected = ambientContextDetected.get()
        optambientContextDetected.toTlv(ContextSpecificTag(TAG_AMBIENT_CONTEXT_DETECTED), this)
      }
      if (objectCountReached.isPresent) {
        val optobjectCountReached = objectCountReached.get()
        put(ContextSpecificTag(TAG_OBJECT_COUNT_REACHED), optobjectCountReached)
      }
      if (objectCount.isPresent) {
        val optobjectCount = objectCount.get()
        put(ContextSpecificTag(TAG_OBJECT_COUNT), optobjectCount)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_AMBIENT_CONTEXT_DETECTED = 0
    private const val TAG_OBJECT_COUNT_REACHED = 1
    private const val TAG_OBJECT_COUNT = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AmbientContextSensingClusterAmbientContextDetectStartedEvent {
      tlvReader.enterStructure(tlvTag)
      val ambientContextDetected =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_AMBIENT_CONTEXT_DETECTED))) {
          Optional.of(
            chip.devicecontroller.cluster.structs
              .AmbientContextSensingClusterAmbientContextTypeStruct
              .fromTlv(ContextSpecificTag(TAG_AMBIENT_CONTEXT_DETECTED), tlvReader)
          )
        } else {
          Optional.empty()
        }
      val objectCountReached =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_OBJECT_COUNT_REACHED))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_OBJECT_COUNT_REACHED)))
        } else {
          Optional.empty()
        }
      val objectCount =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_OBJECT_COUNT))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_OBJECT_COUNT)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return AmbientContextSensingClusterAmbientContextDetectStartedEvent(
        ambientContextDetected,
        objectCountReached,
        objectCount,
      )
    }
  }
}
