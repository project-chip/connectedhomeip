# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import pytest
import subprocess
from common.utils import validIPAddress
from time import sleep

import logging
log = logging.getLogger(__name__)

@pytest.mark.networktest
def test_server_udp(connected_device, chip_tools_dir):
    # Enable server
    ret = connected_device.send(command="server on UDP", expected_output="Done", wait_before_read=5)
    assert ret != None and len(ret) > 1

    # Get device IP address
    ip_address = ret[-2].rstrip().split()[-1]
    assert validIPAddress(ip_address) != "Invalid"

    process = subprocess.Popen(['./chip-echo-requester', ip_address], cwd=chip_tools_dir)
    process.wait()
    assert process.returncode == 1

    ret = connected_device.wait_for_output("Echo Message")
    assert ret != None

    # Disable server
    ret = connected_device.send(command="server off", expected_output="Done", wait_before_read=5)
    assert ret != None and len(ret) > 1

# Not supported
# @pytest.mark.networktest
# def test_server_tcp(connected_device, chip_tools_dir):
#     # Enable server
#     ret = connected_device.send(command="server on TCP", expected_output="Done", wait_before_read=5)
#     assert ret != None and len(ret) > 1

#     # Get device IP address
#     ip_address = ret[-2].rstrip().split()[-1]
#     assert validIPAddress(ip_address) != "Invalid"

#     process = subprocess.Popen(['./chip-echo-requester', ip_address, '--tcp'], cwd=chip_tools_dir)
#     process.wait()
#     assert process.returncode == 1

#     ret = connected_device.wait_for_output("Echo Message")
#     assert ret != None

#     # Disable server
#     ret = connected_device.send(command="server off", expected_output="Done", wait_before_read=5)
#     assert ret != None and len(ret) > 1