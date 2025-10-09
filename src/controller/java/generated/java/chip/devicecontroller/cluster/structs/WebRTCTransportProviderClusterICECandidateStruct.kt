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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class WebRTCTransportProviderClusterICECandidateStruct(
  val candidate: String,
  val SDPMid: String?,
  val SDPMLineIndex: UInt?,
) {
  override fun toString(): String = buildString {
    append("WebRTCTransportProviderClusterICECandidateStruct {\n")
    append("\tcandidate : $candidate\n")
    append("\tSDPMid : $SDPMid\n")
    append("\tSDPMLineIndex : $SDPMLineIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CANDIDATE), candidate)
      if (SDPMid != null) {
        put(ContextSpecificTag(TAG_SDP_MID), SDPMid)
      } else {
        putNull(ContextSpecificTag(TAG_SDP_MID))
      }
      if (SDPMLineIndex != null) {
        put(ContextSpecificTag(TAG_SDPM_LINE_INDEX), SDPMLineIndex)
      } else {
        putNull(ContextSpecificTag(TAG_SDPM_LINE_INDEX))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CANDIDATE = 0
    private const val TAG_SDP_MID = 1
    private const val TAG_SDPM_LINE_INDEX = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): WebRTCTransportProviderClusterICECandidateStruct {
      tlvReader.enterStructure(tlvTag)
      val candidate = tlvReader.getString(ContextSpecificTag(TAG_CANDIDATE))
      val SDPMid =
        if (!tlvReader.isNull()) {
          tlvReader.getString(ContextSpecificTag(TAG_SDP_MID))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_SDP_MID))
          null
        }
      val SDPMLineIndex =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_SDPM_LINE_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_SDPM_LINE_INDEX))
          null
        }

      tlvReader.exitContainer()

      return WebRTCTransportProviderClusterICECandidateStruct(candidate, SDPMid, SDPMLineIndex)
    }
  }
}
