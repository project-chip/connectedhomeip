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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ChannelClusterChannelPagingStruct(
  val previousToken: Optional<ChannelClusterPageTokenStruct>?,
  val nextToken: Optional<ChannelClusterPageTokenStruct>?,
) {
  override fun toString(): String = buildString {
    append("ChannelClusterChannelPagingStruct {\n")
    append("\tpreviousToken : $previousToken\n")
    append("\tnextToken : $nextToken\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (previousToken != null) {
        if (previousToken.isPresent) {
          val optpreviousToken = previousToken.get()
          optpreviousToken.toTlv(ContextSpecificTag(TAG_PREVIOUS_TOKEN), this)
        }
      } else {
        putNull(ContextSpecificTag(TAG_PREVIOUS_TOKEN))
      }
      if (nextToken != null) {
        if (nextToken.isPresent) {
          val optnextToken = nextToken.get()
          optnextToken.toTlv(ContextSpecificTag(TAG_NEXT_TOKEN), this)
        }
      } else {
        putNull(ContextSpecificTag(TAG_NEXT_TOKEN))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_PREVIOUS_TOKEN = 0
    private const val TAG_NEXT_TOKEN = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ChannelClusterChannelPagingStruct {
      tlvReader.enterStructure(tlvTag)
      val previousToken =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_PREVIOUS_TOKEN))) {
            Optional.of(
              ChannelClusterPageTokenStruct.fromTlv(
                ContextSpecificTag(TAG_PREVIOUS_TOKEN),
                tlvReader,
              )
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_PREVIOUS_TOKEN))
          null
        }
      val nextToken =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_NEXT_TOKEN))) {
            Optional.of(
              ChannelClusterPageTokenStruct.fromTlv(ContextSpecificTag(TAG_NEXT_TOKEN), tlvReader)
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NEXT_TOKEN))
          null
        }

      tlvReader.exitContainer()

      return ChannelClusterChannelPagingStruct(previousToken, nextToken)
    }
  }
}
