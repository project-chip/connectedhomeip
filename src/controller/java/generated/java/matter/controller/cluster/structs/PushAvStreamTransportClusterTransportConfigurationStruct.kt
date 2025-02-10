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

class PushAvStreamTransportClusterTransportConfigurationStruct(
  val connectionID: UShort,
  val transportStatus: UByte,
  val transportOptions: PushAvStreamTransportClusterTransportOptionsStruct,
) {
  override fun toString(): String = buildString {
    append("PushAvStreamTransportClusterTransportConfigurationStruct {\n")
    append("\tconnectionID : $connectionID\n")
    append("\ttransportStatus : $transportStatus\n")
    append("\ttransportOptions : $transportOptions\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CONNECTION_ID), connectionID)
      put(ContextSpecificTag(TAG_TRANSPORT_STATUS), transportStatus)
      transportOptions.toTlv(ContextSpecificTag(TAG_TRANSPORT_OPTIONS), this)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONNECTION_ID = 0
    private const val TAG_TRANSPORT_STATUS = 1
    private const val TAG_TRANSPORT_OPTIONS = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): PushAvStreamTransportClusterTransportConfigurationStruct {
      tlvReader.enterStructure(tlvTag)
      val connectionID = tlvReader.getUShort(ContextSpecificTag(TAG_CONNECTION_ID))
      val transportStatus = tlvReader.getUByte(ContextSpecificTag(TAG_TRANSPORT_STATUS))
      val transportOptions =
        PushAvStreamTransportClusterTransportOptionsStruct.fromTlv(
          ContextSpecificTag(TAG_TRANSPORT_OPTIONS),
          tlvReader,
        )

      tlvReader.exitContainer()

      return PushAvStreamTransportClusterTransportConfigurationStruct(
        connectionID,
        transportStatus,
        transportOptions,
      )
    }
  }
}
