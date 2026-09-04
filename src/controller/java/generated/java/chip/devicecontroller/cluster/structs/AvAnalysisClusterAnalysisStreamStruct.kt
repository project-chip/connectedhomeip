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

class AvAnalysisClusterAnalysisStreamStruct(
  val analysisStreamID: UInt,
  val webRTCEndpointID: Optional<UInt>?,
  val pushAVEndpointID: Optional<UInt>?,
  val analysisStreamState: UInt,
) {
  override fun toString(): String = buildString {
    append("AvAnalysisClusterAnalysisStreamStruct {\n")
    append("\tanalysisStreamID : $analysisStreamID\n")
    append("\twebRTCEndpointID : $webRTCEndpointID\n")
    append("\tpushAVEndpointID : $pushAVEndpointID\n")
    append("\tanalysisStreamState : $analysisStreamState\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ANALYSIS_STREAM_ID), analysisStreamID)
      if (webRTCEndpointID != null) {
        if (webRTCEndpointID.isPresent) {
          val optwebRTCEndpointID = webRTCEndpointID.get()
          put(ContextSpecificTag(TAG_WEB_RTC_ENDPOINT_ID), optwebRTCEndpointID)
        }
      } else {
        putNull(ContextSpecificTag(TAG_WEB_RTC_ENDPOINT_ID))
      }
      if (pushAVEndpointID != null) {
        if (pushAVEndpointID.isPresent) {
          val optpushAVEndpointID = pushAVEndpointID.get()
          put(ContextSpecificTag(TAG_PUSH_AV_ENDPOINT_ID), optpushAVEndpointID)
        }
      } else {
        putNull(ContextSpecificTag(TAG_PUSH_AV_ENDPOINT_ID))
      }
      put(ContextSpecificTag(TAG_ANALYSIS_STREAM_STATE), analysisStreamState)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ANALYSIS_STREAM_ID = 0
    private const val TAG_WEB_RTC_ENDPOINT_ID = 1
    private const val TAG_PUSH_AV_ENDPOINT_ID = 2
    private const val TAG_ANALYSIS_STREAM_STATE = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): AvAnalysisClusterAnalysisStreamStruct {
      tlvReader.enterStructure(tlvTag)
      val analysisStreamID = tlvReader.getUInt(ContextSpecificTag(TAG_ANALYSIS_STREAM_ID))
      val webRTCEndpointID =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_WEB_RTC_ENDPOINT_ID))) {
            Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_WEB_RTC_ENDPOINT_ID)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_WEB_RTC_ENDPOINT_ID))
          null
        }
      val pushAVEndpointID =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_PUSH_AV_ENDPOINT_ID))) {
            Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_PUSH_AV_ENDPOINT_ID)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_PUSH_AV_ENDPOINT_ID))
          null
        }
      val analysisStreamState = tlvReader.getUInt(ContextSpecificTag(TAG_ANALYSIS_STREAM_STATE))

      tlvReader.exitContainer()

      return AvAnalysisClusterAnalysisStreamStruct(
        analysisStreamID,
        webRTCEndpointID,
        pushAVEndpointID,
        analysisStreamState,
      )
    }
  }
}
