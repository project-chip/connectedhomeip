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

class MediaFileManagementClusterFileDescriptionStruct(
  val fileID: ULong,
  val name: String,
  val size: ULong,
  val mimeType: String,
  val imageUri: String,
) {
  override fun toString(): String = buildString {
    append("MediaFileManagementClusterFileDescriptionStruct {\n")
    append("\tfileID : $fileID\n")
    append("\tname : $name\n")
    append("\tsize : $size\n")
    append("\tmimeType : $mimeType\n")
    append("\timageUri : $imageUri\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_FILE_ID), fileID)
      put(ContextSpecificTag(TAG_NAME), name)
      put(ContextSpecificTag(TAG_SIZE), size)
      put(ContextSpecificTag(TAG_MIME_TYPE), mimeType)
      put(ContextSpecificTag(TAG_IMAGE_URI), imageUri)
      endStructure()
    }
  }

  companion object {
    private const val TAG_FILE_ID = 0
    private const val TAG_NAME = 1
    private const val TAG_SIZE = 2
    private const val TAG_MIME_TYPE = 3
    private const val TAG_IMAGE_URI = 4

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): MediaFileManagementClusterFileDescriptionStruct {
      tlvReader.enterStructure(tlvTag)
      val fileID = tlvReader.getULong(ContextSpecificTag(TAG_FILE_ID))
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val size = tlvReader.getULong(ContextSpecificTag(TAG_SIZE))
      val mimeType = tlvReader.getString(ContextSpecificTag(TAG_MIME_TYPE))
      val imageUri = tlvReader.getString(ContextSpecificTag(TAG_IMAGE_URI))

      tlvReader.exitContainer()

      return MediaFileManagementClusterFileDescriptionStruct(fileID, name, size, mimeType, imageUri)
    }
  }
}
