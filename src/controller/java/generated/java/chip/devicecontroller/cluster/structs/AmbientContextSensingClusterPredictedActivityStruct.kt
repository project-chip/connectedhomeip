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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AmbientContextSensingClusterPredictedActivityStruct(
  val startTimestamp: ULong,
  val endTimestamp: ULong,
  val ambientContextType: Optional<List<AmbientContextSensingClusterSemanticTagStruct>>,
  val crowdDetected: Optional<Boolean>,
  val crowdCount: Optional<UInt>,
  val confidence: UInt,
) {
  override fun toString(): String = buildString {
    append("AmbientContextSensingClusterPredictedActivityStruct {\n")
    append("\tstartTimestamp : $startTimestamp\n")
    append("\tendTimestamp : $endTimestamp\n")
    append("\tambientContextType : $ambientContextType\n")
    append("\tcrowdDetected : $crowdDetected\n")
    append("\tcrowdCount : $crowdCount\n")
    append("\tconfidence : $confidence\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_START_TIMESTAMP), startTimestamp)
      put(ContextSpecificTag(TAG_END_TIMESTAMP), endTimestamp)
      if (ambientContextType.isPresent) {
        val optambientContextType = ambientContextType.get()
        startArray(ContextSpecificTag(TAG_AMBIENT_CONTEXT_TYPE))
        for (item in optambientContextType.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      }
      if (crowdDetected.isPresent) {
        val optcrowdDetected = crowdDetected.get()
        put(ContextSpecificTag(TAG_CROWD_DETECTED), optcrowdDetected)
      }
      if (crowdCount.isPresent) {
        val optcrowdCount = crowdCount.get()
        put(ContextSpecificTag(TAG_CROWD_COUNT), optcrowdCount)
      }
      put(ContextSpecificTag(TAG_CONFIDENCE), confidence)
      endStructure()
    }
  }

  companion object {
    private const val TAG_START_TIMESTAMP = 0
    private const val TAG_END_TIMESTAMP = 1
    private const val TAG_AMBIENT_CONTEXT_TYPE = 2
    private const val TAG_CROWD_DETECTED = 3
    private const val TAG_CROWD_COUNT = 4
    private const val TAG_CONFIDENCE = 5

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AmbientContextSensingClusterPredictedActivityStruct {
      tlvReader.enterStructure(tlvTag)
      val startTimestamp = tlvReader.getULong(ContextSpecificTag(TAG_START_TIMESTAMP))
      val endTimestamp = tlvReader.getULong(ContextSpecificTag(TAG_END_TIMESTAMP))
      val ambientContextType =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_AMBIENT_CONTEXT_TYPE))) {
          Optional.of(
            buildList<AmbientContextSensingClusterSemanticTagStruct> {
              tlvReader.enterArray(ContextSpecificTag(TAG_AMBIENT_CONTEXT_TYPE))
              while (!tlvReader.isEndOfContainer()) {
                add(AmbientContextSensingClusterSemanticTagStruct.fromTlv(AnonymousTag, tlvReader))
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }
      val crowdDetected =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CROWD_DETECTED))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_CROWD_DETECTED)))
        } else {
          Optional.empty()
        }
      val crowdCount =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CROWD_COUNT))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_CROWD_COUNT)))
        } else {
          Optional.empty()
        }
      val confidence = tlvReader.getUInt(ContextSpecificTag(TAG_CONFIDENCE))

      tlvReader.exitContainer()

      return AmbientContextSensingClusterPredictedActivityStruct(
        startTimestamp,
        endTimestamp,
        ambientContextType,
        crowdDetected,
        crowdCount,
        confidence,
      )
    }
  }
}
