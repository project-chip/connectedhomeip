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
from common.utils import validIPAddress
import logging
log = logging.getLogger(__name__)

def get_network_interface_details(response):
    netif_set = []

    for line in response:
        if "interface id" in line:
            netif_detials = {}
            params = line.split(",")
            netif_detials["id"] = int(params[0].split(":")[1])
            netif_detials["name"] = params[1].split(":")[1].replace(' ', '')
            netif_detials["state"] = params[2].split(":")[1].replace(' ', '')
            netif_set.append(netif_detials)

    return netif_set

def get_network_interface_address(response):
    netif_addr_set = []

    for index, line in enumerate(response):
        if "interface address" in line:
            netif_detials = {}
            params = line.split(",")
            netif_detials["ip"] = params[0].split(":")[1].split("/")[0].replace(' ', '')
            prev_line = response[index -1]
            params = prev_line.split(",")
            netif_detials["id"] = int(params[0].split(":")[1])
            netif_addr_set.append(netif_detials)

    return netif_addr_set

@pytest.mark.networktest
def test_network_interface_check(device):
    # Get network interface
    ret = device.send(command="network interface", expected_output="Done")
    assert ret != None and len(ret) > 2

    netif_set = get_network_interface_details(ret)

    if not netif_set:
        return
    
    for netif in netif_set:
        log.info("Interface id: {}, name {}, state {}".format(netif["id"], netif["name"], netif["state"]))

    # Check idToName
    ret = device.send(command="network idToName {}".format(netif_set[0]["id"]), expected_output="Done")
    assert ret != None and len(ret) > 1
    if_name = ret[-2].rstrip().split(":")[1].replace(' ', '')
    assert netif_set[0]["name"] == if_name

    ret = device.send(command="network idToName {}".format(len(netif_set) + 1), expected_output="Error network")
    assert ret != None

    # Check nameToId
    ret = device.send(command="network nameToId {}".format(netif_set[0]["name"]), expected_output="Done")
    assert ret != None and len(ret) > 1
    if_id = int(ret[-2].rstrip().split(":")[1])
    assert netif_set[0]["id"] == if_id

    ret = device.send(command="network nameToId dummy", expected_output="Error network")
    assert ret != None

@pytest.mark.networktest
def test_network_interface_address_check(connected_device):
    # Get network interface
    ret = connected_device.send(command="network interface", expected_output="Done")
    assert ret != None and len(ret) > 3

    netif_addr_set = get_network_interface_address(ret)

    if not netif_addr_set:
        return
    
    for netif in netif_addr_set:
        log.info("Interface id: {}, IP {}".format(netif["id"], netif["ip"]))

    for netif in netif_addr_set:
        assert validIPAddress(netif["ip"]) != "Invalid"
