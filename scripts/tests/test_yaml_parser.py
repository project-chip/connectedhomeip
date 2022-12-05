#
#    Copyright (c) 2022 Project CHIP Authors
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

# TODO Once yamltest is a proper self contained module we can move this file
# to a more appropriate spot. For now, having this file to do some quick checks
# is arguably better then no checks at all.

import os
import unittest
from pathlib import Path

from yamltests.ClustersDefinitions import ClustersDefinitions
from yamltests.YamlParser import YamlParser

_DEFAULT_MATTER_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))
_YAML_TEST_SUITE_PATH = os.path.abspath(
    os.path.join(_DEFAULT_MATTER_ROOT, 'src/app/tests/suites'))

_CLUSTER_DEFINITION_DIRECTORY = os.path.abspath(
    os.path.join(_DEFAULT_MATTER_ROOT, 'src/app/zap-templates/zcl/data-model'))


class TestYamlParser(unittest.TestCase):
    def setUp(self):
        # TODO we should not be reliant on an external YAML file. Possible that we should have
        # either a local .yaml testfile, of the file contents should exist in this file where we
        # write out the yaml file to temp directory for our testing use. This approach was taken
        # since this test is better than no test.
        yaml_test_suite_path = Path(_YAML_TEST_SUITE_PATH)
        if not yaml_test_suite_path.exists():
            raise FileNotFoundError(f'Expected directory {_YAML_TEST_SUITE_PATH} to exist')
        yaml_test_filename = 'TestCluster.yaml'
        path_to_test = None
        for path in yaml_test_suite_path.rglob(yaml_test_filename):
            if not path.is_file():
                continue
            if path.name != yaml_test_filename:
                continue
            path_to_test = str(path)
            break
        if path_to_test is None:
            raise FileNotFoundError(f'Could not file {yaml_test_filename} in directory {_YAML_TEST_SUITE_PATH}')
        pics_file = None

        # TODO Again we should not be reliant on extneral XML files. But some test (even brittal)
        # are better than no tests.

        clusters_definitions = ClustersDefinitions(_CLUSTER_DEFINITION_DIRECTORY)

        self._yaml_parser = YamlParser(path_to_test, pics_file, clusters_definitions)

    def test_able_to_iterate_over_all_tests(self):
        # self._yaml_parser.tests implements `__next__`, which does value substitution. We are
        # simply ensure there is no exceptions raise.
        for idx, test_step in enumerate(self._yaml_parser.tests):
            pass
        self.assertTrue(True)


def main():
    unittest.main()


if __name__ == '__main__':
    main()
