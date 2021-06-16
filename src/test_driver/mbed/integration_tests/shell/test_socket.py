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
import logging
log = logging.getLogger(__name__)

SOCKET_ECHO_TEST_MESSAGE = "Test_message"

@pytest.mark.networktest
def test_socket_example_check(connected_device):
    # Get network interface
    ret = connected_device.send(command="socket example", expected_output="Done")
    assert ret != None


@pytest.mark.networktest
def test_socket_client_echo(connected_device, echo_server):
    server_ip = echo_server[0]
    server_port = echo_server[1]

    # UDP
    ret = connected_device.send(command="socket client UDP {} {} {} wait".format(server_ip, server_port, SOCKET_ECHO_TEST_MESSAGE), expected_output="Done", wait_before_read=5)
    assert ret != None and len(ret) > 2
    assert ret[-3].rstrip() == SOCKET_ECHO_TEST_MESSAGE

    # TCP
    ret = connected_device.send(command="socket client TCP {} {} {} wait".format(server_ip, server_port, SOCKET_ECHO_TEST_MESSAGE), expected_output="Done", wait_before_read=5)
    assert ret != None and len(ret) > 2
    assert ret[-3].rstrip() == SOCKET_ECHO_TEST_MESSAGE