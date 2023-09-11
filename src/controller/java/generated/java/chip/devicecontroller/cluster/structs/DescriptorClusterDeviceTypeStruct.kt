/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class DescriptorClusterDeviceTypeStruct(val deviceType: Long, val revision: Int) {
  override fun toString(): String = buildString {
    append("DescriptorClusterDeviceTypeStruct {\n")
    append("\tdeviceType : $deviceType\n")
    append("\trevision : $revision\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_DEVICE_TYPE), deviceType)
      put(ContextSpecificTag(TAG_REVISION), revision)
      endStructure()
    }
  }

  companion object {
    private const val TAG_DEVICE_TYPE = 0
    private const val TAG_REVISION = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): DescriptorClusterDeviceTypeStruct {
      tlvReader.enterStructure(tag)
      val deviceType = tlvReader.getLong(ContextSpecificTag(TAG_DEVICE_TYPE))
      val revision = tlvReader.getInt(ContextSpecificTag(TAG_REVISION))

      tlvReader.exitContainer()

      return DescriptorClusterDeviceTypeStruct(deviceType, revision)
    }
  }
}
