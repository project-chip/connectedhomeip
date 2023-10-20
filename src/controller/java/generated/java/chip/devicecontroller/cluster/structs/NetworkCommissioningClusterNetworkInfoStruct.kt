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

class NetworkCommissioningClusterNetworkInfoStruct (
    val networkID: ByteArray,
    val connected: Boolean) {
  override fun toString(): String  = buildString {
    append("NetworkCommissioningClusterNetworkInfoStruct {\n")
    append("\tnetworkID : $networkID\n")
    append("\tconnected : $connected\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NETWORK_I_D), networkID)
      put(ContextSpecificTag(TAG_CONNECTED), connected)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NETWORK_I_D = 0
    private const val TAG_CONNECTED = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : NetworkCommissioningClusterNetworkInfoStruct {
      tlvReader.enterStructure(tlvTag)
      val networkID = tlvReader.getByteArray(ContextSpecificTag(TAG_NETWORK_I_D))
      val connected = tlvReader.getBoolean(ContextSpecificTag(TAG_CONNECTED))
      
      tlvReader.exitContainer()

      return NetworkCommissioningClusterNetworkInfoStruct(networkID, connected)
    }
  }
}
