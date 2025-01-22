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
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class TlsCertificateManagementClusterTLSClientCertificateDetailStruct(
  val ccdid: UInt,
  val clientCertificate: ByteArray,
  val intermediateCertificates: List<ByteArray>,
) {
  override fun toString(): String = buildString {
    append("TlsCertificateManagementClusterTLSClientCertificateDetailStruct {\n")
    append("\tccdid : $ccdid\n")
    append("\tclientCertificate : $clientCertificate\n")
    append("\tintermediateCertificates : $intermediateCertificates\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CCDID), ccdid)
      put(ContextSpecificTag(TAG_CLIENT_CERTIFICATE), clientCertificate)
      startArray(ContextSpecificTag(TAG_INTERMEDIATE_CERTIFICATES))
      for (item in intermediateCertificates.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_CCDID = 0
    private const val TAG_CLIENT_CERTIFICATE = 1
    private const val TAG_INTERMEDIATE_CERTIFICATES = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): TlsCertificateManagementClusterTLSClientCertificateDetailStruct {
      tlvReader.enterStructure(tlvTag)
      val ccdid = tlvReader.getUInt(ContextSpecificTag(TAG_CCDID))
      val clientCertificate = tlvReader.getByteArray(ContextSpecificTag(TAG_CLIENT_CERTIFICATE))
      val intermediateCertificates =
        buildList<ByteArray> {
          tlvReader.enterArray(ContextSpecificTag(TAG_INTERMEDIATE_CERTIFICATES))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getByteArray(AnonymousTag))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return TlsCertificateManagementClusterTLSClientCertificateDetailStruct(
        ccdid,
        clientCertificate,
        intermediateCertificates,
      )
    }
  }
}
