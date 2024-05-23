#
#    Copyright (c) 2021-2023 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

from ctypes import CFUNCTYPE, c_char_p, c_int32, c_uint8, c_uint16, c_uint32
from typing import Optional

from chip.exceptions import ChipStackError
from chip.native import GetLibraryHandle, NativeLibraryHandleMethodArguments


class SetupPayload:
    # AttributeVisitor: void(const char* name, const char* value)
    AttributeVisitor = CFUNCTYPE(None, c_char_p, c_char_p)

    # VendorAttributeVisitor: void(uint8_t tag, const char* value)
    VendorAttributeVisitor = CFUNCTYPE(None, c_uint8, c_char_p)

    def __init__(self):
        self.chipLib = GetLibraryHandle()
        self.__InitNativeFunctions(self.chipLib)
        self.attributes = {}
        self.vendor_attributes = {}

        def AddAttribute(name, value):
            self.attributes[name.decode()] = value.decode()

        def AddVendorAttribute(tag, value):
            self.vendor_attributes[tag] = value.decode()

        self.attribute_visitor = SetupPayload.AttributeVisitor(AddAttribute)
        self.vendor_attribute_visitor = SetupPayload.VendorAttributeVisitor(
            AddVendorAttribute)

    def ParseQrCode(self, qrCode: str):
        self.Clear()
        err = self.chipLib.pychip_SetupPayload_ParseQrCode(qrCode.upper().encode(),
                                                           self.attribute_visitor,
                                                           self.vendor_attribute_visitor)

        if err != 0:
            raise ChipStackError(err)

        return self

    def ParseManualPairingCode(self, manualPairingCode: str):
        self.Clear()
        err = self.chipLib.pychip_SetupPayload_ParseManualPairingCode(manualPairingCode.encode(),
                                                                      self.attribute_visitor,
                                                                      self.vendor_attribute_visitor)

        if err != 0:
            raise ChipStackError(err)

        return self

    # DEPRECATED
    def PrintOnboardingCodes(self, passcode, vendorId, productId, discriminator, customFlow, capabilities, version):
        self.Clear()
        err = self.chipLib.pychip_SetupPayload_PrintOnboardingCodes(
            passcode, vendorId, productId, discriminator, customFlow, capabilities, version)

        if err != 0:
            raise ChipStackError(err)

    # DEPRECATED
    def Print(self):
        for name, value in self.attributes.items():
            decorated_value = self.__DecorateValue(name, value)
            decorated_value = f" [{decorated_value}]" if decorated_value else ""
            print(f"{name}: {value}{decorated_value}")

        for tag in self.vendor_attributes:
            print(
                f"Vendor attribute '{tag:>3}': {self.vendor_attributes[tag]}")

    # DEPRECATED
    def Clear(self):
        self.attributes.clear()
        self.vendor_attributes.clear()

    # DEPRECATED
    def __DecorateValue(self, name, value):
        if name == "RendezvousInformation":
            rendezvous_methods = []
            if int(value) & 0b001:
                rendezvous_methods += ["SoftAP"]
            if int(value) & 0b010:
                rendezvous_methods += ["BLE"]
            if int(value) & 0b100:
                rendezvous_methods += ["OnNetwork"]
            return ', '.join(rendezvous_methods)

        return None

    def __InitNativeFunctions(self, chipLib):
        if chipLib.pychip_SetupPayload_ParseQrCode is not None:
            return
        setter = NativeLibraryHandleMethodArguments(chipLib)
        setter.Set("pychip_SetupPayload_ParseQrCode",
                   c_int32,
                   [c_char_p, SetupPayload.AttributeVisitor, SetupPayload.VendorAttributeVisitor])
        setter.Set("pychip_SetupPayload_ParseManualPairingCode",
                   c_int32,
                   [c_char_p, SetupPayload.AttributeVisitor, SetupPayload.VendorAttributeVisitor])
        setter.Set("pychip_SetupPayload_PrintOnboardingCodes",
                   c_int32,
                   [c_uint32, c_uint16, c_uint16, c_uint16, c_uint8, c_uint8, c_uint8])

    # Getters from parsed contents.
    # Prefer using the methods below to access setup payload information once parse.

    @property
    def version(self) -> int:
        return int(self.attributes.get("Version", "0"))

    @property
    def vendor_id(self) -> int:
        return int(self.attributes.get("VendorID", "0"))

    @property
    def product_id(self) -> int:
        return int(self.attributes.get("ProductID", "0"))

    @property
    def setup_passcode(self) -> int:
        if "SetUpPINCode" not in self.attributes:
            raise KeyError("Missing setup passcode in setup payload: parsing likely not yet done")

        return int(self.attributes["SetUpPINCode"])

    @property
    def long_discriminator(self) -> Optional[int]:
        if "Long discriminator" not in self.attributes:
            return None

        return int(self.attributes["Long discriminator"])

    @property
    def short_discriminator(self) -> Optional[int]:
        if "Short discriminator" not in self.attributes:
            return None

        return int(self.attributes["Short discriminator"])

    @property
    def commissioning_flow(self) -> int:
        return int(self.attributes.get("CommissioningFlow", "0"))

    @property
    def rendezvous_information(self) -> int:
        return int(self.attributes.get("RendezvousInformation", "0"))

    @property
    def supports_ble_commissioning(self) -> bool:
        return (self.rendezvous_information & 0b010) != 0

    @property
    def supports_on_network_commissioning(self) -> bool:
        return (self.rendezvous_information & 0b100) != 0
