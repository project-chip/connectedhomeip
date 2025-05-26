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

from chip.testing.matter_testing import parse_matter_test_args, run_tests_return_results, MatterBaseTest, MatterStackState

import asyncio
import importlib
import logging
import os
import sys
from pathlib import Path
from mobly import utils


def main():
    # TODO: glob
    case_list = ['TC_DeviceBasicComposition', 'TC_DeviceConformance',
                 'TC_FAN_3_2', 'TC_VALCC_4_5', 'TC_CNET_4_3', 'TC_DRLK_2_5']

    test_classes = []
    matter_test_config = parse_matter_test_args()
    stack = MatterStackState(matter_test_config)
    default_controller = stack.certificate_authorities[0].adminList[0].NewController(
        nodeId=matter_test_config.controller_node_id,
        paaTrustStorePath=str(matter_test_config.paa_trust_store_path),
        catTags=matter_test_config.controller_cat_tags,
        dacRevocationSetPath=str(matter_test_config.dac_revocation_set_path),
    )
    for filename in case_list:
        try:
            module = importlib.import_module(Path(os.path.basename(filename)).stem)
        except ModuleNotFoundError:
            logging.error(f'Unable to load python module from {filename}. Please ensure this is a valid python file path')
            return -1

        try:
            subclasses = utils.find_subclasses_in_module([MatterBaseTest], module)
            subclasses = [c for c in subclasses if c.__name__ != "MatterBaseTest"]
            if len(subclasses) != 1:
                print(
                    'Exactly one subclass of `MatterBaseTest` should be in the main file. Found %s.' %
                    str([subclass.__name__ for subclass in subclasses]))
                sys.exit(1)

            test_classes.append(subclasses[0])
        except AttributeError:
            logging.error(f'Unable to load the test class {filename}. Please ensure this class is implemented in {filename}')
            return -1

    matter_test_config.global_test_params["dry_run"] = True
    with asyncio.Runner() as runner:
        _, requirements, results = run_tests_return_results(test_classes=test_classes, matter_test_config=matter_test_config,
                                                            event_loop=runner.get_loop(), default_controller=default_controller, external_stack=stack)
    matter_test_config.global_test_params["dry_run"] = False
    # Going to do this by endpoint so we have separate results files
    endpoints = set()
    for t in requirements:
        if not t.applicable_endpoints:
            continue
        endpoints.update(t.applicable_endpoints)

    # This is just an optimization, we can run them all, but it's longer
    test_classes_per_endpoint = {}
    for e in endpoints:
        test_classes_per_endpoint[e] = list(set([t.test_class for t in requirements if e in t.applicable_endpoints]))

    pass_fail = {}

    for e in endpoints:
        print(f"Endpoint {e}")
        print(test_classes_per_endpoint[e])
        with asyncio.Runner() as runner:
            matter_test_config.endpoint = e
            _, requirements, results = run_tests_return_results(test_classes=test_classes_per_endpoint[e], matter_test_config=matter_test_config,
                                                                event_loop=runner.get_loop(), default_controller=default_controller, external_stack=stack)
            pass_fail[e] = results.summary_str()

    for e, result in pass_fail.items():
        print(f'{e}: {result}')
    stack.Shutdown()


if __name__ == "__main__":
    sys.exit(main())
