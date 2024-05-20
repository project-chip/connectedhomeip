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

class ContentControlClusterBlockChannelStruct(
  val blockChannelIndex: UShort?,
  val majorNumber: UShort,
  val minorNumber: UShort,
  val identifier: Optional<String>
) {
  override fun toString(): String = buildString {
    append("ContentControlClusterBlockChannelStruct {\n")
    append("\tblockChannelIndex : $blockChannelIndex\n")
    append("\tmajorNumber : $majorNumber\n")
    append("\tminorNumber : $minorNumber\n")
    append("\tidentifier : $identifier\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (blockChannelIndex != null) {
        put(ContextSpecificTag(TAG_BLOCK_CHANNEL_INDEX), blockChannelIndex)
      } else {
        putNull(ContextSpecificTag(TAG_BLOCK_CHANNEL_INDEX))
      }
      put(ContextSpecificTag(TAG_MAJOR_NUMBER), majorNumber)
      put(ContextSpecificTag(TAG_MINOR_NUMBER), minorNumber)
      if (identifier.isPresent) {
        val optidentifier = identifier.get()
        put(ContextSpecificTag(TAG_IDENTIFIER), optidentifier)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_BLOCK_CHANNEL_INDEX = 0
    private const val TAG_MAJOR_NUMBER = 1
    private const val TAG_MINOR_NUMBER = 2
    private const val TAG_IDENTIFIER = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ContentControlClusterBlockChannelStruct {
      tlvReader.enterStructure(tlvTag)
      val blockChannelIndex =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_BLOCK_CHANNEL_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_BLOCK_CHANNEL_INDEX))
          null
        }
      val majorNumber = tlvReader.getUShort(ContextSpecificTag(TAG_MAJOR_NUMBER))
      val minorNumber = tlvReader.getUShort(ContextSpecificTag(TAG_MINOR_NUMBER))
      val identifier =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_IDENTIFIER))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_IDENTIFIER)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ContentControlClusterBlockChannelStruct(
        blockChannelIndex,
        majorNumber,
        minorNumber,
        identifier
      )
    }
  }
}
