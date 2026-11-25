#
#    Copyright (c) 2026 Project CHIP Authors
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

import os
import tempfile
import unittest

from matter.storage import PersistentStorageINI, PersistentStorageJSON


class TestCaKeysBackwardCompatibilityRewrite(unittest.TestCase):
    def test_ipk_epoch_key_is_renumbered_with_ca_keys(self):
        sdkData = {
            'ExampleOpCredsCAKey1': 'root-key',
            'ExampleOpCredsICAKey1': 'intermediate-key',
            'ExampleCARootCert1': 'root-cert',
            'ExampleCAIntermediateCert1': 'intermediate-cert',
            'ExampleOpCredsIPK1': 'ipk',
            'unrelated': 'value',
        }
        data = {'caList': {'1': []}}

        # The rewrite reads only its arguments, so no storage instance (and no native library) is needed.
        data, sdkData = PersistentStorageJSON._caKeysBackwardCompatibilityRewrite(None, data, sdkData)

        self.assertEqual(data['caList'], {'0': []})
        self.assertEqual(sdkData, {
            'ExampleOpCredsCAKey0': 'root-key',
            'ExampleOpCredsICAKey0': 'intermediate-key',
            'ExampleCARootCert0': 'root-cert',
            'ExampleCAIntermediateCert0': 'intermediate-cert',
            'ExampleOpCredsIPK0': 'ipk',
            'unrelated': 'value',
        })


class TestChipToolFabricStorageCommit(unittest.TestCase):
    def test_ipk_epoch_key_is_written_to_chip_tool_fabric_storage(self):
        with tempfile.TemporaryDirectory() as tmp:
            replPath = os.path.join(tmp, 'repl.ini')
            fabricPath = os.path.join(tmp, 'chip_tool_fabric.ini')

            # Skip __init__, which needs the native library; Commit() reads only these attributes.
            storage = PersistentStorageINI.__new__(PersistentStorageINI)
            storage._path = replPath
            storage._chipToolFabricStoragePath = fabricPath
            storage._data = {}
            storage._sdkData = {
                'ExampleOpCredsCAKey0': 'root-key',
                'ExampleOpCredsIPK0': 'ipk',
                'unrelated': 'value',
            }

            storage.Commit()

            fabric = PersistentStorageINI.ConfigParser()
            fabric.read(fabricPath)
            self.assertEqual(dict(fabric.items('Default')), {
                'ExampleOpCredsCAKey0': 'root-key',
                'ExampleOpCredsIPK0': 'ipk',
            })

            repl = PersistentStorageINI.ConfigParser()
            repl.read(replPath)
            self.assertEqual(dict(repl.items('Default')), {'unrelated': 'value'})


if __name__ == '__main__':
    unittest.main()
