# Copyright (c) 2021 Project CHIP Authors
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

import time

from chip_mobly import pigweed_device
from mobly import asserts, base_test, test_runner


class PigweedEchoTest(base_test.BaseTestClass):
    def setup_class(self):
        ''' Registering pigweed_device controller module declares the test's
        dependency on CHIP/Pigweed device hardware. By default, we expect at least one
        object is created from this.'''
        self.ads = self.register_controller(pigweed_device)
        self.dut = self.ads[0]
        # Flashes the image passed in the configuration yml.
        self.dut.platform.flash()
        time.sleep(1)  # give the device time to boot and register rpcs

    def test_hello(self):
        ''' Tests EchoService.Echo '''
        expected = "hello!"
        status, payload = self.dut.rpcs().EchoService.Echo(msg=expected)
        asserts.assert_true(status.ok(), "Status is %s" % status)
        asserts.assert_equal(
            payload.msg,
            expected,
            'Returned payload is "%s" expected "%s"' % (payload.msg, expected),
        )


if __name__ == "__main__":
    test_runner.main()
