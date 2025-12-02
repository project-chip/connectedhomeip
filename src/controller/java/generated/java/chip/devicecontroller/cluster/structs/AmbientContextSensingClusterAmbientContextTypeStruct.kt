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

class AmbientContextSensingClusterAmbientContextTypeStruct (
    val ambientContextSensed: List<AmbientContextSensingClusterSemanticTagStruct>,
    val detectionStartTime: Optional<ULong>) {
  override fun toString(): String  = buildString {
    append("AmbientContextSensingClusterAmbientContextTypeStruct {\n")
    append("\tambientContextSensed : $ambientContextSensed\n")
    append("\tdetectionStartTime : $detectionStartTime\n")
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
      endStructure()
    }
  }

  companion object {
    private const val TAG_AMBIENT_CONTEXT_SENSED = 0
    private const val TAG_DETECTION_START_TIME = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : AmbientContextSensingClusterAmbientContextTypeStruct {
      tlvReader.enterStructure(tlvTag)
      val ambientContextSensed = buildList<AmbientContextSensingClusterSemanticTagStruct> {
      tlvReader.enterArray(ContextSpecificTag(TAG_AMBIENT_CONTEXT_SENSED))
      while(!tlvReader.isEndOfContainer()) {
        add(AmbientContextSensingClusterSemanticTagStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
      val detectionStartTime = if (tlvReader.isNextTag(ContextSpecificTag(TAG_DETECTION_START_TIME))) {
      Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_DETECTION_START_TIME)))
    } else {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return AmbientContextSensingClusterAmbientContextTypeStruct(ambientContextSensed, detectionStartTime)
    }
  }
}
