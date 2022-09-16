import subprocess
import sys
import argparse
import struct

# global variables
Discriminator_ui = 0
Passcode_ui = 0
IterationCount_ui = 0
Salt_str = ""
Verifier_str = ""
mcuFamily = ""
jtagSerial = ""
isDeviceConnected = True

# CONSTANTS
TEMP_FILE = 'tmp_nvm3.s37'
OUT_FILE = 'matter_factorydata.s37'  # Final output file containing the nvm3 data to flash to the device
# nvm3 keys to set
PASSCODE_NVM3_KEY = "0x87205:"
DISCRIMINATOR_NVM3_KEY = "0x87207:"
ITERATIONCOUNT_NVM3_KEY = "0x87208:"
SALT_NVM3_KEY = "0x87209:"
VERIFIER_NVM3_KEY = "0x8720A:"


def generate_spake2p_verifier(args):
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
        args.gen_spake2p_path, 'gen-verifier',
        '--iteration-count', str(args.spake2_iteration),
        '--salt', args.spake2_salt,
        '--pin-code', str(args.passcode),
        '--out', '-',
    ]
    output = subprocess.check_output(cmd)
    output = output.decode('utf-8').splitlines()
    generation_results = dict(zip(output[0].split(','), output[1].split(',')))
    return generation_results["Verifier"]


def processArguments(args):
    """ Do some checks on the received arguments.
        Generate the Spake2+ verifier if needed and assign the values
        to the global variables

    Args:
        The whole set of args passed to the script.
    """
    global Discriminator_ui
    global Passcode_ui
    global IterationCount_ui
    global Salt_str
    global Verifier_str
    global mcuFamily
    global jtagSerial

    INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444,
                         55555555, 66666666, 77777777, 88888888, 99999999, 12345678, 87654321]

    assert (bool(args.gen_spake2p_path) != bool(args.spake2_verifier)
            ), "Provide either the spake2_verifier string or the path to the spake2 generator"
    assert not (args.passcode in INVALID_PASSCODES), "The provided passcode is invalid"

    print("passcode :", args.passcode)
    print("discriminator :", args.discriminator)
    print("spake2_iteration :", args.spake2_iteration)
    print("spake2_salt :",  args.spake2_salt)
    print("spake2_verifier :",  args.spake2_verifier)

    Passcode_ui = args.passcode
    Discriminator_ui = args.discriminator
    IterationCount_ui = args.spake2_iteration
    Salt_str = args.spake2_salt

    if args.spake2_verifier:
        Verifier_str = args.spake2_verifier
    elif args.gen_spake2p_path:
        Verifier_str = generate_spake2p_verifier(args)

    if args.mcu_family:
        mcuFamily = args.mcu_family

    if args.jtag_serial:
        jtagSerial = args.jtag_serial


def add_SerialNo_To_CMD(cmdList):
    """ Add the jtag serial command to the commander command

        Args:
        The commander command in list format
    """
    if jtagSerial:
        cmdList.extend(["--serialno", jtagSerial])


def create_nvm3injected_image():
    """ Use commander command lines create a binary flashable to the EFR32 
        containing the factory commissioning data in NVM3 section
    """
    global isDeviceConnected

    # Retrieve the device current nvm3 data in a binary file
    # It will be used as base to add the new credentials
    inputImage = TEMP_FILE
    cmd = ['commander', 'nvm3', 'read', '-o', inputImage, ]
    add_SerialNo_To_CMD(cmd)
    results = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)

    if results.returncode != 0:
        # No nvm3 section found. Retrieve the device info
        cmd = ['commander', 'device', 'info', ]
        add_SerialNo_To_CMD(cmd)
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
            if mcuFamily:
                if "EFR32MG12" in deviceInfo["Part Number"]:
                    inputImage = "matter_mg12_nvm3.s37"
                elif "EFR32MG24" in deviceInfo["Part Number"]:
                    inputImage = "matter_mg24_nvm3.s37"
            else:
                print("Connect debug port or provide the mcu_family")
                return

    # Convert interger to little endian hex format and strings to hex byte array format for nvm3 storage
    Passcode = Passcode_ui.to_bytes((Passcode_ui.bit_length() + 7) // 8, "little").hex()
    Spake2pIterationCount = IterationCount_ui.to_bytes((IterationCount_ui.bit_length() + 7) // 8, 'little').hex()
    Discriminator = Discriminator_ui.to_bytes((Discriminator_ui.bit_length() + 7) // 8, 'little').hex()
    SaltByteArray = bytes(Salt_str, 'utf-8').hex()
    VerifierByteArray = bytes(Verifier_str, 'utf-8').hex()

    # create the binary containing the new nvm3 data
    results = subprocess.run(
        [
            "commander", "nvm3", "set", inputImage,
            "--object", DISCRIMINATOR_NVM3_KEY + str(Discriminator),
            "--object", PASSCODE_NVM3_KEY + str(Passcode),
            "--object", ITERATIONCOUNT_NVM3_KEY + str(Spake2pIterationCount),
            "--object", SALT_NVM3_KEY + str(SaltByteArray),
            "--object", VERIFIER_NVM3_KEY + str(VerifierByteArray),
            "--outfile", OUT_FILE,
        ]
    )

    # A tempfile was create/used, delete it.
    if inputImage == TEMP_FILE:
        cmd = ['rm', '-rf', 'tmp_nvm3.s37', ]
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)

    results.check_returncode

    # Flash the binary if the device is connected
    if isDeviceConnected:
        cmd = ['commander', 'flash', OUT_FILE, ]
        add_SerialNo_To_CMD(cmd)
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
    processArguments(args)
    create_nvm3injected_image()


if __name__ == "__main__":
    main()
