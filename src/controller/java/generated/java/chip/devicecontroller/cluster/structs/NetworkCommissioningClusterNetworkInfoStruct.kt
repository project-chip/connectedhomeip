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

class NetworkCommissioningClusterNetworkInfoStruct(
  val networkID: ByteArray,
  val connected: Boolean,
  val networkIdentifier: Optional<ByteArray>?,
  val clientIdentifier: Optional<ByteArray>?
) {
  override fun toString(): String = buildString {
    append("NetworkCommissioningClusterNetworkInfoStruct {\n")
    append("\tnetworkID : $networkID\n")
    append("\tconnected : $connected\n")
    append("\tnetworkIdentifier : $networkIdentifier\n")
    append("\tclientIdentifier : $clientIdentifier\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NETWORK_I_D), networkID)
      put(ContextSpecificTag(TAG_CONNECTED), connected)
      if (networkIdentifier != null) {
        if (networkIdentifier.isPresent) {
          val optnetworkIdentifier = networkIdentifier.get()
          put(ContextSpecificTag(TAG_NETWORK_IDENTIFIER), optnetworkIdentifier)
        }
      } else {
        putNull(ContextSpecificTag(TAG_NETWORK_IDENTIFIER))
      }
      if (clientIdentifier != null) {
        if (clientIdentifier.isPresent) {
          val optclientIdentifier = clientIdentifier.get()
          put(ContextSpecificTag(TAG_CLIENT_IDENTIFIER), optclientIdentifier)
        }
      } else {
        putNull(ContextSpecificTag(TAG_CLIENT_IDENTIFIER))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_NETWORK_I_D = 0
    private const val TAG_CONNECTED = 1
    private const val TAG_NETWORK_IDENTIFIER = 2
    private const val TAG_CLIENT_IDENTIFIER = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): NetworkCommissioningClusterNetworkInfoStruct {
      tlvReader.enterStructure(tlvTag)
      val networkID = tlvReader.getByteArray(ContextSpecificTag(TAG_NETWORK_I_D))
      val connected = tlvReader.getBoolean(ContextSpecificTag(TAG_CONNECTED))
      val networkIdentifier =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_NETWORK_IDENTIFIER))) {
            Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_NETWORK_IDENTIFIER)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NETWORK_IDENTIFIER))
          null
        }
      val clientIdentifier =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_CLIENT_IDENTIFIER))) {
            Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_CLIENT_IDENTIFIER)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CLIENT_IDENTIFIER))
          null
        }

      tlvReader.exitContainer()

      return NetworkCommissioningClusterNetworkInfoStruct(
        networkID,
        connected,
        networkIdentifier,
        clientIdentifier
      )
    }
  }
}
