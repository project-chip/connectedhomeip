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
package matter.controller.cluster.eventstructs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class MediaFileManagementClusterSharedFilesAddedEvent(
  val requestID: UShort,
  val responseID: UShort,
) {
  override fun toString(): String = buildString {
    append("MediaFileManagementClusterSharedFilesAddedEvent {\n")
    append("\trequestID : $requestID\n")
    append("\tresponseID : $responseID\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_REQUEST_ID), requestID)
      put(ContextSpecificTag(TAG_RESPONSE_ID), responseID)
      endStructure()
    }
  }

  companion object {
    private const val TAG_REQUEST_ID = 0
    private const val TAG_RESPONSE_ID = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): MediaFileManagementClusterSharedFilesAddedEvent {
      tlvReader.enterStructure(tlvTag)
      val requestID = tlvReader.getUShort(ContextSpecificTag(TAG_REQUEST_ID))
      val responseID = tlvReader.getUShort(ContextSpecificTag(TAG_RESPONSE_ID))

      tlvReader.exitContainer()

      return MediaFileManagementClusterSharedFilesAddedEvent(requestID, responseID)
    }
  }
}
