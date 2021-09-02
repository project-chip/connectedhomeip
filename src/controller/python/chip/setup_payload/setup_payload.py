#
#    Copyright (c) 2021 Project CHIP Authors
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

from chip.native import GetLibraryHandle, NativeLibraryHandleMethodArguments
from chip.exceptions import ChipStackError

from ctypes import CFUNCTYPE, c_char_p, c_int32, c_uint8


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

    def PrintOnboardingCodes(self, passcode, vendorId, productId, discriminator, customFlow, capabilities, version):
        self.Clear()
        err = self.chipLib.pychip_SetupPayload_PrintOnboardingCodes(
            passcode, vendorId, productId, discriminator, customFlow, capabilities, version)

        if err != 0:
            raise ChipStackError(err)

    def Print(self):
        for name, value in self.attributes.items():
            decorated_value = self.__DecorateValue(name, value)
            decorated_value = f" [{decorated_value}]" if decorated_value else ""
            print(f"{name}: {value}{decorated_value}")

        for tag in self.vendor_attributes:
            print(
                f"Vendor attribute '{tag:>3}': {self.vendor_attributes[tag]}")

    def Clear(self):
        self.attributes.clear()
        self.vendor_attributes.clear()

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
                   [c_uint32, c_uint16, c_uint16, c_uint16, uint8_t, uint8_t, uint8_t])
