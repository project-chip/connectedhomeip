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

import subprocess
import sys
import argparse
import struct

class FactoryDataWriter:
    # CONSTANTS
    TEMP_FILE = 'tmp_nvm3.s37'
    OUT_FILE = 'matter_factorydata.s37'  # Final output file containing the nvm3 data to flash to the device
    # nvm3 keys to set
    PASSCODE_NVM3_KEY = "0x87205:"
    DISCRIMINATOR_NVM3_KEY = "0x87207:"
    ITERATIONCOUNT_NVM3_KEY = "0x87208:"
    SALT_NVM3_KEY = "0x87209:"
    VERIFIER_NVM3_KEY = "0x8720A:"

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

    def __init__(self, arguments) -> None:
        """ Do some checks on the received arguments.
            Generate the Spake2+ verifier if needed and assign the values
            to the global variables

        Args:
            The whole set of args passed to the script.
        """
        INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444,
                            55555555, 66666666, 77777777, 88888888, 99999999, 12345678, 87654321]

        assert (bool(arguments.gen_spake2p_path) != bool(arguments.spake2_verifier)
                ), "Provide either the spake2_verifier string or the path to the spake2 generator"
        assert not (arguments.passcode in INVALID_PASSCODES), "The provided passcode is invalid"

        self._args=arguments

        if self._args.gen_spake2p_path:
            self._args.spake2_verifier = self.generate_spake2p_verifier()


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
        print("passcode :", self._args.passcode)
        print("discriminator :",self._args.discriminator)
        print("spake2_iteration :", self._args.spake2_iteration)
        print("spake2_salt :",  self._args.spake2_salt)
        print("spake2_verifier :",  self._args.spake2_verifier)

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
                    inputImage = "base_matter_mg12_nvm3.s37"
                elif "EFR32MG24" in deviceInfo["Part Number"]:
                    inputImage = "base_matter_mg24_nvm3.s37"
                else:
                    raise Exception('Invalid MCU')
            except:
                isDeviceConnected = False
                print("Device not connected")
                # When no device is connected user needs to provide the mcu family for which those credentials are to be created
                if self._args.mcu_family:
                    if "EFR32MG12" in deviceInfo["Part Number"]:
                        inputImage = "matter_mg12_nvm3.s37"
                    elif "EFR32MG24" in deviceInfo["Part Number"]:
                        inputImage = "matter_mg24_nvm3.s37"
                else:
                    print("Connect debug port or provide the mcu_family")
                    return


        # Convert interger to little endian hex format and strings to hex byte array format for nvm3 storage
        Passcode = self._args.passcode.to_bytes(4, "little").hex()
        Spake2pIterationCount = self._args.spake2_iteration.to_bytes(4, 'little').hex()
        Discriminator = self._args.discriminator.to_bytes(2, 'little').hex()
        SaltByteArray = bytes(self._args.spake2_salt, 'utf-8').hex()
        VerifierByteArray = bytes(self._args.spake2_verifier, 'utf-8').hex()

        # create the binary containing the new nvm3 data
        results = subprocess.run(
            [
                "commander", "nvm3", "set", inputImage,
                "--object", self.DISCRIMINATOR_NVM3_KEY + str(Discriminator),
                "--object", self.PASSCODE_NVM3_KEY + str(Passcode),
                "--object", self.ITERATIONCOUNT_NVM3_KEY + str(Spake2pIterationCount),
                "--object", self.SALT_NVM3_KEY + str(SaltByteArray),
                "--object", self.VERIFIER_NVM3_KEY + str(VerifierByteArray),
                "--outfile", self.OUT_FILE,
            ]
        )

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

    args = parser.parse_args()
    writer = FactoryDataWriter(args)
    writer.create_nvm3injected_image()


if __name__ == "__main__":
    main()
