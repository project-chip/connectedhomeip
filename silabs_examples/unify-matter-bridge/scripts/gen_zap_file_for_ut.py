#!/usr/bin/env python3
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

def copy_and_replace(input_path, output_path):
    try:
        with open(input_path, 'r') as input_file:
            content = input_file.read()
            
            # Replace the specified content for "path"
            content = content.replace(
                '"path": "../../../src/app/zap-templates/zcl/zcl.json",',
                '"path": "../../../../../../src/app/zap-templates/zcl/zcl.json",'
            )
            
            # Replace the specified content for "path"
            content = content.replace(
                '"path": "../../../src/app/zap-templates/app-templates.json",',
                '"path": "../../../../../../src/app/zap-templates/app-templates.json",'
            )
            
            # Replace the specified content to disable Time Synchronization
            content = content.replace(
                '"define": "TIME_SYNCHRONIZATION_CLUSTER",\n          "side": "server",\n          "enabled": 1,',
                '"define": "TIME_SYNCHRONIZATION_CLUSTER",\n          "side": "server",\n          "enabled": 0,'
            )
            
        with open(output_path, 'w') as output_file:
            output_file.write(content)
            
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    
    # Specify the input and output file paths
    input_file_path = 'silabs_examples/unify-matter-bridge/unify-matter-bridge-common/unify-matter-bridge.zap'
    output_file_path = 'silabs_examples/unify-matter-bridge/linux/src/tests/unify-matter-bridge-ut/unify-matter-bridge-ut.zap'

    if not os.path.exists(input_file_path):
        print(f"Error: Input file not found - {input_file_path}")

    # Copy and replace content
    copy_and_replace(input_file_path, output_file_path)
