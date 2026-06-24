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

class NetworkIdentityManagementClusterActiveNetworkIdentityStruct(
  val index: UInt,
  val type: UInt,
  val identifier: ByteArray,
  val createdTimestamp: ULong,
  val current: Boolean,
  val remainingClients: UInt?,
) {
  override fun toString(): String = buildString {
    append("NetworkIdentityManagementClusterActiveNetworkIdentityStruct {\n")
    append("\tindex : $index\n")
    append("\ttype : $type\n")
    append("\tidentifier : $identifier\n")
    append("\tcreatedTimestamp : $createdTimestamp\n")
    append("\tcurrent : $current\n")
    append("\tremainingClients : $remainingClients\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_INDEX), index)
      put(ContextSpecificTag(TAG_TYPE), type)
      put(ContextSpecificTag(TAG_IDENTIFIER), identifier)
      put(ContextSpecificTag(TAG_CREATED_TIMESTAMP), createdTimestamp)
      put(ContextSpecificTag(TAG_CURRENT), current)
      if (remainingClients != null) {
        put(ContextSpecificTag(TAG_REMAINING_CLIENTS), remainingClients)
      } else {
        putNull(ContextSpecificTag(TAG_REMAINING_CLIENTS))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_INDEX = 0
    private const val TAG_TYPE = 1
    private const val TAG_IDENTIFIER = 2
    private const val TAG_CREATED_TIMESTAMP = 3
    private const val TAG_CURRENT = 4
    private const val TAG_REMAINING_CLIENTS = 5

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): NetworkIdentityManagementClusterActiveNetworkIdentityStruct {
      tlvReader.enterStructure(tlvTag)
      val index = tlvReader.getUInt(ContextSpecificTag(TAG_INDEX))
      val type = tlvReader.getUInt(ContextSpecificTag(TAG_TYPE))
      val identifier = tlvReader.getByteArray(ContextSpecificTag(TAG_IDENTIFIER))
      val createdTimestamp = tlvReader.getULong(ContextSpecificTag(TAG_CREATED_TIMESTAMP))
      val current = tlvReader.getBoolean(ContextSpecificTag(TAG_CURRENT))
      val remainingClients =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_REMAINING_CLIENTS))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_REMAINING_CLIENTS))
          null
        }

      tlvReader.exitContainer()

      return NetworkIdentityManagementClusterActiveNetworkIdentityStruct(
        index,
        type,
        identifier,
        createdTimestamp,
        current,
        remainingClients,
      )
    }
  }
}
