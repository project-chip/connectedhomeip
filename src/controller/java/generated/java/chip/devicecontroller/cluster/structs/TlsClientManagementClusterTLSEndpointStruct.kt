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

class TlsClientManagementClusterTLSEndpointStruct(
  val endpointID: UInt,
  val hostname: ByteArray,
  val port: UInt,
  val caid: UInt,
  val ccdid: UInt?,
  val status: UInt,
) {
  override fun toString(): String = buildString {
    append("TlsClientManagementClusterTLSEndpointStruct {\n")
    append("\tendpointID : $endpointID\n")
    append("\thostname : $hostname\n")
    append("\tport : $port\n")
    append("\tcaid : $caid\n")
    append("\tccdid : $ccdid\n")
    append("\tstatus : $status\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ENDPOINT_ID), endpointID)
      put(ContextSpecificTag(TAG_HOSTNAME), hostname)
      put(ContextSpecificTag(TAG_PORT), port)
      put(ContextSpecificTag(TAG_CAID), caid)
      if (ccdid != null) {
        put(ContextSpecificTag(TAG_CCDID), ccdid)
      } else {
        putNull(ContextSpecificTag(TAG_CCDID))
      }
      put(ContextSpecificTag(TAG_STATUS), status)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ENDPOINT_ID = 0
    private const val TAG_HOSTNAME = 1
    private const val TAG_PORT = 2
    private const val TAG_CAID = 3
    private const val TAG_CCDID = 4
    private const val TAG_STATUS = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): TlsClientManagementClusterTLSEndpointStruct {
      tlvReader.enterStructure(tlvTag)
      val endpointID = tlvReader.getUInt(ContextSpecificTag(TAG_ENDPOINT_ID))
      val hostname = tlvReader.getByteArray(ContextSpecificTag(TAG_HOSTNAME))
      val port = tlvReader.getUInt(ContextSpecificTag(TAG_PORT))
      val caid = tlvReader.getUInt(ContextSpecificTag(TAG_CAID))
      val ccdid =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_CCDID))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CCDID))
          null
        }
      val status = tlvReader.getUInt(ContextSpecificTag(TAG_STATUS))

      tlvReader.exitContainer()

      return TlsClientManagementClusterTLSEndpointStruct(
        endpointID,
        hostname,
        port,
        caid,
        ccdid,
        status,
      )
    }
  }
}
