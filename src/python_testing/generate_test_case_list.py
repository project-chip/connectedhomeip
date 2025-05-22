#
#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
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

import subprocess
import importlib
import logging
import os
from pathlib import Path


def main():
    # TODO: glob
    case_list = ['TC_DeviceBasicComposition.py', 'TC_DeviceConformance.py',
                 'TC_FAN_3_2.py', 'TC_VALCC_4_5.py', 'TC_CNET_4_3.py', 'TC_DRLK_2_5.py']
    for file in case_list:
        cmd = f'python src/python_testing/{file} --storage-path admin_storage.json --bool-arg test_case_list:True'
        # subprocess.run(cmd, shell=True)

    for filename in case_list:
        try:
            module = importlib.import_module(Path(os.path.basename(filename)).stem)
        except ModuleNotFoundError:
            logging.error(f'Unable to load python module from {filename}. Please ensure this is a valid python file path')
            return -1

        try:
            # TODO: find the real class by checking if this is a MatterBaseTest class
            # For now we assume it's
            test_class = getattr(module, filename)
        except AttributeError:
            logging.error(f'Unable to load the test class {filename}. Please ensure this class is implemented in {filename}')


if __name__ == "__main__":
    main()
