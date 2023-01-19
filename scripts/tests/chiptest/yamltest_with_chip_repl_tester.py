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

import functools
import glob
import os
import tempfile

# isort: off

from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin
import chip.FabricAdmin  # Needed before chip.CertificateAuthority

# isort: on

import chip.CertificateAuthority
import chip.native
import click
from chip.ChipStack import *
from chip.yaml.runner import ReplTestRunner
from matter_yamltests.definitions import ParseSource, SpecDefinitions
from matter_yamltests.parser import TestParser

_DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", ".."))
_CLUSTER_XML_DIRECTORY_PATH = os.path.abspath(
    os.path.join(_DEFAULT_CHIP_ROOT, "src/app/zap-templates/zcl/data-model/"))


def _sort_with_global_attribute_first(a, b):
    if a.endswith('global-attributes.xml'):
        return -1
    elif b.endswith('global-attributes.xml'):
        return 1
    elif a > b:
        return 1
    elif a == b:
        return 0
    elif a < b:
        return -1


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
def main(setup_code, yaml_path, node_id):
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
            certificate_authority_manager.activeCaList[0].NewFabricAdmin(vendorId=0xFFF1, fabricId=1)

        ca_list = certificate_authority_manager.activeCaList

        # Creating and commissioning to a single controller to match what is currently done when
        # running.
        dev_ctrl = ca_list[0].adminList[0].NewController()
        dev_ctrl.CommissionWithCode(setup_code, node_id)

        # Creating Cluster definition.
        cluster_xml_filenames = glob.glob(_CLUSTER_XML_DIRECTORY_PATH + '/*/*.xml', recursive=False)
        cluster_xml_filenames.sort(key=functools.cmp_to_key(_sort_with_global_attribute_first))
        sources = [ParseSource(source=name) for name in cluster_xml_filenames]
        clusters_definitions = SpecDefinitions(sources)

        # Parsing YAML test and setting up chip-repl yamltests runner.
        yaml = TestParser(yaml_path, None, clusters_definitions)
        runner = ReplTestRunner(clusters_definitions, certificate_authority_manager, dev_ctrl)

        # Executing and validating test
        for test_step in yaml.tests:
            test_action = runner.encode(test_step)
            # TODO if test_action is None we should see if it is a pseudo cluster.
            if test_action is not None:
                response = runner.execute(test_action)
                decoded_response = runner.decode(response)
                post_processing_result = test_step.post_process_response(decoded_response)
                if not post_processing_result.is_success():
                    exit(-2)
            else:
                exit(-2)

        runner.shutdown()
        # Tearing down chip stack. If not done in the correct order test will fail.
        certificate_authority_manager.Shutdown()
        chip_stack.Shutdown()


if __name__ == '__main__':
    main()
