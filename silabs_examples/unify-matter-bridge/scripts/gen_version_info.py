# ******************************************************************************
# License
# <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
# ******************************************************************************
# The licensor of this software is Silicon Laboratories Inc. Your use of this
# software is governed by the terms of Silicon Labs Master Software License
# Agreement (MSLA) available at
# www.silabs.com/about-us/legal/master-software-license-agreement. This
# software is distributed to you in Source Code format and is governed by the
# sections of the MSLA applicable to Source Code.
#
#*****************************************************************************/

import os
import subprocess

def generate_git_info(output_file):
    try:
        version = subprocess.check_output(['git', 'describe', '--tags'], text=True, stderr=subprocess.DEVNULL).strip().split('-', 1)[1]
        sha = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD'], text=True, stderr=subprocess.DEVNULL).strip()
        
    except subprocess.CalledProcessError:
        version = "1.2"  # Fallback version string
        sha = ""      # Empty SHA string

        try:
            # Get version and sha from command2 and append to fallback values
            version_output = subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], text=True, stderr=subprocess.DEVNULL).strip()
            sha_output = subprocess.check_output(['git', 'describe', '--always', '--dirty', '--exclude', '*'], text=True, stderr=subprocess.DEVNULL).strip()

            version += "_" + version_output
            sha += sha_output

        except subprocess.CalledProcessError:
            pass  # Keep the fallback values as they are

    with open(output_file, "w") as f:
        f.write("#ifndef GIT_INFO_H\n")
        f.write("#define GIT_INFO_H\n\n")
        f.write(f"#define CHIP_VERSION \"{version}\"\n")
        f.write(f"#define CHIP_SHA \"{sha}\"\n\n")
        f.write("#endif // GIT_INFO_H\n")

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print(" Usage: gen_version_info.py <output_file> ")
        sys.exit(1)
    
    output_file = sys.argv[1]
    if not os.path.exists(output_file):
        os.makedirs(os.path.dirname(output_file), exist_ok=True)
    generate_git_info(output_file)
