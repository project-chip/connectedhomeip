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

class CommissioningProxyClusterScanResultStruct(
  val address: ByteArray?,
  val transport: UInt,
  val discriminator: UInt,
  val vendorID: UInt,
  val productID: UInt,
  val extendedData: ByteArray?,
  val wiFiBand: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("CommissioningProxyClusterScanResultStruct {\n")
    append("\taddress : $address\n")
    append("\ttransport : $transport\n")
    append("\tdiscriminator : $discriminator\n")
    append("\tvendorID : $vendorID\n")
    append("\tproductID : $productID\n")
    append("\textendedData : $extendedData\n")
    append("\twiFiBand : $wiFiBand\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (address != null) {
        put(ContextSpecificTag(TAG_ADDRESS), address)
      } else {
        putNull(ContextSpecificTag(TAG_ADDRESS))
      }
      put(ContextSpecificTag(TAG_TRANSPORT), transport)
      put(ContextSpecificTag(TAG_DISCRIMINATOR), discriminator)
      put(ContextSpecificTag(TAG_VENDOR_ID), vendorID)
      put(ContextSpecificTag(TAG_PRODUCT_ID), productID)
      if (extendedData != null) {
        put(ContextSpecificTag(TAG_EXTENDED_DATA), extendedData)
      } else {
        putNull(ContextSpecificTag(TAG_EXTENDED_DATA))
      }
      if (wiFiBand.isPresent) {
        val optwiFiBand = wiFiBand.get()
        put(ContextSpecificTag(TAG_WI_FI_BAND), optwiFiBand)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ADDRESS = 0
    private const val TAG_TRANSPORT = 1
    private const val TAG_DISCRIMINATOR = 2
    private const val TAG_VENDOR_ID = 3
    private const val TAG_PRODUCT_ID = 4
    private const val TAG_EXTENDED_DATA = 5
    private const val TAG_WI_FI_BAND = 6

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): CommissioningProxyClusterScanResultStruct {
      tlvReader.enterStructure(tlvTag)
      val address =
        if (!tlvReader.isNull()) {
          tlvReader.getByteArray(ContextSpecificTag(TAG_ADDRESS))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ADDRESS))
          null
        }
      val transport = tlvReader.getUInt(ContextSpecificTag(TAG_TRANSPORT))
      val discriminator = tlvReader.getUInt(ContextSpecificTag(TAG_DISCRIMINATOR))
      val vendorID = tlvReader.getUInt(ContextSpecificTag(TAG_VENDOR_ID))
      val productID = tlvReader.getUInt(ContextSpecificTag(TAG_PRODUCT_ID))
      val extendedData =
        if (!tlvReader.isNull()) {
          tlvReader.getByteArray(ContextSpecificTag(TAG_EXTENDED_DATA))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_EXTENDED_DATA))
          null
        }
      val wiFiBand =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_WI_FI_BAND))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_WI_FI_BAND)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return CommissioningProxyClusterScanResultStruct(
        address,
        transport,
        discriminator,
        vendorID,
        productID,
        extendedData,
        wiFiBand,
      )
    }
  }
}
