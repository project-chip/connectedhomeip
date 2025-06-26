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

class PushAvStreamTransportClusterSupportedFormatStruct(
  val containerFormat: UByte,
  val ingestMethod: UByte,
) {
  override fun toString(): String = buildString {
    append("PushAvStreamTransportClusterSupportedFormatStruct {\n")
    append("\tcontainerFormat : $containerFormat\n")
    append("\tingestMethod : $ingestMethod\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CONTAINER_FORMAT), containerFormat)
      put(ContextSpecificTag(TAG_INGEST_METHOD), ingestMethod)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONTAINER_FORMAT = 0
    private const val TAG_INGEST_METHOD = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): PushAvStreamTransportClusterSupportedFormatStruct {
      tlvReader.enterStructure(tlvTag)
      val containerFormat = tlvReader.getUByte(ContextSpecificTag(TAG_CONTAINER_FORMAT))
      val ingestMethod = tlvReader.getUByte(ContextSpecificTag(TAG_INGEST_METHOD))

      tlvReader.exitContainer()

      return PushAvStreamTransportClusterSupportedFormatStruct(containerFormat, ingestMethod)
    }
  }
}
