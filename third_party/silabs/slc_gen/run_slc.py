# Importing required module
import glob
import os
import subprocess
import sys

if len(sys.argv) != 9:
    print("wrong number of arguments")
    sys.exit(1)


def asBoolean(valueToTest):
    return ("true" == valueToTest)


def isMG24(partnumber):
    if "EFR32MG24" in partnumber or "MGM240" in partnumber:
        return True
    else:
        return False


root_path = sys.argv[1]
silabs_board = str(sys.argv[2]).lower()
disable_lcd = asBoolean(sys.argv[3])
use_wstk_buttons = asBoolean(sys.argv[4])
use_wstk_leds = asBoolean(sys.argv[5])
use_external_flash = asBoolean(sys.argv[6])
silabs_mcu = str(sys.argv[7])
output_path = str(sys.argv[8])

slcp_file_path = os.path.join(root_path, "examples/platform/silabs/matter-platform.slcp")
template_path = os.path.join(root_path, "third_party/silabs/slc_gen/")

slc_arguments = ""

# Add Familly specific component
if isMG24(silabs_mcu):
    slc_arguments += "uartdrv_eusart:vcom,"
else:
    slc_arguments += "uartdrv_usart:vcom,"

# Translate GN arguments in SLC arguments
if not disable_lcd:
    slc_arguments += "memlcd_usart,dmd_memlcd,"
if use_wstk_buttons:
    slc_arguments += "simple_button:btn0:btn1,"
if use_wstk_leds:
    # Sparkfun board
    if silabs_board == "brd2704a":
        slc_arguments += "simple_led:led0,"
    else:
        slc_arguments += "simple_led:led0:led1,"
if use_external_flash:
    slc_arguments += "mx25_flash_shutdown_usart,"

slc_arguments += silabs_board

print(slc_arguments)

if "GSDK_ROOT" in os.environ:
    gsdk_root = os.getenv('GSDK_ROOT')
else:
    # If no gsdk path is set in the environment, use the standard path to the submodule
    gsdk_root = os.path.join(root_path, "third_party/silabs/gecko_sdk/")

# make sure we have a configured and trusted gsdk in slc
subprocess.run(["slc", "configuration", "--sdk", gsdk_root], check=True)
subprocess.run(["slc", "signature", "trust", "--sdk", gsdk_root], check=True)

subprocess.run(["slc", "generate", slcp_file_path, "-d", output_path, "--with", slc_arguments], check=True)

# cleanup of unwanted files
fileList = glob.glob(os.path.join(output_path, "matter-platform.*"))
for filePath in fileList:
    try:
        os.remove(filePath)
    except OSError:
        print("failed to remove file: {}".format(filePath))

try:
    os.remove(os.path.join(output_path, "vscode.conf"))
except OSError:
    print("failed to remove file: {}".format(filePath))


# Create a Build.gn files with all of the sources
source_file_path = "    \"autogen/{fileName}\",\r\n"

template = open(os.path.join(template_path, "buildgn_template.txt"), "r")
final = open(os.path.join(output_path, "BUILD.gn"), "w")
for line in template:
    if "//REPLACE_ME" in line:
        for file in os.listdir(os.path.join(output_path, 'autogen/')):
            if file.endswith(".c"):
                # print path name of selected files
                final.write(source_file_path.format(fileName=file))
    else:
        final.write(line)

template.close()
final.close()
