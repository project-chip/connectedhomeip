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

class ThreadNetworkDirectoryClusterThreadNetworkStruct(
  val extendedPanID: ByteArray,
  val networkName: String,
  val channel: UShort,
  val activeTimestamp: ULong,
) {
  override fun toString(): String = buildString {
    append("ThreadNetworkDirectoryClusterThreadNetworkStruct {\n")
    append("\textendedPanID : $extendedPanID\n")
    append("\tnetworkName : $networkName\n")
    append("\tchannel : $channel\n")
    append("\tactiveTimestamp : $activeTimestamp\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_EXTENDED_PAN_ID), extendedPanID)
      put(ContextSpecificTag(TAG_NETWORK_NAME), networkName)
      put(ContextSpecificTag(TAG_CHANNEL), channel)
      put(ContextSpecificTag(TAG_ACTIVE_TIMESTAMP), activeTimestamp)
      endStructure()
    }
  }

  companion object {
    private const val TAG_EXTENDED_PAN_ID = 0
    private const val TAG_NETWORK_NAME = 1
    private const val TAG_CHANNEL = 2
    private const val TAG_ACTIVE_TIMESTAMP = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ThreadNetworkDirectoryClusterThreadNetworkStruct {
      tlvReader.enterStructure(tlvTag)
      val extendedPanID = tlvReader.getByteArray(ContextSpecificTag(TAG_EXTENDED_PAN_ID))
      val networkName = tlvReader.getString(ContextSpecificTag(TAG_NETWORK_NAME))
      val channel = tlvReader.getUShort(ContextSpecificTag(TAG_CHANNEL))
      val activeTimestamp = tlvReader.getULong(ContextSpecificTag(TAG_ACTIVE_TIMESTAMP))

      tlvReader.exitContainer()

      return ThreadNetworkDirectoryClusterThreadNetworkStruct(
        extendedPanID,
        networkName,
        channel,
        activeTimestamp,
      )
    }
  }
}
