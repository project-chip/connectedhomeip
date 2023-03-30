#!/usr/bin/env -S python3 -B

#    Copyright (c) 2022 Project CHIP Authors
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

import asyncio
import atexit
import logging
import os
import tempfile
import traceback

# isort: off

from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin
import chip.FabricAdmin  # Needed before chip.CertificateAuthority

# isort: on

# ensure matter IDL is availale for import, otherwise set relative paths
try:
    from matter_idl import matter_idl_types
except:
    SCRIPT_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
    import sys

    sys.path.append(os.path.join(SCRIPT_PATH, 'py_matter_idl'))
    sys.path.append(os.path.join(SCRIPT_PATH, 'py_matter_yamltests'))

    from matter_idl import matter_idl_types


import chip.CertificateAuthority
import chip.native
import click
from chip.ChipStack import *
from chip.yaml.runner import ReplTestRunner
from matter_yamltests.definitions import SpecDefinitionsFromPaths
from matter_yamltests.parser import PostProcessCheckStatus, TestParser, TestParserConfig

_DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", ".."))
_CLUSTER_XML_DIRECTORY_PATH = os.path.abspath(
    os.path.join(_DEFAULT_CHIP_ROOT, "src/app/zap-templates/zcl/data-model/"))


async def execute_test(yaml, runner):
    # Executing and validating test
    for test_step in yaml.tests:
        if not test_step.is_pics_enabled:
            continue
        test_action = runner.encode(test_step)
        if test_action is None:
            raise Exception(
                f'Failed to encode test step {test_step.label}')

        response = await runner.execute(test_action)
        decoded_response = runner.decode(response)
        post_processing_result = test_step.post_process_response(
            decoded_response)
        if not post_processing_result.is_success():
            logging.warning(f"Test step failure in 'test_step.label'")
            for entry in post_processing_result.entries:
                if entry.state == PostProcessCheckStatus.SUCCESS:
                    continue
                logging.warning("%s: %s", entry.state, entry.message)
            raise Exception(f'Test step failed {test_step.label}')


@click.command()
@click.option(
    '--setup-code',
    default=None,
    help='setup-code')
@click.option(
    '--yaml-path',
    default=None,
    help='yaml-path')
@click.option(
    '--node-id',
    default=0x12344321,
    help='Node ID to use when commissioning device')
@click.option(
    '--pics-file',
    default=None,
    help='Optional PICS file')
def main(setup_code, yaml_path, node_id, pics_file):
    # Setting up python environment for running YAML CI tests using python parser.
    with tempfile.NamedTemporaryFile() as chip_stack_storage:
        chip.native.Init()
        chip_stack = ChipStack(chip_stack_storage.name)
        certificate_authority_manager = chip.CertificateAuthority.CertificateAuthorityManager(
            chip_stack, chip_stack.GetStorageManager())
        certificate_authority_manager.LoadAuthoritiesFromStorage()

        if len(certificate_authority_manager.activeCaList) == 0:
            ca = certificate_authority_manager.NewCertificateAuthority()
            ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=1)
        elif len(certificate_authority_manager.activeCaList[0].adminList) == 0:
            certificate_authority_manager.activeCaList[0].NewFabricAdmin(
                vendorId=0xFFF1, fabricId=1)

        ca_list = certificate_authority_manager.activeCaList

        # Creating and commissioning to a single controller to match what is currently done when
        # running.
        dev_ctrl = ca_list[0].adminList[0].NewController()
        dev_ctrl.CommissionWithCode(setup_code, node_id)

        def _StackShutDown():
            # Tearing down chip stack. If not done in the correct order test will fail.
            certificate_authority_manager.Shutdown()
            chip_stack.Shutdown()

        atexit.register(_StackShutDown)

        try:
            # Creating Cluster definition.
            clusters_definitions = SpecDefinitionsFromPaths([
                _CLUSTER_XML_DIRECTORY_PATH + '/chip/*.xml',
            ])

            # Parsing YAML test and setting up chip-repl yamltests runner.
            parser_config = TestParserConfig(pics_file, clusters_definitions)
            yaml = TestParser(yaml_path, parser_config)
            runner = ReplTestRunner(
                clusters_definitions, certificate_authority_manager, dev_ctrl)

            asyncio.run(execute_test(yaml, runner))

        except Exception:
            print(traceback.format_exc())
            exit(-2)

        runner.shutdown()


if __name__ == '__main__':
    main()
