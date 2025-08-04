#
#   Copyright (c) 2022 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

import argparse
import datetime
import os
import subprocess


class FactoryDataWriter:
    script_dir = os.path.dirname(__file__)
    # CONSTANTS
    TEMP_FILE = script_dir + "/tmp_nvm3.s37"
    OUT_FILE = script_dir + "/matter_factorydata.s37"  # Final output file containing the nvm3 data to flash to the device
    BASE_MG12_FILE = script_dir + "/base_matter_mg12_nvm3.s37"
    BASE_MG24_FILE = script_dir + "/base_matter_mg24_nvm3.s37"
    # nvm3 keys to set
    SERIAL_NUMBER_NVM3_KEY = "0x87200:"
    MANUFACTURING_DATE_NVM3_KEY = "0x87204:"
    SETUP_PAYLOAD_NVM3_KEY = "0x87205:"
    DISCRIMINATOR_NVM3_KEY = "0x87207:"
    ITERATIONCOUNT_NVM3_KEY = "0x87208:"
    SALT_NVM3_KEY = "0x87209:"
    VERIFIER_NVM3_KEY = "0x8720A:"
    PRODUCT_ID_NVM3_KEY = "0x8720B:"
    VENDOR_ID_NVM3_KEY = "0x8720C:"
    VENDOR_NAME_NVM3_KEY = "0x8720D:"
    PRODUCT_NAME_NVM3_KEY = "0x8720E:"
    HW_VER_STR_NVM3_KEY = "0x8720F:"
    UNIQUE_ID_NVM3_KEY = "0x8721F:"
    HW_VER_NVM3_KEY = "0x87308:"
    PRODUCT_LABEL_NVM3_KEY = "0x87210:"
    PRODUCT_URL_NVM3_KEY = "0x87211:"
    PART_NUMBER_NVM3_KEY = "0x87212:"

    def generate_spake2p_verifier(self):
        """ Generate Spake2+ verifier using the external spake2p tool

        Args:
            The whole set of args passed to the script. The required one are:
            gen_spake2p_path: path to spake2p executable
            spake2_iteration: Iteration counter for Spake2+ verifier generation
            passcode: Pairing passcode using in Spake2+
            spake2_salt: Salt used to generate Spake2+ verifier

        Returns:
            The generated verifier string
        """
        cmd = [
            self._args.gen_spake2p_path, 'gen-verifier',
            '--iteration-count', str(self._args.spake2_iteration),
            '--salt', self._args.spake2_salt,
            '--pin-code', str(self._args.passcode),
            '--out', '-',
        ]
        output = subprocess.check_output(cmd)
        output = output.decode('utf-8').splitlines()
        generation_results = dict(zip(output[0].split(','), output[1].split(',')))
        return generation_results["Verifier"]

    # Populates numberOfBits starting from LSB of input into bits, which is assumed to be zero-initialized
    def WriteBits(self, bits, offset, input, numberOfBits, totalPayloadSizeInBits):
        if ((offset + numberOfBits) > totalPayloadSizeInBits):
            print("THIS IS NOT VALID")
            return
        # input < 1u << numberOfBits);

        index = offset
        offset += numberOfBits
        while (input != 0):
            if (input & 1):
                bits[int(index / 8)] |= (1 << (index % 8))
            index += 1
            input >>= 1

        return offset

    def generateQrCodeBitSet(self):
        kVersionFieldLengthInBits = 3
        kVendorIDFieldLengthInBits = 16
        kProductIDFieldLengthInBits = 16
        kCommissioningFlowFieldLengthInBits = 2
        kRendezvousInfoFieldLengthInBits = 8
        kPayloadDiscriminatorFieldLengthInBits = 12
        kSetupPINCodeFieldLengthInBits = 27
        kPaddingFieldLengthInBits = 4

        kTotalPayloadDataSizeInBits = (kVersionFieldLengthInBits + kVendorIDFieldLengthInBits + kProductIDFieldLengthInBits +
                                       kCommissioningFlowFieldLengthInBits + kRendezvousInfoFieldLengthInBits + kPayloadDiscriminatorFieldLengthInBits +
                                       kSetupPINCodeFieldLengthInBits + kPaddingFieldLengthInBits)

        offset = 0
        fillBits = [0] * int(kTotalPayloadDataSizeInBits / 8)
        offset = self.WriteBits(fillBits, offset, 0, kVersionFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, self._args.vendor_id, kVendorIDFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, self._args.product_id, kProductIDFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, self._args.commissioning_flow,
                                kCommissioningFlowFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, self._args.rendezvous_flag,
                                kRendezvousInfoFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, self._args.discriminator,
                                kPayloadDiscriminatorFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, self._args.passcode, kSetupPINCodeFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, 0, kPaddingFieldLengthInBits, kTotalPayloadDataSizeInBits)

        return str(bytes(fillBits).hex())

    def __init__(self, arguments) -> None:
        """ Do some checks on the received arguments.
            Generate the Spake2+ verifier if needed and assign the values
            to the global variables

        Args:
            The whole set of args passed to the script.
        """
        kMaxVendorNameLength = 32
        kMaxProductNameLength = 32
        kMaxHardwareVersionStringLength = 64
        kMaxSerialNumberLength = 32
        kUniqueIDLength = 16
        kMaxProductUrlLenght = 256
        kMaxPartNumberLength = 32
        kMaxProductLabelLength = 64

        INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444,
                             55555555, 66666666, 77777777, 88888888, 99999999, 12345678, 87654321]

        assert (bool(arguments.gen_spake2p_path) != bool(arguments.spake2_verifier)
                ), "Provide either the spake2_verifier string or the path to the spake2 generator"
        assert arguments.passcode not in INVALID_PASSCODES, "The provided passcode is invalid"

        self._args = arguments

        if self._args.unique_id:
            assert (len(bytearray.fromhex(self._args.unique_id)) == kUniqueIDLength), "Provide a 16 bytes unique id"
        if self._args.product_name:
            assert (len(self._args.product_name) <= kMaxProductNameLength), "Product name exceeds the size limit"
        if self._args.vendor_name:
            assert (len(self._args.vendor_name) <= kMaxVendorNameLength), "Vendor name exceeds the size limit"
        if self._args.hw_version_str:
            assert (len(self._args.hw_version_str) <= kMaxHardwareVersionStringLength), "Hardware version string exceeds the size limit"
        if self._args.serial_number:
            assert (len(self._args.serial_number) <= kMaxSerialNumberLength), "Serial number exceeds the size limit"
        if self._args.manufacturing_date:
            try:
                datetime.datetime.strptime(self._args.manufacturing_date, '%Y-%m-%d')
            except ValueError:
                raise ValueError("Incorrect manufacturing data format, should be YYYY-MM-DD")
        if self._args.commissioning_flow:
            assert (self._args.commissioning_flow <= 3), "Invalid commissioning flow value"
        if self._args.rendezvous_flag:
            assert (self._args.rendezvous_flag <= 7), "Invalid rendez-vous flag value"
        if self._args.gen_spake2p_path:
            self._args.spake2_verifier = self.generate_spake2p_verifier()
        if self._args.product_label:
            assert (len(self._args.product_label) <= kMaxProductLabelLength), "Product Label exceeds the size limit"
        if self._args.product_url:
            assert (len(self._args.product_url) <= kMaxProductUrlLenght), "Product URL exceeds the size limit"
        if self._args.part_number:
            assert (len(self._args.part_number) <= kMaxPartNumberLength), "Part number exceeds the size limit"

    def add_SerialNo_To_CMD(self, cmdList):
        """ Add the jtag serial command to the commander command

            Args:
            The commander command in list format
        """
        if self._args.jtag_serial:
            cmdList.extend(["--serialno", self._args.jtagSerial])

    def create_nvm3injected_image(self):
        """ Use commander command lines create a binary flashable to the EFR32
            containing the factory commissioning data in NVM3 section
        """
        isDeviceConnected = True

        # Retrieve the device current nvm3 data in a binary file
        # It will be used as base to add the new credentials
        inputImage = self.TEMP_FILE
        cmd = ['commander', 'nvm3', 'read', '-o', inputImage, ]
        self.add_SerialNo_To_CMD(cmd)
        results = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)

        if results.returncode != 0:
            # No nvm3 section found. Retrieve the device info
            cmd = ['commander', 'device', 'info', ]
            self.add_SerialNo_To_CMD(cmd)
            try:
                output = subprocess.check_output(cmd)
                output = output.decode('utf-8').splitlines()
                deviceInfo = dict(map(str.strip, lines.split(':')) for lines in output[0:len(output)-1])
                # Only MG12 and MG24 are supported in matter currently
                if "EFR32MG12" in deviceInfo["Part Number"]:
                    inputImage = self.BASE_MG12_FILE
                elif "EFR32MG24" in deviceInfo["Part Number"]:
                    inputImage = self.BASE_MG24_FILE
                else:
                    raise Exception('Invalid MCU')
            except Exception:
                isDeviceConnected = False
                print("Device not connected")
                # When no device is connected user needs to provide the mcu family for which those credentials are to be created
                if self._args.mcu_family:
                    if "EFR32MG12" == self._args.mcu_family:
                        inputImage = self.BASE_MG12_FILE
                    elif "EFR32MG24" == self._args.mcu_family:
                        inputImage = self.BASE_MG24_FILE
                else:
                    print("Connect debug port or provide the mcu_family")
                    return

        # Convert interger to little endian hex format and strings to hex byte array format for nvm3 storage
        spake2pIterationCount = self._args.spake2_iteration.to_bytes(4, 'little').hex()
        discriminator = self._args.discriminator.to_bytes(2, 'little').hex()
        saltByteArray = bytes(self._args.spake2_salt, 'utf-8').hex()
        verifierByteArray = bytes(self._args.spake2_verifier, 'utf-8').hex()

        productId = self._args.product_id.to_bytes(2, "little").hex()
        vendorId = self._args.vendor_id.to_bytes(2, "little").hex()

        # create the binary containing the new nvm3 data
        cmd = [
            "commander", "nvm3", "set", inputImage,
            "--object", self.DISCRIMINATOR_NVM3_KEY + str(discriminator),
            "--object", self.SETUP_PAYLOAD_NVM3_KEY + self.generateQrCodeBitSet(),
            "--object", self.ITERATIONCOUNT_NVM3_KEY + str(spake2pIterationCount),
            "--object", self.SALT_NVM3_KEY + str(saltByteArray),
            "--object", self.VERIFIER_NVM3_KEY + str(verifierByteArray),
            "--object", self.PRODUCT_ID_NVM3_KEY + str(productId),
            "--object", self.VENDOR_ID_NVM3_KEY + str(vendorId),
        ]

        if self._args.product_name:
            productNameByteArray = bytes(self._args.product_name, 'utf-8').hex()
            cmd.extend(["--object", self.PRODUCT_NAME_NVM3_KEY + str(productNameByteArray)])

        if self._args.vendor_name:
            vendorNameByteArray = bytes(self._args.vendor_name, 'utf-8').hex()
            cmd.extend(["--object", self.VENDOR_NAME_NVM3_KEY + str(vendorNameByteArray)])

        if self._args.hw_version:
            hwVersionByteArray = self._args.hw_version.to_bytes(2, "little").hex()
            cmd.extend(["--object", self.HW_VER_NVM3_KEY + str(hwVersionByteArray)])

        if self._args.hw_version_str:
            hwVersionByteArray = bytes(self._args.hw_version_str, 'utf-8').hex()
            cmd.extend(["--object", self.HW_VER_STR_NVM3_KEY + str(hwVersionByteArray)])

        if self._args.unique_id:
            cmd.extend(["--object", self.UNIQUE_ID_NVM3_KEY + self._args.unique_id])

        if self._args.manufacturing_date:
            dateByteArray = bytes(self._args.manufacturing_date, 'utf-8').hex()
            cmd.extend(["--object", self.MANUFACTURING_DATE_NVM3_KEY + str(dateByteArray)])

        if self._args.serial_number:
            serialNumberByteArray = bytes(self._args.serial_number, 'utf-8').hex()
            cmd.extend(["--object", self.SERIAL_NUMBER_NVM3_KEY + str(serialNumberByteArray)])

        if self._args.part_number:
            partNumberByteArray = bytes(self._args.part_number, 'utf-8').hex()
            cmd.extend(["--object", self.PART_NUMBER_NVM3_KEY + str(partNumberByteArray)])

        if self._args.product_label:
            productLabelByteArray = bytes(self._args.product_label, 'utf-8').hex()
            cmd.extend(["--object", self.PRODUCT_LABEL_NVM3_KEY + str(productLabelByteArray)])

        if self._args.product_url:
            productUrlByteArray = bytes(self._args.product_url, 'utf-8').hex()
            cmd.extend(["--object", self.PRODUCT_URL_NVM3_KEY + str(productUrlByteArray)])

        cmd.extend(["--outfile", self.OUT_FILE])
        results = subprocess.run(cmd)

        # A tempfile was create/used, delete it.
        if inputImage == self.TEMP_FILE:
            cmd = ['rm', '-rf', 'tmp_nvm3.s37', ]
            subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)

        results.check_returncode

        # Flash the binary if the device is connected
        if isDeviceConnected:
            cmd = ['commander', 'flash', self.OUT_FILE, ]
            self.add_SerialNo_To_CMD(cmd)
            results = subprocess.run(cmd)


def main():
    def all_int_format(i): return int(i, 0)
    parser = argparse.ArgumentParser(description='EFR32 NVM3 Factory data provider')

    parser.add_argument("--discriminator", type=all_int_format, required=True,
                        help="[int| hex] Provide BLE pairing discriminator.")
    parser.add_argument("--passcode", type=all_int_format, required=True,
                        help="[int | hex] Provide the default PASE session passcode.")
    parser.add_argument("--spake2_iteration", type=all_int_format, required=True,
                        help="[int | hex int] Provide Spake2+ iteration count.")
    parser.add_argument("--spake2_salt", type=str, required=True,
                        help="[string] Provide Spake2+ salt.")
    parser.add_argument("--spake2_verifier", type=str,
                        help="[string] Provide Spake2+ verifier without generating it.")
    parser.add_argument("--gen_spake2p_path", type=str,
                        help="[string] Provide a path to spake2p generator. It can be built from connectedhomeip/src/tools/spake2p")
    parser.add_argument("--mcu_family", type=str,
                        help="[string] mcu Family target. Only need if your board isn't plugged in")
    parser.add_argument("--jtag_serial", type=str,
                        help="[string] Provide the serial number of the jtag if you have more than one board connected")
    parser.add_argument("--product_id", type=all_int_format, default=32773,
                        help="[int | hex int] Provide the product ID")
    parser.add_argument("--vendor_id", type=all_int_format, default=65521,
                        help="[int | hex int] Provide the vendor ID")
    parser.add_argument("--product_name", type=str,
                        help="[string] Provide the product name [optional]")
    parser.add_argument("--vendor_name", type=str,
                        help="[string] Provide the vendor name [optional]")
    parser.add_argument("--hw_version", type=all_int_format,
                        help="[int | hex int] Provide the hardware version value[optional]")
    parser.add_argument("--hw_version_str", type=str,
                        help="[string] Provide the hardware version string[optional]")
    parser.add_argument("--product_label", type=str,
                        help="[string] Provide the product label [optional]")
    parser.add_argument("--product_url", type=str,
                        help="[string] Provide the product url [optional]")
    parser.add_argument("--unique_id", type=str,
                        help="[hex_string] A 128 bits hex string unique id (without 0x) [optional]")
    parser.add_argument("--serial_number", type=str,
                        help="[string] Provide serial number of the device")
    parser.add_argument("--manufacturing_date", type=str,
                        help="[string] Provide Manufacturing date in YYYY-MM-DD format [optional]")
    parser.add_argument("--part_number", type=str,
                        help="[string] Provide part number [optional]")
    parser.add_argument("--commissioning_flow", type=all_int_format, default=0,
                        help="[int| hex] Provide Commissioning Flow: 0=Standard, 1=kUserActionRequired, 2=Custom (Default:Standard)")
    parser.add_argument("--rendezvous_flag", type=all_int_format, default=2,
                        help="[int| hex] Provide Rendez-vous flag: 1=SoftAP, 2=BLE 4=OnNetwork (Default=BLE Only)")

    args = parser.parse_args()
    writer = FactoryDataWriter(args)
    writer.create_nvm3injected_image()


if __name__ == "__main__":
    main()
