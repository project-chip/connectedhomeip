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
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class ApplicationLauncherClusterApplicationStruct(
  val catalogVendorID: Int,
  val applicationID: String
) {
  override fun toString(): String = buildString {
    append("ApplicationLauncherClusterApplicationStruct {\n")
    append("\tcatalogVendorID : $catalogVendorID\n")
    append("\tapplicationID : $applicationID\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_CATALOG_VENDOR_I_D), catalogVendorID)
      put(ContextSpecificTag(TAG_APPLICATION_I_D), applicationID)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CATALOG_VENDOR_I_D = 0
    private const val TAG_APPLICATION_I_D = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ApplicationLauncherClusterApplicationStruct {
      tlvReader.enterStructure(tag)
      val catalogVendorID = tlvReader.getInt(ContextSpecificTag(TAG_CATALOG_VENDOR_I_D))
      val applicationID = tlvReader.getString(ContextSpecificTag(TAG_APPLICATION_I_D))

      tlvReader.exitContainer()

      return ApplicationLauncherClusterApplicationStruct(catalogVendorID, applicationID)
    }
  }
}
