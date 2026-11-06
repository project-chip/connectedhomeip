# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import unittest
from unittest.mock import patch

from matter.testing.linux import namespace


class TestNamespaceAvailability(unittest.TestCase):
    def test_root_reexecutes_in_private_mount_namespace(self):
        with patch.object(namespace.os, "getuid", return_value=0), \
                patch.object(namespace.os, "execvpe") as execvpe, \
                patch.object(namespace.sys, "argv", ["runner.py", "--script", "test.py"]):
            namespace.ensure_namespace_availability()
            execvpe.assert_not_called()
            namespace.ensure_namespace_availability(isolate_root=True)

        execvpe.assert_called_once_with(
            "unshare",
            ["unshare", "--mount", "--propagation", "private", namespace.sys.executable,
             "runner.py", "--internal-inside-unshare", "--script", "test.py"],
            namespace.test_environ,
        )

    def test_unprivileged_user_also_gets_root_mapping(self):
        with patch.object(namespace.os, "getuid", return_value=1000), \
                patch.object(namespace.os, "execvpe") as execvpe, \
                patch.object(namespace.sys, "argv", ["runner.py"]):
            namespace.ensure_namespace_availability()

        execvpe.assert_called_once_with(
            "unshare",
            ["unshare", "--mount", "--propagation", "private", "--map-root-user",
             namespace.sys.executable, "runner.py", "--internal-inside-unshare"],
            namespace.test_environ,
        )

    def test_failed_isolation_stops_execution(self):
        with patch.object(namespace.os, "getuid", return_value=0), \
                patch.object(namespace.os, "execvpe", side_effect=PermissionError("unshare denied")), \
                self.assertRaises(PermissionError):
            namespace.ensure_namespace_availability(isolate_root=True)


if __name__ == "__main__":
    unittest.main()
