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

from chip.testing.matter_testing import parse_matter_test_args, run_tests_no_exit

import asyncio
import importlib
import logging
import os
import sys
from pathlib import Path


def main():
    # TODO: glob
    case_list = ['TC_DeviceBasicComposition', 'TC_DeviceConformance',
                 'TC_FAN_3_2', 'TC_VALCC_4_5', 'TC_CNET_4_3', 'TC_DRLK_2_5']
    for file in case_list:
        cmd = f'python src/python_testing/{file} --storage-path admin_storage.json --bool-arg test_case_list:True'
        # subprocess.run(cmd, shell=True)

    test_classes = []
    matter_test_config = parse_matter_test_args()
    for filename in case_list:
        try:
            module = importlib.import_module(Path(os.path.basename(filename)).stem)
        except ModuleNotFoundError:
            logging.error(f'Unable to load python module from {filename}. Please ensure this is a valid python file path')
            return -1

        try:
            # TODO: find the real class by checking if this is a MatterBaseTest class
            # For now we assume it's the same name, that's clearly not true, we'll refine later
            test_classes.append(getattr(module, filename))
        except AttributeError:
            logging.error(f'Unable to load the test class {filename}. Please ensure this class is implemented in {filename}')
            return -1

    matter_test_config.global_test_params["dry_run"] = True
    with asyncio.Runner() as runner:
        run_tests_no_exit(test_classes=test_classes, matter_test_config=matter_test_config,
                          event_loop=runner.get_loop())


if __name__ == "__main__":
    sys.exit(main())
